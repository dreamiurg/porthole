import science from './discoveries-science.js';
import culture from './discoveries-culture.js';
import life from './discoveries-life.js';

export const topics = [
  { id: 'space', name: 'Space' }, { id: 'physics', name: 'Physics' },
  { id: 'earth', name: 'Earth' }, { id: 'nature', name: 'Nature' },
  { id: 'history', name: 'History' }, { id: 'humanity', name: 'Humanity' },
  { id: 'art', name: 'Art' }, { id: 'inventions', name: 'Inventions' },
  { id: 'math', name: 'Mathematics' }, { id: 'animals', name: 'Animals' },
  { id: 'language', name: 'Languages' }, { id: 'body', name: 'Human body' },
];
export const discoveries = [...science, ...culture, ...life];

// Three distinct topic groups; every entry appears once in each 32-day cycle.
export function dailyDiscoveries(day) {
  const ordinal = Math.floor(new Date(`${day}T12:00:00Z`).getTime() / 86_400_000);
  const mod = (value, length) => ((value % length) + length) % length;
  return [0, 4, 8].map(offset => discoveries.filter(fact => fact.topic === topics[offset + mod(ordinal, 4)].id)[mod(Math.floor(ordinal / 4), 8)]);
}
