import test from 'node:test';
import assert from 'node:assert/strict';
import { discoveries, topics, dailyDiscoveries } from '../src/discoveries.js';

test('96 sourced discoveries cover all twelve topics with readable authored text', () => {
  assert.equal(discoveries.length, 96);
  assert.equal(new Set(discoveries.map(fact => fact.id)).size, 96);
  for (const topic of topics) assert.equal(discoveries.filter(fact => fact.topic === topic.id).length, 8, topic.id);
  for (const fact of discoveries) {
    assert.ok(fact.title.length <= 64 && fact.wonder.length <= 150, fact.id);
    assert.equal(fact.pages.length, 2, fact.id);
    assert.ok(fact.pages.every(page => page.length > 40 && page.length <= 180), fact.id);
    assert.equal(new URL(fact.source.url).protocol, 'https:', fact.id);
    assert.ok(fact.source.name, fact.id);
  }
});

test('daily suggestions are stable, span three topic families, and exhaust the collection in 32 days', () => {
  const seen = new Set();
  for (let day = 0; day < 32; day++) {
    const date = new Date(Date.UTC(2026, 8, 24 + day)).toISOString().slice(0, 10);
    const picks = dailyDiscoveries(date);
    assert.deepEqual(dailyDiscoveries(date), picks);
    assert.equal(new Set(picks.map(fact => Math.floor(topics.findIndex(topic => topic.id === fact.topic) / 4))).size, 3);
    for (const fact of picks) { assert.ok(!seen.has(fact.id), fact.id); seen.add(fact.id); }
  }
  assert.equal(seen.size, 96);
});
