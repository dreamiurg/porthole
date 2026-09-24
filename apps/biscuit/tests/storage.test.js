import test from 'node:test';
import assert from 'node:assert/strict';
import { createState, finishStory, discover, act, dailyAdventure, SAVE_KEY } from '../src/game.js';
import { progressStore } from '../src/storage.js';

const now = new Date(2026, 8, 24, 12).getTime();
function memoryStorage() {
  const values = new Map();
  return { getItem: key => values.get(key) ?? null, setItem: (key, value) => values.set(key, value) };
}

test('another tab cannot erase permanent reading progress with an older save', () => {
  const storage = memoryStorage();
  const a = progressStore(storage, () => now);
  const b = progressStore(storage, () => now);
  const stale = b.load();
  a.save(finishStory(a.load(), 'moon', now));
  b.save(stale);
  assert.deepEqual(a.load().completedStories, ['moon']);
  assert.equal(a.load().stars, 3);
  const fed = a.save(act(a.load(), 'feed', now));
  assert.equal(b.load().fullness, fed.fullness);
});

test('stale tabs merge notebook discoveries without replaying reading rewards', () => {
  const storage = memoryStorage();
  const a = progressStore(storage, () => now);
  const b = progressStore(storage, () => now);
  const stale = b.load();
  a.save(discover(a.load(), 'space-01', now));
  const merged = b.save(discover(stale, 'physics-01', now));
  assert.deepEqual(new Set(merged.discoveries), new Set(['space-01', 'physics-01']));
  assert.equal(merged.friendship, 2);
  assert.equal(merged.stars, 0);
  delete stale.discoveries;
  b.save(stale);
  const restored = a.load();
  assert.deepEqual(new Set(restored.discoveries), new Set(['space-01', 'physics-01']));
  assert.equal(restored.friendship, 2);
  assert.deepEqual(restored.daily.completed, ['read']);
});

test('distinct same-day actions merge their rewards and complete the adventure exactly once', () => {
  const storage = memoryStorage();
  const a = progressStore(storage, () => now);
  const b = progressStore(storage, () => now);
  const base = { ...createState(now), friendship: 40 };
  a.save(base);
  const stale = b.load();
  const first = a.save(discover(act(a.load(), 'feed', now), 'space-01', now));
  const second = b.save(act(stale, 'play', now));
  assert.deepEqual(new Set(second.daily.completed), new Set(['feed', 'read', 'play']));
  assert.equal(second.friendship, 50);
  assert.equal(second.daily.claimed, true);
  assert.deepEqual(second.stickers, [dailyAdventure(now).sticker]);
  assert.equal(second.stars, 0);
  assert.deepEqual(second.discoveries, ['space-01']);
  a.save(first);
  b.save(second);
  const repeated = a.save(discover(a.load(), 'physics-01', now));
  assert.equal(repeated.friendship, 50);
  assert.deepEqual(repeated.stickers, second.stickers);
  assert.equal(repeated.daily.claimed, true);
});

test('readable but unwritable storage never rolls back in-memory progress', () => {
  const storage = memoryStorage();
  storage.setItem(SAVE_KEY, JSON.stringify(createState(now)));
  storage.setItem = () => { throw new Error('QuotaExceededError'); };
  const store = progressStore(storage, () => now);
  const unsaved = store.save(finishStory(store.load(), 'moon', now));
  assert.equal(store.available, false);
  assert.equal(store.load(unsaved).stars, 3);
  assert.equal(store.save(act(store.load(unsaved), 'pet', now)).stars, 3);
});

test('storage disabled entirely leaves the game playable', () => {
  const store = progressStore(null, () => now);
  const state = store.save(act(store.load(), 'feed', now));
  assert.equal(state.fullness, 96);
  assert.equal(store.load(state), state);
  assert.equal(store.available, false);
});

test('a stale save across midnight keeps the current daily record valid', () => {
  const storage = memoryStorage();
  const tomorrow = new Date(2026, 8, 25, 12).getTime();
  const old = finishStory(createState(now), 'moon', now);
  const current = finishStory(createState(tomorrow), 'library', tomorrow);
  storage.setItem(SAVE_KEY, JSON.stringify(current));
  const store = progressStore(storage, () => tomorrow);
  store.save(old);
  const restored = store.load();
  assert.equal(restored.stars, 6);
  assert.equal(restored.lastVisitDay, restored.daily.day);
  assert.equal(restored.daily.day, '2026-09-25');
});
