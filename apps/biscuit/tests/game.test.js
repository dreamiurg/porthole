import test from 'node:test';
import assert from 'node:assert/strict';
import { createState, tick, act, finishStory, discover, hydrate, dailyAdventure,
  practiceTrick, recordReading, companionStage, personality, lessonPattern, TRICKS, STICKERS } from '../src/game.js';

const HOUR = 3_600_000;
const NOW = new Date(2026, 8, 24, 12).getTime();
const laterDay = (days) => new Date(2026, 8, 24 + days, 12).getTime();

test('care clamps stats, counts actions, and preserves its input', () => {
  const original = createState(NOW);
  const fed = act({ ...original, fullness: 98 }, 'feed', NOW);
  assert.equal(fed.fullness, 100);
  assert.equal(fed.careCounts.feed, 1);
  assert.equal(original.careCounts.feed, 0);
  const played = act({ ...original, happiness: 97, energy: 21 }, 'play', NOW);
  assert.equal(played.happiness, 100);
  assert.equal(played.energy, 20);
  assert.equal(played.careCounts.play, 1);
  assert.equal(act(original, 'pet', NOW).careCounts.pet, 1);
});

test('sleep restores energy and ignores care until the puppy wakes', () => {
  const sleeping = act(createState(NOW), 'sleep', NOW);
  const rested = act(sleeping, 'feed', NOW + HOUR);
  assert.equal(rested.energy, 100);
  assert.equal(rested.fullness, 76);
  assert.equal(rested.happiness, 86);
  assert.equal(rested.careCounts.feed, 0);
  assert.equal(act(rested, 'sleep', NOW + HOUR).sleeping, false);
});

test('absence is gentle, capped at eight hours, and resilient to clock rollback', () => {
  const original = createState(NOW);
  const absent = tick(original, NOW + 100 * HOUR);
  assert.deepEqual([absent.fullness, absent.happiness, absent.energy], [46, 70, 56]);
  const low = tick({ ...original, fullness: 21, happiness: 21, energy: 21 }, NOW + HOUR);
  assert.deepEqual([low.fullness, low.happiness, low.energy], [20, 20, 20]);
  const rollback = tick(original, NOW - HOUR);
  assert.deepEqual(rollback, original);
  assert.equal(tick(rollback, NOW + HOUR).fullness, 74);
  assert.equal(original.fullness, 78);
});

test('each story earns its reward once and unknown IDs do nothing', () => {
  const original = createState(NOW);
  const read = finishStory(original, 'moon', NOW);
  assert.equal(read.stars, 3);
  assert.equal(read.happiness, 96);
  assert.deepEqual(read.completedStories, ['moon']);
  assert.deepEqual(finishStory(read, 'moon', NOW), read);
  assert.deepEqual(finishStory(read, '__proto__', NOW), read);
  const complete = ['library', 'dragon', 'seed', 'cloud', 'lighthouse', 'comet']
    .reduce((state, id) => finishStory(state, id, NOW), read);
  assert.equal(complete.stars, 21);
  assert.equal(complete.happiness, 100);
  const reread = finishStory(read, 'moon', laterDay(1));
  assert.equal(reread.stars, 3);
  assert.equal(reread.friendship, read.friendship + 2);
  assert.deepEqual(reread.daily.completed, ['read']);
  assert.deepEqual(original.completedStories, []);
});

test('finished discoveries enter the notebook once and share the existing daily reading reward', () => {
  const original = createState(NOW);
  assert.deepEqual(tick(original, NOW).discoveries, []);
  assert.deepEqual(discover(original, '__proto__', NOW), original);
  const sleeping = act(original, 'sleep', NOW);
  assert.deepEqual(discover(sleeping, 'space-01', NOW), sleeping);
  const first = discover(original, 'space-01', NOW);
  assert.deepEqual(first.discoveries, ['space-01']);
  assert.equal(first.friendship, 2);
  assert.equal(first.stars, 0);
  assert.deepEqual(discover(first, 'space-01', NOW), first);
  const second = discover(first, 'physics-01', NOW);
  assert.deepEqual(second.discoveries, ['space-01', 'physics-01']);
  assert.equal(second.friendship, 2);
  assert.equal(second.stars, 0);
  const story = finishStory(second, 'moon', NOW);
  assert.equal(story.friendship, 2);
  assert.equal(story.stars, 3);
  assert.deepEqual(hydrate(JSON.stringify(story), NOW), story);
  assert.deepEqual(original.discoveries, []);
});

test('untrusted saves reject malformed fields without throwing', () => {
  const fresh = createState(NOW);
  const invalid = [null, '{', 'null', '[]', '7', '{}',
    JSON.stringify({ ...fresh, fullness: '99' }),
    JSON.stringify({ ...fresh, energy: null }),
    JSON.stringify({ ...fresh, updatedAt: -1 }),
    JSON.stringify({ ...fresh, createdAt: NOW + 1 }),
    JSON.stringify({ ...fresh, sleeping: 'false' }),
    JSON.stringify({ ...fresh, completedStories: ['__proto__'] }),
    JSON.stringify({ ...fresh, stars: 22 }),
    JSON.stringify({ ...fresh, careCounts: { feed: -1, pet: 0, play: 0 } }),
    JSON.stringify({ ...fresh, careCounts: null }),
    JSON.stringify({ ...fresh, friendship: -1 }),
    JSON.stringify({ ...fresh, daysTogether: 0 }),
    JSON.stringify({ ...fresh, lastVisitDay: '2026-02-30' }),
    JSON.stringify({ ...fresh, daily: { ...fresh.daily, completed: ['buy'] } }),
    JSON.stringify({ ...fresh, daily: { ...fresh.daily, day: '2026-09-23' } }),
    JSON.stringify({ ...fresh, daily: { ...fresh.daily, claimed: 'yes' } }),
    JSON.stringify({ ...fresh, tricks: { ...fresh.tricks, roll: 4 } }),
    JSON.stringify({ ...fresh, stickers: [12] }),
    JSON.stringify({ ...fresh, discoveries: ['unknown-fact'] }),
    JSON.stringify({ ...fresh, discoveries: 'space-01' }),
    JSON.stringify(fresh).replace('"fullness":78', '"fullness":1e999')];
  for (const raw of invalid) assert.deepEqual(hydrate(raw, NOW), fresh, String(raw));
});

test('valid saves retain progress, normalize rewards, and apply elapsed time', () => {
  const saved = { ...finishStory(createState(NOW), 'library', NOW),
    completedStories: ['library', 'library'], stars: 9,
    careCounts: { feed: Number.MAX_SAFE_INTEGER, pet: 2, play: 3 } };
  const loaded = hydrate(JSON.stringify(saved), NOW + HOUR);
  assert.equal(loaded.fullness, 74);
  assert.equal(loaded.happiness, 94);
  assert.equal(loaded.energy, 77);
  assert.equal(loaded.stars, 3);
  assert.deepEqual(loaded.completedStories, ['library']);
  assert.equal(loaded.careCounts.feed, 1_000_000);
  assert.equal(loaded.careCounts.pet, 2);
  assert.equal(loaded.updatedAt, NOW + HOUR);
});

test('version one saves migrate in place without losing care or story progress', () => {
  const old = { version: 1, name: 'Biscuit', createdAt: NOW - HOUR, updatedAt: NOW,
    fullness: 65, happiness: 72, energy: 55, stars: 6, completedStories: ['moon', 'dragon'],
    careCounts: { feed: 8, play: 3, pet: 4 }, sleeping: true };
  const migrated = hydrate(JSON.stringify(old), NOW);
  assert.equal(migrated.version, 2);
  assert.equal(migrated.createdAt, old.createdAt);
  assert.deepEqual(migrated.completedStories, old.completedStories);
  assert.deepEqual(migrated.careCounts, old.careCounts);
  assert.equal(migrated.stars, 6);
  assert.equal(migrated.fullness, 65);
  assert.equal(migrated.sleeping, true);
  assert.equal(migrated.daysTogether, 1);
  assert.equal(migrated.lastVisitDay, '2026-09-24');
  assert.equal(migrated.friendship, 0);
  assert.deepEqual(hydrate(JSON.stringify(migrated), NOW), migrated);
});

test('version two saves from before the notebook retain all existing progress', () => {
  const old = finishStory(act(createState(NOW), 'feed', NOW), 'moon', NOW);
  delete old.discoveries;
  assert.deepEqual(hydrate(JSON.stringify(old), NOW), { ...old, discoveries: [] });
});

test('local calendar visits count active days, with no gap or rollback farming', () => {
  const start = act(createState(NOW), 'pet', NOW);
  const returning = tick(start, laterDay(14));
  assert.equal(returning.daysTogether, 2);
  assert.equal(returning.lastVisitDay, '2026-10-08');
  assert.deepEqual(returning.daily.completed, []);
  assert.equal(returning.friendship, start.friendship);
  const rollback = tick(returning, NOW);
  assert.deepEqual(rollback, returning);
  assert.equal(tick(rollback, laterDay(14)).daysTogether, 2);
  assert.equal(tick(rollback, laterDay(15)).daysTogether, 3);
  const midnight = new Date(2026, 8, 25, 0).getTime();
  assert.equal(tick(createState(midnight - 1), midnight).daysTogether, 2);
  assert.equal(act(createState(0), 'feed', 0).careCounts.feed, 1);
  const timezone = process.env.TZ;
  try {
    const instant = Date.parse('2026-09-25T00:30:00Z');
    process.env.TZ = 'Pacific/Auckland';
    const abroad = finishStory(createState(instant), 'moon', instant);
    process.env.TZ = 'America/Los_Angeles';
    assert.deepEqual(hydrate(JSON.stringify(abroad), instant), abroad);
  } finally {
    if (timezone === undefined) delete process.env.TZ;
    else process.env.TZ = timezone;
  }
});

test('daily adventures reward varied activities once and collect all sticker types', () => {
  const stickers = new Set();
  for (let day = 0; day < 84; day++) {
    const now = laterDay(day);
    const adventure = dailyAdventure(now);
    assert.deepEqual(dailyAdventure(now + HOUR), adventure);
    assert.equal(new Set(adventure.actions).size, 3);
    assert.ok(adventure.word && adventure.meaning && adventure.title && adventure.description);
    let state = createState(now);
    const perform = (current, id) => {
      if (current.sleeping) current = act(current, 'sleep', now);
      if (id === 'read') return recordReading(current, now);
      if (id === 'train') return practiceTrick(current, 'sit', now);
      return act(current, id === 'rest' ? 'sleep' : id, now);
    };
    state = perform(state, adventure.actions[0]);
    assert.equal(state.friendship, 2);
    state = perform(state, adventure.actions[0]);
    assert.equal(state.friendship, 2);
    for (const action of adventure.actions.slice(1)) state = perform(state, action);
    assert.equal(state.friendship, 10);
    assert.equal(state.daily.claimed, true);
    assert.deepEqual(state.stickers, [adventure.sticker]);
    for (const action of adventure.actions) state = perform(state, action);
    assert.equal(state.friendship, 10);
    assert.equal(state.stickers.length, 1);
    assert.equal(tick(state, laterDay(day - 1)).daily.claimed, true);
    stickers.add(adventure.sticker);
  }
  assert.equal(stickers.size, STICKERS.length);
});

test('tricks unlock by active days, practice to mastery, and survive absences', () => {
  const original = createState(NOW);
  assert.equal(TRICKS.length, 6);
  assert.deepEqual(practiceTrick(original, 'spin', NOW), original);
  assert.deepEqual(practiceTrick(original, '__proto__', NOW), original);
  let learned = original;
  for (let i = 0; i < 5; i++) learned = practiceTrick(learned, 'sit', NOW);
  assert.equal(learned.tricks.sit, 3);
  assert.equal(learned.friendship, 2);
  assert.equal(original.tricks.sit, 0);
  const nextDay = practiceTrick(learned, 'spin', laterDay(1));
  assert.equal(nextDay.tricks.spin, 1);
  const sleeping = act(nextDay, 'sleep', laterDay(1));
  assert.deepEqual(practiceTrick(sleeping, 'paw', laterDay(1)), sleeping);
  assert.deepEqual(tick(sleeping, laterDay(100)).tricks, sleeping.tricks);
  assert.deepEqual(hydrate(JSON.stringify(sleeping), laterDay(1)).tricks, sleeping.tricks);
});

test('trick lessons grow in difficulty and remain intact when a player uses a pattern', () => {
  const before = structuredClone(TRICKS);
  for (const trick of TRICKS) {
    const lessons = [0, 1, 2].map((practice) => lessonPattern(trick, practice));
    assert.deepEqual(lessons.map((pattern) => pattern.length).slice(0, 2), [3, 4]);
    assert.ok([5, 6].includes(lessons[2].length));
    assert.equal(new Set(lessons.map((pattern) => pattern.join(','))).size, 3);
    assert.ok(lessons.flat().every((cue) => ['left', 'right', 'up', 'down', 'paw'].includes(cue)));
    assert.deepEqual(lessonPattern(trick, -1), lessons[0]);
    assert.deepEqual(lessonPattern(trick, 3), lessons[2]);
    lessons[0].reverse();
    lessons[1].push('paw');
  }
  assert.deepEqual(TRICKS, before);
});

test('growth needs both active days and friendship, and personality reflects play', () => {
  const state = createState(NOW);
  assert.equal(companionStage({ ...state, daysTogether: 20 }).name, 'Puppy');
  assert.equal(companionStage({ ...state, friendship: 100 }).name, 'Puppy');
  assert.equal(companionStage({ ...state, daysTogether: 3, friendship: 20 }).name, 'Young pup');
  assert.equal(companionStage({ ...state, daysTogether: 7, friendship: 60 }).name, 'Story dog');
  assert.ok(companionStage(state).next);
  assert.equal(personality(state), 'Cuddlebug');
  assert.equal(personality(act(state, 'play', NOW)), 'Playful');
  assert.equal(personality({ ...state, completedStories: ['moon', 'library'] }), 'Bookworm');
});
