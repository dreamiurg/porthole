import { SAVE_KEY, createState, tick, act, finishStory, TRICKS, STICKERS, dailyAdventure, companionStage, personality, practiceTrick, lessonPattern, discover } from './game.js';
import { progressStore } from './storage.js';
import { renderScene } from './art.js';
import { stories } from './stories.js';
import { discoveries, topics, dailyDiscoveries } from './discoveries.js';
import { readingPages } from './reading.js';
import { singleTouch } from './touch.js';
import { discoveryPictures, renderDiscoveryArt } from './discovery-art.js';

await document.fonts.load('24px Montserrat');
const textMeasure = document.createElement('canvas').getContext('2d');
textMeasure.font = '24px Montserrat';
const paginate = text => readingPages(text, value => textMeasure.measureText(value).width);

const $ = selector => document.querySelector(selector);
const screen = $('#screen');
const panel = $('#screen-panel');
const canvas = $('#scene');
const ctx = canvas.getContext('2d');
const reducedMotion = matchMedia('(prefers-reduced-motion: reduce)');
let storage;
try { storage = window.localStorage; } catch { storage = null; }
const store = progressStore(storage);
let state = store.load();
let activity = 'idle';
let activityUntil = 0;
let currentStory = null;
let storyPage = 0;
let storyEnding = null;
let endingPage = 0;
let returnFocus = null;
let fetchScore = 0;
let fetchTarget = null;
let sound = false;
let audio;
let libraryPage = 0;
let training = null;
let trainingStep = 0;
let trainingPhase = 'watch';
let lastSurprise = 0;
let panelView = 'library';
let discoveryView = 'today';
let discoveryTopic = null;
let discoveryListPage = 0;
let currentDiscovery = null;
let discoveryPage = 0;

// Read before every change so an older tab cannot overwrite another tab's care.
function refreshState() {
  state = store.load(state);
}

function save() {
  state = store.save(state);
  $('#save-status').innerHTML = store.available
    ? '<span class="status-dot"></span> Progress saved on this device'
    : 'Saving unavailable — keep this tab open to play';
}

function say(message) { $('#speech').textContent = message; }

function bleep(notes = [440, 660]) {
  if (!sound) return;
  try {
    audio ??= new AudioContext();
    if (audio.state === 'suspended') audio.resume().catch(() => {});
    notes.forEach((note, index) => {
      const oscillator = audio.createOscillator();
      const volume = audio.createGain();
      const start = audio.currentTime + index * 0.085;
      oscillator.type = 'triangle';
      oscillator.frequency.value = note;
      volume.gain.setValueAtTime(0, start);
      volume.gain.linearRampToValueAtTime(0.035, start + 0.012);
      volume.gain.exponentialRampToValueAtTime(0.001, start + 0.12);
      oscillator.connect(volume).connect(audio.destination);
      oscillator.start(start);
      oscillator.stop(start + 0.13);
    });
  } catch {
    sound = false;
    updateSound();
    say('Sound is unavailable, but we can still play!');
  }
}

function updateUI() {
  for (const need of ['fullness', 'happiness', 'energy']) {
    const value = Math.round(state[need]);
    $(`#${need}-meter`).value = value;
    $(`#${need}-value`).textContent = value;
  }
  $('#pet-mood').textContent = state.sleeping ? 'dreaming of stories' : `Day ${state.daysTogether} · ${companionStage(state).name.toLowerCase()}`;
  screen.classList.toggle('sleeping', state.sleeping);
  $('#more-button span').textContent = state.sleeping ? 'Wake' : 'More';
  $('#more-button').dataset.action = state.sleeping ? 'sleep' : 'more';
  $('#more-button').setAttribute('aria-label', state.sleeping ? 'Wake Biscuit' : 'Biscuit’s world: adventures, tricks, and naps');
  for (const button of document.querySelectorAll('[data-action]')) {
    button.disabled = activity === 'play' || (state.sleeping && button.dataset.action !== 'sleep');
  }
  updateShelf();
}

function updateShelf() {
  const count = state.completedStories.length;
  $('#reading-progress').textContent = `${count} of ${stories.length} stories discovered · ${state.stars} stars`;
  $('#reading-title').textContent = count === stories.length ? 'A whole shelf of shared adventures' : count ? 'A new chapter of friendship' : 'Seven stories waiting for us';
  $('#friendship-progress').textContent = `Day ${state.daysTogether} together · ${state.friendship} friendship`;
  for (const button of document.querySelectorAll('.book-spine')) {
    const read = state.completedStories.includes(button.dataset.story);
    button.querySelector('.book-check').textContent = read ? '✓' : '○';
    button.querySelector('.book-check').setAttribute('aria-label', read ? 'Read together' : 'Not yet read');
  }
}

function pulse(nextActivity, message) {
  activity = nextActivity;
  activityUntil = performance.now() + 2400;
  say(message);
}

function care(type) {
  if (activity === 'play') return;
  refreshState();
  if (state.sleeping && type !== 'sleep') {
    say('Biscuit is all curled up. Tap Wake when you’re ready.');
    return;
  }
  state = act(state, type);
  if (type === 'feed') pulse('feed', ['Mmm. A crumb on my nose is a snack for later.', 'Happy tummy, happy tail. Thank you, friend!', 'I saved you a biscuit. Then I sat on it. Still yours?'][state.careCounts.feed % 3]);
  if (type === 'pet') pulse('pet', ['That’s the spot. My back paw agrees.', 'We make a good team. You do the doors; I do the smells.', 'Your hand is my favorite place to put my head.'][state.careCounts.pet % 3]);
  if (type === 'sleep') {
    activity = 'idle';
    say(state.sleeping ? 'One little yawn… and a very big stretch. Night, friend.' : 'I dreamed we could fly. You brought snacks.');
  }
  bleep(type === 'sleep' ? [392, 330, 262] : [523, 659]);
  updateUI();
  save();
}

function stopFetch() {
  fetchTarget = null;
  $('#fetch-ball').hidden = true;
  $('#stop-play').hidden = true;
  screen.classList.remove('playing');
  if (activity === 'play') activity = 'idle';
}

function home() {
  const wasPlaying = activity === 'play';
  stopFetch();
  panel.hidden = true;
  screen.classList.remove('panel-open');
  currentStory = null;
  currentDiscovery = null;
  training = null;
  activity = 'idle';
  updateUI();
  say(state.sleeping ? 'Dreaming of warm laps and very tall sunflowers.' : wasPlaying ? 'A little rest? I’ll bring the ball. Just in case.' : 'Books, biscuits, and you. My favorite things.');
  if (returnFocus?.isConnected) returnFocus.focus({ preventScroll: true });
  returnFocus = null;
}

function openPanel(label) {
  if (panel.hidden) returnFocus = document.activeElement;
  stopFetch();
  panel.hidden = false;
  screen.classList.add('panel-open');
  panel.setAttribute('aria-label', label);
  panel.dataset.view = panelView;
  updateUI();
}

// biome-ignore lint/complexity/noExcessiveCognitiveComplexity: one flat lookup of the back-button label per panel state
function panelTop(kicker) {
  const backLabel = currentDiscovery ? 'Back to discoveries' : panelView === 'discoveries' ? 'Back to bookshelf' : currentStory ? 'Back to bookshelf' : ['world', 'library'].includes(panelView) ? 'Back to Biscuit' : panelView === 'training' ? 'Back to tricks' : panelView === 'word' ? 'Back to today’s adventure' : 'Back to Biscuit’s world';
  return `<div class="panel-top"><button class="panel-back" data-panel="back" aria-label="${backLabel}"><svg><use href="#i-arrow"/></svg></button><span class="panel-kicker">${kicker}</span><span class="panel-stars">${state.stars} ★</span></div>`;
}

function focusPanel() {
  panel.scrollTop = 0;
  const target = panel.querySelector('.story-text, .discovery-cover-title') ?? panel.querySelector('.library-entry:not(:disabled)') ?? panel.querySelector('.story-next') ?? panel.querySelector('button');
  target?.focus({ preventScroll: true });
}

function openLibrary(page = 0) {
  refreshState();
  libraryPage = Math.max(0, Math.min(Math.ceil(stories.length / 3) - 1, page));
  currentStory = null;
  currentDiscovery = null;
  panelView = 'library';
  openPanel('Biscuit’s bookshelf');
  activity = 'read';
  panel.innerHTML = `${panelTop('OUR BOOKSHELF')}<nav class="reading-tabs" aria-label="Reading mode"><button aria-pressed="true" data-reading="stories">Stories</button><button aria-pressed="false" data-reading="discoveries">Discover</button></nav><div class="library-list">${stories.slice(libraryPage * 3, libraryPage * 3 + 3).map(story => {
    const locked = state.daysTogether < story.unlockDay;
    return `<button class="library-entry" data-open-story="${story.id}" ${locked ? 'disabled' : ''}><span aria-hidden="true">${story.symbol}</span><span><strong>${story.title}</strong><small>${locked ? `Opens on day ${story.unlockDay} together` : story.subtitle}</small></span><span class="library-check" aria-label="${locked ? 'Locked' : state.completedStories.includes(story.id) ? 'Read together' : 'New story'}">${locked ? '·' : state.completedStories.includes(story.id) ? '✓' : '+'}</span></button>`;
  }).join('')}</div><div class="shelf-pages"><button data-shelf="prev" aria-label="Previous shelf" ${libraryPage === 0 ? 'disabled' : ''}>←</button><span>Shelf ${libraryPage + 1} of ${Math.ceil(stories.length / 3)}</span><button data-shelf="next" aria-label="Next shelf" ${libraryPage === Math.ceil(stories.length / 3) - 1 ? 'disabled' : ''}>→</button></div>`;
  focusPanel();
  bleep([523]);
}

function paintDiscoveries() {
  for (const picture of panel.querySelectorAll('canvas[data-picture]')) {
    renderDiscoveryArt(picture.getContext('2d'), picture.dataset.picture);
    if (picture.getAttribute('role') === 'img') picture.setAttribute('aria-label', discoveryPictures[picture.dataset.picture].alt);
  }
}

function openDiscoveries(view = discoveryView, topic = discoveryTopic, page = discoveryListPage) {
  refreshState();
  currentStory = null;
  currentDiscovery = null;
  discoveryView = view;
  discoveryTopic = topic;
  discoveryListPage = page;
  panelView = 'discoveries';
  openPanel('Our discovery notebook');
  const listingTopics = view === 'topics' && !topic;
  const list = listingTopics ? topics : view === 'today' ? dailyDiscoveries(state.daily.day)
    : discoveries.filter(fact => view === 'notebook' ? state.discoveries.includes(fact.id) : fact.topic === topic);
  const pageSize = listingTopics ? 3 : 2;
  const totalPages = Math.max(1, Math.ceil(list.length / pageSize));
  discoveryListPage = Math.max(0, Math.min(totalPages - 1, page));
  const entries = list.slice(discoveryListPage * pageSize, discoveryListPage * pageSize + pageSize);
  const heading = topic && view === 'topics' ? topics.find(item => item.id === topic).name : 'OUR LITTLE NOTEBOOK';
  panel.innerHTML = `${panelTop(heading)}<nav class="discovery-tabs" aria-label="Explore discoveries">${[['today', 'Today'], ['topics', 'Topics'], ['notebook', 'Notebook']].map(([id, label]) => `<button data-discovery-view="${id}" aria-pressed="${view === id}">${label}</button>`).join('')}</nav><div class="discovery-list">${entries.map(item => listingTopics
    ? `<button data-topic="${item.id}"><canvas data-picture="${item.id}-01" width="96" height="48" aria-hidden="true"></canvas><strong>${item.name}</strong><span>→</span></button>`
    : `<button data-discovery="${item.id}"><canvas data-picture="${item.id}" width="96" height="48" aria-hidden="true"></canvas><strong>${item.title}</strong><span aria-label="${state.discoveries.includes(item.id) ? 'In our notebook' : 'New discovery'}">${state.discoveries.includes(item.id) ? '✓' : '+'}</span></button>`).join('') || '<p class="notebook-empty">Keep discoveries here as you explore. Pick a question from Today or Topics.</p>'}</div><div class="shelf-pages"><button data-discovery-list="prev" aria-label="Previous discoveries" ${discoveryListPage === 0 ? 'disabled' : ''}>←</button><span>${view === 'today' ? `Today · ${discoveryListPage + 1} / ${totalPages}` : `${discoveryListPage + 1} / ${totalPages}`}</span><button data-discovery-list="next" aria-label="Next discoveries" ${discoveryListPage === totalPages - 1 ? 'disabled' : ''}>→</button></div>`;
  paintDiscoveries();
  panel.querySelector('.discovery-list button, .discovery-tabs button')?.focus({ preventScroll: true });
}

function startDiscovery(id) {
  const fact = discoveries.find(item => item.id === id);
  if (!fact) return;
  refreshState();
  if (state.sleeping) { state = act(state, 'sleep'); save(); }
  currentStory = null;
  currentDiscovery = { ...fact, pages: paginate(fact.pages.join(' ')) };
  discoveryPage = -1;
  panelView = 'discovery';
  openPanel(fact.title);
  activity = 'read';
  renderDiscovery();
}

function renderDiscovery() {
  panelView = 'discovery';
  panel.dataset.view = panelView;
  if (discoveryPage === -1) {
    panel.dataset.view = 'discovery-cover';
    panel.innerHTML = `${panelTop(topics.find(topic => topic.id === currentDiscovery.topic).name)}<h2 class="discovery-cover-title" tabindex="-1">${currentDiscovery.title}</h2><canvas class="discovery-illustration" data-picture="${currentDiscovery.id}" width="96" height="48" role="img"></canvas><button class="story-next" data-fact="next">Let’s find out →</button>`;
    paintDiscoveries();
    focusPanel();
    return;
  }
  const wondering = discoveryPage === currentDiscovery.pages.length;
  panel.innerHTML = `${panelTop(topics.find(topic => topic.id === currentDiscovery.topic).name)}<p class="story-text" tabindex="-1">${wondering ? currentDiscovery.wonder : currentDiscovery.pages[discoveryPage]}</p><div class="story-navigation"><button class="story-previous" data-fact="previous" aria-label="Previous discovery page">←</button><button class="story-next" data-fact="${wondering ? 'keep' : 'next'}">${wondering ? 'Keep in notebook' : 'Continue →'}</button><button class="discovery-source" data-fact="source">Source</button></div><div class="page-count">${wondering ? 'Something to wonder about' : `Discovery · ${discoveryPage + 1} / ${currentDiscovery.pages.length}`}</div>`;
  focusPanel();
}

function showDiscoverySource() {
  panel.dataset.view = 'source';
  panel.innerHTML = `${panelTop('OUR SOURCE')}<div class="source-copy"><h2>${currentDiscovery.source.name}</h2><p>Read our source online, or return to the discovery.</p><a href="${currentDiscovery.source.url}" target="_blank" rel="noopener noreferrer">Open original source ↗</a></div><button class="story-next" data-fact="return">Back to discovery</button>`;
  focusPanel();
}

function startStory(id) {
  const story = stories.find(item => item.id === id);
  if (!story) return;
  refreshState();
  if (state.daysTogether < story.unlockDay) return;
  if (state.sleeping) { state = act(state, 'sleep'); save(); }
  currentDiscovery = null;
  currentStory = { ...story, pages: paginate(story.pages.join(' ')), choices: story.choices.map(choice => ({ ...choice, ending: paginate(choice.ending.join(' ')) })) };
  panelView = 'story';
  storyPage = 0;
  storyEnding = null;
  endingPage = 0;
  openPanel(story.title);
  activity = 'read';
  renderStory();
  if (matchMedia('(max-width: 1000px)').matches) screen.scrollIntoView({ block: 'center', behavior: reducedMotion.matches ? 'instant' : 'smooth' });
  bleep([440, 523, 659]);
}

// biome-ignore lint/complexity/noExcessiveCognitiveComplexity: renders every story page state in one template; untested DOM glue
function renderStory() {
  const choosing = storyPage === currentStory.pages.length && storyEnding === null;
  panel.dataset.view = choosing ? 'choice' : 'story';
  const ending = storyEnding !== null;
  const finished = ending && endingPage === storyEnding.length - 1;
  const pageLabel = ending ? `Ending · ${endingPage + 1} / ${storyEnding.length}` : choosing ? 'Where shall we go?' : `Chapter · ${storyPage + 1} / ${currentStory.pages.length}`;
  panel.innerHTML = `${panelTop(currentStory.title)}<p class="story-text" tabindex="-1">${ending ? storyEnding[endingPage] : choosing ? currentStory.prompt : currentStory.pages[storyPage]}</p>${choosing ? `<div class="story-choices">${currentStory.choices.map((choice, index) => `<button class="story-choice" data-choice="${index}">${choice.label}</button>`).join('')}</div><button class="revisit-clues" data-panel="previous">← Read that bit again</button>` : `<div class="story-navigation"><button class="story-previous" data-panel="previous" aria-label="Previous page" ${!ending && storyPage === 0 ? 'disabled' : ''}>←</button><button class="story-next" data-panel="${finished ? 'finish' : 'next'}">${finished ? 'The end · collect stars' : 'Turn the page →'}</button></div>`}${choosing ? '' : `<div class="page-count">${pageLabel}</div>`}`;
  focusPanel();
}

function completeReading() {
  if (!currentStory || storyEnding === null || endingPage !== storyEnding.length - 1) return;
  refreshState();
  const firstRead = !state.completedStories.includes(currentStory.id);
  state = finishStory(state, currentStory.id);
  save();
  updateUI();
  panel.dataset.view = 'reward';
  panel.innerHTML = `${panelTop('STORY TIME')}<h2>${firstRead ? 'A story shared.' : 'Hello again, old friend.'}</h2><p class="reward-copy">${firstRead ? '+3 story stars.<br />Shall we try the other ending sometime?' : 'I notice something new<br />every time we read together.'}</p><button class="story-next" data-panel="home">Give Biscuit a cuddle</button><p class="panel-footnote">${state.completedStories.length} of ${stories.length} stories discovered</p>`;
  storyEnding = null;
  bleep([523, 659, 784, 1047]);
  focusPanel();
}

function openWorld() {
  refreshState();
  currentStory = null;
  currentDiscovery = null;
  training = null;
  panelView = 'world';
  openPanel('Biscuit’s world');
  activity = 'idle';
  panel.innerHTML = `${panelTop('BISCUIT & YOU')}<p class="panel-subtitle">Day ${state.daysTogether} · ${companionStage(state).name} · ${state.friendship} friendship</p><div class="world-grid"><button data-world="daily"><svg><use href="#i-star"/></svg><strong>Today’s adventure</strong><small>${state.daily.claimed ? 'Sticker earned!' : 'Something to discover'}</small></button><button data-world="tricks"><svg><use href="#i-paw"/></svg><strong>Learn tricks</strong><small>${Object.values(state.tricks).filter(value => value === 3).length} of 6 mastered</small></button><button data-world="journal"><svg><use href="#i-book"/></svg><strong>Our scrapbook</strong><small>Growing up, page by page</small></button><button data-world="nap"><svg><use href="#i-moon"/></svg><strong>${state.sleeping ? 'Wake up' : 'Cozy nap'}</strong><small>A lovely place to pause</small></button></div><button class="screen-sound" data-world="sound" aria-pressed="${sound}"><svg><use href="#i-sound"/></svg>Sound ${sound ? 'on' : 'off'} · tap to ${sound ? 'mute' : 'listen'}</button>`;
  panel.querySelector('[data-world="daily"]').focus({ preventScroll: true });
}

const actionNames = { feed: 'Share a snack', pet: 'Give a cuddle', play: 'Play a round of fetch', read: 'Read or discover', train: 'Practice a trick', rest: 'Settle in for a nap' };
function openDaily() {
  refreshState();
  panelView = 'daily';
  openPanel('Today’s adventure');
  const adventure = dailyAdventure(new Date(`${state.daily.day}T12:00:00`).getTime());
  panel.innerHTML = `${panelTop('TODAY, TOGETHER')}<h2>${adventure.title}</h2><div class="daily-list">${adventure.actions.map(id => `<button data-invitation="${id}"><span class="task-check">${state.daily.completed.includes(id) ? '✓' : '○'}</span><span>${actionNames[id]}</span><span aria-hidden="true">→</span></button>`).join('')}</div><button class="journal-toggle" data-world="word">Word: ${adventure.word} →</button>`;
  panel.querySelector('[data-invitation]').focus({ preventScroll: true });
}

function openWord() {
  const adventure = dailyAdventure(new Date(`${state.daily.day}T12:00:00`).getTime());
  panelView = 'word';
  openPanel('Our pocket word');
  panel.innerHTML = `${panelTop('OUR POCKET WORD')}<div class="word-card"><h2>${adventure.word}</h2><p>${adventure.meaning}</p></div><button class="story-next" data-world="daily">Back to our adventure</button>`;
  panel.querySelector('.story-next').focus({ preventScroll: true });
}

function openJournal(stickers = false) {
  refreshState();
  panelView = 'journal';
  openPanel('Our scrapbook');
  const stage = companionStage(state);
  panel.innerHTML = stickers
    ? `${panelTop('OUR STICKER PAGES')}<div class="sticker-grid">${STICKERS.map((name, id) => `<div class="sticker ${state.stickers.includes(id) ? 'earned' : ''}"><svg aria-hidden="true"><use href="#i-${['star', 'book', 'heart', 'paw'][id % 4]}"/></svg><span>${state.stickers.includes(id) ? name : 'To discover'}</span></div>`).join('')}</div><button class="journal-toggle" data-journal="profile">← Biscuit’s story</button>`
    : `${panelTop('BISCUIT’S STORY')}<dl class="profile-facts"><div><dt>Growing into</dt><dd>${stage.name}</dd></div><div><dt>At heart</dt><dd>${personality(state)}</dd></div><div><dt>Days together</dt><dd>${state.daysTogether}</dd></div><div><dt>Friendship</dt><dd>${state.friendship}</dd></div><div><dt>Tricks mastered</dt><dd>${Object.values(state.tricks).filter(value => value === 3).length} / 6</dd></div></dl><p class="next-milestone">${stage.next}</p><button class="journal-toggle" data-journal="stickers">Our stickers · ${state.stickers.length} / ${STICKERS.length} →</button>`;
  panel.querySelector('.journal-toggle').focus({ preventScroll: true });
}

function openTricks() {
  refreshState();
  panelView = 'tricks';
  currentStory = null;
  currentDiscovery = null;
  training = null;
  openPanel('Biscuit’s trick book');
  panel.innerHTML = `${panelTop('BISCUIT’S TRICK BOOK')}<div class="trick-grid">${TRICKS.map(trick => `<button data-trick="${trick.id}" ${state.daysTogether < trick.day ? 'disabled' : ''}><strong>${trick.name}</strong><span>${state.daysTogether < trick.day ? `Day ${trick.day} together` : state.tricks[trick.id] === 3 ? '★ Mastered · show me' : `${state.tricks[trick.id]} of 3 practices`}</span></button>`).join('')}</div><p class="panel-footnote">Learned tricks are yours forever.<br />Days together don’t have to be in a row.</p>`;
  panel.querySelector('[data-trick]:not(:disabled)').focus({ preventScroll: true });
}

const cues = { left: ['←', 'Left'], right: ['→', 'Right'], up: ['↑', 'Up'], down: ['↓', 'Down'], paw: ['✦', 'Paw'] };
function chooseTrick(id) {
  refreshState();
  const trick = TRICKS.find(item => item.id === id);
  if (!trick || trick.day > state.daysTogether) return;
  if (state.sleeping) { state = act(state, 'sleep'); save(); }
  if (state.tricks[id] === 3) {
    state = practiceTrick(state, id);
    save();
    home();
    pulse(`trick-${id}`, `${trick.name}! Ta-da. A little flourish for you.`);
    bleep([523, 784]);
    return;
  }
  training = { ...trick, pattern: lessonPattern(trick, state.tricks[id]) };
  trainingStep = 0;
  trainingPhase = 'watch';
  panelView = 'training';
  openPanel(`Learn ${trick.name}`);
  renderTraining();
}

function renderTraining(message = '') {
  const watching = trainingPhase === 'watch';
  panel.innerHTML = `${panelTop(`LEARN: ${training.name}`)}<p class="panel-subtitle" role="status">${message || (watching ? 'Have a look, then try with me.' : 'Your turn! Tap the same pattern.')}</p>${watching ? `<div class="cue-sequence">${training.pattern.map((cue, index) => `<span><i>${index + 1}</i><b>${cues[cue][0]}</b><small>${cues[cue][1]}</small></span>`).join('')}</div><p class="training-note">Lesson ${state.tricks[training.id] + 1} of 3 · ${training.pattern.length} cues</p><button class="story-next" data-training="ready">I’m ready →</button>` : `<div class="training-dots" aria-label="${trainingStep} of ${training.pattern.length} cues matched">${training.pattern.map((_, index) => `<span>${index < trainingStep ? '●' : '○'}</span>`).join('')}</div><div class="cue-buttons">${['left', 'up', 'right', 'down', 'paw'].map(cue => `<button data-cue="${cue}" aria-label="${cues[cue][1]} cue"><b>${cues[cue][0]}</b><span>${cues[cue][1]}</span></button>`).join('')}<button data-training="repeat" aria-label="See pattern again"><span>See it<br />again</span></button></div>`}`;
  panel.querySelector(watching ? '[data-training="ready"]' : '[data-cue]').focus({ preventScroll: true });
}

function answerCue(cue) {
  if (!training || trainingPhase !== 'try') return;
  if (cue !== training.pattern[trainingStep]) {
    trainingStep = 0;
    trainingPhase = 'watch';
    renderTraining('Let’s peek at the pattern and try again.');
    return;
  }
  trainingStep++;
  bleep([440 + trainingStep * 110]);
  if (trainingStep < training.pattern.length) { renderTraining(); return; }
  refreshState();
  const learned = training;
  state = practiceTrick(state, learned.id);
  save();
  home();
  const mastered = state.tricks[learned.id] === 3;
  pulse(`trick-${learned.id}`, mastered ? `${learned.name} learned! That deserves a tail wag.` : `We’ve got that bit! Ready for a longer pattern?`);
  updateUI();
}

const ballSpots = [{ x: 47, y: 77 }, { x: 113, y: 91 }, { x: 103, y: 63 }, { x: 51, y: 99 }, { x: 81, y: 73 }];
function placeBall() {
  fetchTarget = ballSpots[fetchScore];
  const button = $('#fetch-ball');
  button.style.left = `${fetchTarget.x / 160 * 100}%`;
  button.style.top = `${fetchTarget.y / 160 * 100}%`;
  button.setAttribute('aria-label', `Catch ball ${fetchScore + 1} of 5`);
  say(`Fetch! Tap the ball. ${fetchScore} of 5 caught.`);
}

function startFetch() {
  refreshState();
  if (state.sleeping) { state = act(state, 'sleep'); save(); }
  home();
  activity = 'play';
  fetchScore = 0;
  screen.classList.add('playing');
  $('#fetch-ball').hidden = false;
  $('#stop-play').hidden = false;
  placeBall();
  updateUI();
  $('#fetch-ball').focus({ preventScroll: true });
  bleep([392, 523]);
}

$('#fetch-ball').addEventListener('click', () => {
  if (activity !== 'play' || !fetchTarget) return;
  fetchScore++;
  bleep([440 + fetchScore * 80]);
  if (fetchScore === 5) {
    stopFetch();
    refreshState();
    state = act(state, 'play');
    pulse('pet', 'Five catches! My tail would like to keep playing.');
    updateUI();
    save();
    $('[data-action="play"]').focus({ preventScroll: true });
  } else placeBall();
});
$('#stop-play').addEventListener('click', () => { home(); $('[data-action="play"]').focus({ preventScroll: true }); });
$('#pet-button').addEventListener('click', () => care('pet'));
$('#game-actions').addEventListener('click', event => {
  const action = event.target.closest('[data-action]')?.dataset.action;
  if (!action) return;
  if (action === 'read') openLibrary();
  else if (action === 'play') startFetch();
  else if (action === 'more') openWorld();
  else care(action);
});
// biome-ignore lint/complexity/noExcessiveCognitiveComplexity: single delegated click router for every panel control
panel.addEventListener('click', event => {
  const button = event.target.closest('button');
  if (!button) return;
  if (button.disabled) return;
  if (button.dataset.reading) {
    if (button.dataset.reading === 'stories') openLibrary();
    else openDiscoveries('today', null, 0);
    return;
  }
  if (button.dataset.discoveryView) { openDiscoveries(button.dataset.discoveryView, null, 0); return; }
  if (button.dataset.topic) { openDiscoveries('topics', button.dataset.topic, 0); return; }
  if (button.dataset.discoveryList) { openDiscoveries(discoveryView, discoveryTopic, discoveryListPage + (button.dataset.discoveryList === 'next' ? 1 : -1)); return; }
  if (button.dataset.discovery) { startDiscovery(button.dataset.discovery); return; }
  if (button.dataset.fact && currentDiscovery) {
    switch (button.dataset.fact) {
      case 'next': discoveryPage = Math.min(currentDiscovery.pages.length, discoveryPage + 1); renderDiscovery(); break;
      case 'previous': discoveryPage = Math.max(-1, discoveryPage - 1); renderDiscovery(); break;
      case 'source': showDiscoverySource(); break;
      case 'return': renderDiscovery(); break;
      case 'keep':
        if (discoveryPage !== currentDiscovery.pages.length) break;
        refreshState();
        state = discover(state, currentDiscovery.id);
        save();
        openDiscoveries();
        bleep([523, 659]);
        break;
    }
    return;
  }
  if (button.dataset.world) {
    switch (button.dataset.world) {
      case 'daily': openDaily(); break;
      case 'word': openWord(); break;
      case 'tricks': openTricks(); break;
      case 'journal': openJournal(); break;
      case 'nap': home(); care('sleep'); break;
      case 'sound': sound = !sound; openWorld(); bleep([523, 659]); break;
    }
    return;
  }
  if (button.dataset.invitation) {
    const invitation = button.dataset.invitation;
    if (invitation === 'read') openLibrary();
    else if (invitation === 'train') openTricks();
    else if (invitation === 'play') startFetch();
    else { home(); care(invitation === 'rest' ? 'sleep' : invitation); }
    return;
  }
  if (button.dataset.journal) { openJournal(button.dataset.journal === 'stickers'); return; }
  if (button.dataset.trick) { chooseTrick(button.dataset.trick); return; }
  if (button.dataset.cue) { answerCue(button.dataset.cue); return; }
  if (button.dataset.training) {
    trainingStep = 0;
    trainingPhase = button.dataset.training === 'ready' ? 'try' : 'watch';
    renderTraining();
    return;
  }
  if (button.dataset.shelf) { openLibrary(libraryPage + (button.dataset.shelf === 'next' ? 1 : -1)); return; }
  if (button.dataset.openStory) startStory(button.dataset.openStory);
  else if (button.dataset.choice !== undefined && currentStory && storyEnding === null && storyPage === currentStory.pages.length) {
    storyEnding = currentStory.choices[Number(button.dataset.choice)].ending;
    endingPage = 0;
    renderStory();
    bleep([659, 784]);
  } else {
    switch (button.dataset.panel) {
      case 'back':
        if (currentDiscovery) openDiscoveries();
        else if (panelView === 'discoveries') openLibrary(libraryPage);
        else if (currentStory) openLibrary(libraryPage);
        else if (['world', 'library'].includes(panelView)) home();
        else if (panelView === 'training') openTricks();
        else if (panelView === 'word') openDaily();
        else openWorld();
        break;
      case 'previous':
        if (!currentStory) break;
        if (storyEnding !== null) {
          if (endingPage > 0) endingPage--;
          else storyEnding = null;
        } else if (storyPage > 0) storyPage--;
        renderStory();
        break;
      case 'next':
        if (!currentStory) break;
        if (storyEnding !== null && endingPage < storyEnding.length - 1) endingPage++;
        else if (storyEnding === null && storyPage < currentStory.pages.length) storyPage++;
        renderStory();
        bleep([523]);
        break;
      case 'finish': completeReading(); break;
      case 'home': home(); care('pet'); break;
    }
  }
});
singleTouch(screen);
function updatePreviewScale() {
  const enabled = $('#physical-size').checked;
  const scale = Number($('#screen-size').value) / 480;
  $('.emulator').classList.toggle('physical-preview', enabled);
  $('.emulator').style.setProperty('--preview-scale', scale);
  $('.emulator').style.setProperty('--preview-width', `${560 * scale}px`);
  $('.emulator').style.setProperty('--preview-height', `${628 * scale}px`);
  $('#size-calibration').hidden = !enabled;
}
$('#physical-size').addEventListener('change', updatePreviewScale);
$('#screen-size').addEventListener('input', updatePreviewScale);
$('#pet-profile').addEventListener('click', openWorld);
$('#book-stack').addEventListener('click', event => {
  const id = event.target.closest('[data-story]')?.dataset.story;
  if (id) startStory(id);
});

function updateSound() {
  const button = panel.querySelector('[data-world="sound"]');
  if (button) { button.setAttribute('aria-pressed', String(sound)); button.innerHTML = `<svg><use href="#i-sound"/></svg>Sound ${sound ? 'on' : 'off'}`; }
}
$('#help-button').addEventListener('click', () => {
  const help = $('#help-panel');
  help.hidden = !help.hidden;
  $('#help-button').setAttribute('aria-expanded', String(!help.hidden));
  if (!help.hidden) { help.scrollIntoView({ block: 'center', behavior: reducedMotion.matches ? 'instant' : 'smooth' }); $('#close-help').focus({ preventScroll: true }); }
});
$('#close-help').addEventListener('click', () => { $('#help-panel').hidden = true; $('#help-button').setAttribute('aria-expanded', 'false'); $('#help-button').focus(); });
document.addEventListener('keydown', event => {
  if (event.key === 'Escape') {
    if (!panel.hidden || activity === 'play') home();
    else if (!$('#help-panel').hidden) $('#close-help').click();
  }
});

let lastFrame = -Infinity;
// biome-ignore lint/complexity/noExcessiveCognitiveComplexity: animation frame loop; branches are per-activity timing
function draw(time) {
  if (!document.hidden && panel.hidden && time - lastFrame >= 100) {
    if (activityUntil && time > activityUntil && (['feed', 'pet'].includes(activity) || activity.startsWith('trick-'))) { activity = 'idle'; activityUntil = 0; }
    if (activity === 'idle' && panel.hidden && !state.sleeping && !reducedMotion.matches && time - lastSurprise > 20000) {
      const learned = TRICKS.filter(trick => state.tricks[trick.id] === 3);
      if (learned.length) pulse(`trick-${learned[Math.floor(time / 20000) % learned.length].id}`, 'Hey, look! I’ve been practicing.');
      lastSurprise = time;
    }
    const hour = new Date().getHours();
    const timeOfDay = hour >= 20 || hour < 7 ? 'night' : hour >= 17 ? 'evening' : 'day';
    screen.classList.toggle('night', timeOfDay === 'night');
    renderScene(ctx, { time, state, activity, fetchTarget, fetchScore, timeOfDay, reducedMotion: reducedMotion.matches });
    lastFrame = time;
  }
  requestAnimationFrame(draw);
}
setInterval(() => {
  if (document.hidden) return;
  refreshState();
  state = tick(state);
  updateUI();
  save();
}, 10000);
document.addEventListener('visibilitychange', () => {
  refreshState();
  state = tick(state);
  save();
  updateUI();
});
window.addEventListener('storage', event => {
  if (event.key === SAVE_KEY) { refreshState(); updateUI(); }
});
window.addEventListener('pagehide', () => { refreshState(); state = tick(state); save(); });
renderScene($('#portrait').getContext('2d'), { time: 0, state: createState(), activity: 'idle', reducedMotion: true });
updateUI();
save();
if (state.sleeping) say('Biscuit is dreaming. Tap Wake when you’re ready.');
else say(`Hi, friend! Today: ${dailyAdventure(new Date(`${state.daily.day}T12:00:00`).getTime()).title}`);
requestAnimationFrame(draw);
