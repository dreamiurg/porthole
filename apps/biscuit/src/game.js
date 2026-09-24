import { discoveries } from './discoveries.js';

export const SAVE_KEY = 'biscuit-save-v1';

const STORIES = ['moon', 'library', 'dragon', 'seed', 'cloud', 'lighthouse', 'comet'];
const DISCOVERY_IDS = discoveries.map(({ id }) => id);
const CARE = ['feed', 'play', 'pet'];
const ACTIVITIES = [...CARE, 'read', 'train', 'rest'];
const MAX_COUNT = 1_000_000;
const clamp = (value) => Math.max(20, Math.min(100, value));
const localDay = (now) => {
  const date = new Date(now);
  return `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')}`;
};

export const TRICKS = [
  { id: 'sit', name: 'Sit', day: 1, pattern: ['down'], lessons: [
    ['paw', 'down', 'paw'], ['up', 'paw', 'down', 'paw'], ['up', 'paw', 'left', 'down', 'paw'],
  ] },
  { id: 'paw', name: 'Shake a paw', day: 1, pattern: ['paw'], lessons: [
    ['left', 'paw', 'right'], ['left', 'paw', 'right', 'paw'], ['left', 'paw', 'up', 'right', 'paw'],
  ] },
  { id: 'spin', name: 'Twirl', day: 2, pattern: ['left', 'right'], lessons: [
    ['left', 'up', 'right'], ['left', 'up', 'right', 'down'], ['left', 'up', 'right', 'down', 'left', 'paw'],
  ] },
  { id: 'bow', name: 'Take a bow', day: 3, pattern: ['down', 'paw'], lessons: [
    ['up', 'down', 'paw'], ['up', 'paw', 'down', 'paw'], ['up', 'left', 'paw', 'down', 'right', 'paw'],
  ] },
  { id: 'jump', name: 'Happy hop', day: 5, pattern: ['up', 'up'], lessons: [
    ['down', 'up', 'paw'], ['down', 'up', 'up', 'paw'], ['left', 'down', 'up', 'right', 'up', 'paw'],
  ] },
  { id: 'roll', name: 'Roll over', day: 7, pattern: ['left', 'down', 'right'], lessons: [
    ['left', 'down', 'right'], ['left', 'down', 'right', 'up'], ['paw', 'left', 'down', 'right', 'up', 'paw'],
  ] },
];

export function lessonPattern(trick, practice = 0) {
  return [...trick.lessons[Math.max(0, Math.min(2, Math.trunc(practice)))]];
}

export const STICKERS = ['Moonbeam', 'Little library', 'Golden paw', 'Daisy chain',
  'Cloud castle', 'Brave dragon', 'Rainbow scarf', 'Comet tail', 'Tiny lighthouse',
  'Magic acorn', 'Cozy teacup', 'Best friends'];

const ADVENTURES = [
  { title: 'A picnic for two', description: 'A blanket, a book, and a biscuit each. Biscuit has already chosen the sunny spot.',
    actions: ['feed', 'read', 'play'], word: 'Conjecture', meaning: 'An idea you think might be true, before you can prove it. Biscuit thinks every pocket contains a snack.' },
  { title: 'Puddles and pawprints', description: 'Biscuit found a puddle exactly his size. Now the rug has a little trail of flowers. Or pawprints.',
    actions: ['train', 'pet', 'rest'], word: 'Corroborate', meaning: 'To support a claim with more evidence. The muddy pawprints corroborate Biscuit’s story about splashing in puddles.' },
  { title: 'The hidden biscuit', description: 'Biscuit tucked away a biscuit for later. His nose remembers. The rest of him is still thinking.',
    actions: ['read', 'feed', 'pet'], word: 'Paradox', meaning: 'Something that seems to contradict itself. The better Biscuit hides his snack, the worse his chances of eating it.' },
  { title: 'The book on the top shelf', description: 'Biscuit can almost reach the book. Perhaps standing on tip-paws will help.',
    actions: ['play', 'train', 'read'], word: 'Ingenuity', meaning: 'Cleverness at finding an inventive solution. Biscuit cannot reach the shelf, but he can bring Ivy a sturdy step.' },
  { title: 'Under the sofa', description: 'A sock looks like a sleeping dragon from down here. Biscuit gives it a gentle nudge.',
    actions: ['pet', 'feed', 'rest'], word: 'Perspective', meaning: 'A way of seeing something, shaped by where you stand or what you know. From Biscuit’s perspective, the sofa is a mountain.' },
  { title: 'Look what I found!', description: 'Biscuit went looking for his ball and found a book instead. There is room for both on the blanket.',
    actions: ['train', 'read', 'rest'], word: 'Serendipity', meaning: 'Finding something good by happy accident. Biscuit hunted for a tennis ball and discovered the perfect story instead.' },
  { title: 'One more little try', description: 'The ball rolled under the sofa again. Biscuit has a wag, a wiggle, and another idea.',
    actions: ['play', 'pet', 'train'], word: 'Tenacity', meaning: 'Determination to keep trying when something is difficult. Biscuit tries a new way to reach his ball under the sofa.' },
];

function adventureForDay(day) {
  const ordinal = Math.floor(new Date(`${day}T12:00:00Z`).getTime() / 86_400_000);
  const cycle = (length) => ((ordinal % length) + length) % length;
  return { ...ADVENTURES[cycle(ADVENTURES.length)], sticker: cycle(STICKERS.length) };
}

export function dailyAdventure(now = Date.now()) {
  return adventureForDay(localDay(now));
}

export function createState(now = Date.now()) {
  return {
    version: 2, name: 'Biscuit', createdAt: now, updatedAt: now,
    fullness: 78, happiness: 86, energy: 80, stars: 0,
    completedStories: [], discoveries: [], careCounts: { feed: 0, play: 0, pet: 0 }, sleeping: false,
    friendship: 0, daysTogether: 1, lastVisitDay: localDay(now),
    daily: { day: localDay(now), completed: [], claimed: false },
    tricks: Object.fromEntries(TRICKS.map(({ id }) => [id, 0])), stickers: [],
  };
}

export function tick(state, now = Date.now()) {
  const hours = Math.min(8, Math.max(0, (now - state.updatedAt) / 3_600_000));
  const day = localDay(now);
  const newDay = day > state.lastVisitDay;
  return {
    ...state,
    updatedAt: Math.max(state.updatedAt, now),
    fullness: clamp(state.fullness - hours * (state.sleeping ? 2 : 4)),
    happiness: clamp(state.happiness - hours * (state.sleeping ? 0 : 2)),
    energy: clamp(state.energy + hours * (state.sleeping ? 30 : -3)),
    daysTogether: Math.min(MAX_COUNT, state.daysTogether + (newDay ? 1 : 0)),
    lastVisitDay: newDay ? day : state.lastVisitDay,
    daily: newDay ? { day, completed: [], claimed: false } : state.daily,
  };
}

function recordActivity(state, id) {
  const alreadyDone = state.daily.completed.includes(id);
  const completed = alreadyDone ? state.daily.completed : [...state.daily.completed, id];
  const adventure = adventureForDay(state.daily.day);
  const reward = !state.daily.claimed && adventure.actions.every((action) => completed.includes(action));
  return {
    ...state,
    friendship: Math.min(MAX_COUNT, state.friendship + (alreadyDone ? 0 : 2) + (reward ? 4 : 0)),
    daily: { ...state.daily, completed, claimed: state.daily.claimed || reward },
    stickers: reward && !state.stickers.includes(adventure.sticker)
      ? [...state.stickers, adventure.sticker] : state.stickers,
  };
}

// Same-day snapshots include their own daily rewards. Remove those first so
// combining distinct activities preserves them without counting repeats twice.
export function mergeDailyProgress(state, saved) {
  const completed = [...new Set([...state.daily.completed, ...saved.daily.completed])];
  const adventure = adventureForDay(state.daily.day);
  const claimed = state.daily.claimed || saved.daily.claimed
    || adventure.actions.every((action) => completed.includes(action));
  const earned = (daily) => daily.completed.length * 2 + (daily.claimed ? 4 : 0);
  const daily = { day: state.daily.day, completed, claimed };
  const earlierFriendship = Math.max(0,
    state.friendship - earned(state.daily), saved.friendship - earned(saved.daily));
  return {
    daily,
    friendship: Math.min(MAX_COUNT, Math.max(state.friendship, saved.friendship, earlierFriendship + earned(daily))),
    stickers: [...new Set([...state.stickers, ...saved.stickers, ...(claimed ? [adventure.sticker] : [])])],
  };
}

export function act(state, type, now = Date.now()) {
  const next = tick(state, now);
  if (type === 'sleep') {
    const toggled = { ...next, sleeping: !next.sleeping };
    return toggled.sleeping ? recordActivity(toggled, 'rest') : toggled;
  }
  if (next.sleeping || !CARE.includes(type)) return next;
  return recordActivity({
    ...next,
    fullness: clamp(next.fullness + (type === 'feed' ? 18 : 0)),
    happiness: clamp(next.happiness + (type === 'play' ? 14 : type === 'pet' ? 6 : 0)),
    energy: clamp(next.energy - (type === 'play' ? 8 : 0)),
    careCounts: { ...next.careCounts, [type]: Math.min(MAX_COUNT, next.careCounts[type] + 1) },
  }, type);
}

export function recordReading(state, now = Date.now()) {
  const next = tick(state, now);
  return next.sleeping ? next : recordActivity(next, 'read');
}

export function finishStory(state, id, now = Date.now()) {
  if (!STORIES.includes(id)) return state;
  const next = recordReading(state, now);
  if (next.sleeping || next.completedStories.includes(id)) return next;
  const completedStories = [...next.completedStories, id];
  return { ...next, completedStories, stars: completedStories.length * 3,
    happiness: clamp(next.happiness + 10) };
}

export function discover(state, id, now = Date.now()) {
  if (!DISCOVERY_IDS.includes(id)) return state;
  const next = recordReading(state, now);
  if (next.sleeping || next.discoveries.includes(id)) return next;
  return { ...next, discoveries: [...next.discoveries, id] };
}

export function practiceTrick(state, id, now = Date.now()) {
  const next = tick(state, now);
  const trick = TRICKS.find((item) => item.id === id);
  if (!trick || next.sleeping || next.daysTogether < trick.day) return next;
  return recordActivity({ ...next, tricks: { ...next.tricks, [id]: Math.min(3, next.tricks[id] + 1) } }, 'train');
}

export function companionStage(state) {
  if (state.daysTogether >= 7 && state.friendship >= 60) return { name: 'Story dog', next: 'A lifetime of little adventures awaits.' };
  if (state.daysTogether >= 3 && state.friendship >= 20) return { name: 'Young pup', next: 'Story dog: 7 days together and 60 friendship.' };
  return { name: 'Puppy', next: 'Young pup: 3 days together and 20 friendship.' };
}

export function personality(state) {
  return state.completedStories.length >= 2 ? 'Bookworm'
    : state.careCounts.play > state.careCounts.pet ? 'Playful' : 'Cuddlebug';
}

export function hydrate(raw, now = Date.now()) {
  const fresh = () => createState(now);
  try {
    if (typeof raw !== 'string') return fresh();
    const saved = JSON.parse(raw);
    const object = (value) => value !== null && typeof value === 'object' && !Array.isArray(value);
    const integer = (value) => Number.isSafeInteger(value) && value >= 0;
    const timestamp = (value) => integer(value) && value <= 253_402_214_400_000;
    const day = (value) => typeof value === 'string' && /^\d{4}-\d{2}-\d{2}$/.test(value)
      && localDay(new Date(`${value}T12:00:00`).getTime()) === value;
    const list = (value, allowed) => Array.isArray(value) && value.length <= allowed.length * 2
      && value.every((item) => allowed.includes(item));
    if (!object(saved) || ![1, 2].includes(saved.version) || saved.name !== 'Biscuit'
      || !timestamp(saved.createdAt) || !timestamp(saved.updatedAt) || saved.createdAt > saved.updatedAt
      || typeof saved.sleeping !== 'boolean'
      || !['fullness', 'happiness', 'energy'].every((key) => Number.isFinite(saved[key])
        && saved[key] >= 20 && saved[key] <= 100)
      || !integer(saved.stars) || saved.stars > (saved.version === 1 ? 9 : 21)
      || !list(saved.completedStories, saved.version === 1 ? STORIES.slice(0, 3) : STORIES)
      || !object(saved.careCounts) || !CARE.every((key) => integer(saved.careCounts[key]))) return fresh();

    if (saved.version === 2 && (!integer(saved.friendship) || !integer(saved.daysTogether) || saved.daysTogether < 1
      || !day(saved.lastVisitDay)
      || !object(saved.daily) || saved.daily.day !== saved.lastVisitDay
      || !list(saved.daily.completed, ACTIVITIES) || typeof saved.daily.claimed !== 'boolean'
      || !object(saved.tricks) || !TRICKS.every(({ id }) => integer(saved.tricks[id]) && saved.tricks[id] <= 3)
      || !list(saved.stickers, STICKERS.map((_, index) => index))
      || (saved.discoveries !== undefined && !list(saved.discoveries, DISCOVERY_IDS)))) return fresh();

    const completedStories = [...new Set(saved.completedStories)];
    return tick({
      ...createState(now), createdAt: saved.createdAt, updatedAt: saved.updatedAt,
      fullness: saved.fullness, happiness: saved.happiness, energy: saved.energy,
      sleeping: saved.sleeping, completedStories, stars: completedStories.length * 3,
      careCounts: Object.fromEntries(CARE.map((key) => [key, Math.min(MAX_COUNT, saved.careCounts[key])])),
      ...(saved.version === 2 ? {
        friendship: Math.min(MAX_COUNT, saved.friendship), daysTogether: Math.min(MAX_COUNT, saved.daysTogether),
        lastVisitDay: saved.lastVisitDay,
        daily: { day: saved.daily.day, completed: [...new Set(saved.daily.completed)], claimed: saved.daily.claimed },
        tricks: Object.fromEntries(TRICKS.map(({ id }) => [id, saved.tricks[id]])),
        stickers: [...new Set(saved.stickers)],
        discoveries: [...new Set(saved.discoveries ?? [])],
      } : {}),
    }, now);
  } catch {
    return fresh();
  }
}
