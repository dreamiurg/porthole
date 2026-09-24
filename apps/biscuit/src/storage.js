import { SAVE_KEY, createState, hydrate, mergeDailyProgress } from './game.js';

// Read before changing state; merge permanent achievements before writing.
// This also keeps a playable in-memory save when browser storage is blocked/full.
export function progressStore(storage, now = Date.now) {
  let available = Boolean(storage);
  return {
    get available() { return available; },
    load(fallback = createState(now())) {
      if (!available) return fallback;
      try {
        const raw = storage.getItem(SAVE_KEY);
        return raw ? hydrate(raw, now()) : fallback;
      } catch { available = false; return fallback; }
    },
    save(state) {
      try {
        const saved = hydrate(storage.getItem(SAVE_KEY), now());
        const completedStories = [...new Set([...state.completedStories, ...saved.completedStories])];
        const merged = { ...state, completedStories, stars: completedStories.length * 3 };
        if (state.version === 2 && saved.version === 2) {
          merged.friendship = Math.max(state.friendship, saved.friendship);
          merged.daysTogether = Math.max(state.daysTogether, saved.daysTogether);
          merged.lastVisitDay = [state.lastVisitDay, saved.lastVisitDay].sort().at(-1);
          merged.stickers = [...new Set([...state.stickers, ...saved.stickers])];
          merged.discoveries = [...new Set([...(state.discoveries ?? []), ...saved.discoveries])];
          merged.tricks = Object.fromEntries(Object.keys(state.tricks).map(id => [id, Math.max(state.tricks[id], saved.tricks[id])]));
          if (state.daily.day === saved.daily.day) Object.assign(merged, mergeDailyProgress(state, saved));
          else if (saved.daily.day > state.daily.day) merged.daily = saved.daily;
        }
        storage.setItem(SAVE_KEY, JSON.stringify(merged));
        available = true;
        return merged;
      } catch { available = false; return state; }
    },
  };
}
