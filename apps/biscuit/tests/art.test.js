import assert from 'node:assert/strict';
import test from 'node:test';
import { renderScene } from '../src/art.js';

function frame(options) {
  const operations = [];
  const ctx = {
    save() {},
    restore() {},
    fillRect(...bounds) {
      assert.ok(bounds.every(Number.isFinite), 'Rectangle bounds must be finite');
      operations.push([this.fillStyle, ...bounds]);
    },
  };
  renderScene(ctx, options);
  assert.ok(operations.length > 0);
  return JSON.stringify(operations);
}

const tricks = ['sit', 'paw', 'spin', 'bow', 'jump', 'roll'];

test('each trick has a distinct pose and reduced motion freezes every pose', () => {
  const poses = tricks.map(trick => {
    const options = { activity: `trick-${trick}`, reducedMotion: true };
    const pose = frame({ ...options, time: 100 });
    assert.equal(pose, frame({ ...options, time: 1900 }));
    return pose;
  });
  assert.equal(new Set(poses).size, tricks.length);
});

test('growth accessories appear only when both age and friendship are sufficient', () => {
  const puppy = frame({ state: { daysTogether: 1, friendship: 0 } });
  assert.equal(frame({ state: { daysTogether: 3, friendship: 19 } }), puppy);
  assert.equal(frame({ state: { daysTogether: 2, friendship: 60 } }), puppy);
  const young = frame({ state: { daysTogether: 3, friendship: 20 } });
  assert.notEqual(young, puppy);
  assert.equal(frame({ state: { daysTogether: 7, friendship: 59 } }), young);
  assert.notEqual(frame({ state: { daysTogether: 7, friendship: 60 } }), young);
});

test('scene colors use RGB565 channel levels at every time of day', () => {
  const levels = count => new Set(Array.from({ length: count + 1 }, (_, i) => Math.round(i * 255 / count)));
  const channels = [levels(31), levels(63), levels(31)];
  for (const timeOfDay of ['day', 'evening', 'night']) {
    for (const activity of ['idle', 'play', 'feed', 'pet', ...tricks.map(id => `trick-${id}`)]) {
      for (const [color] of JSON.parse(frame({ timeOfDay, activity }))) {
        assert.match(color, /^#[0-9a-f]{6}$/i);
        channels.forEach((allowed, index) => assert.ok(allowed.has(parseInt(color.slice(1 + index * 2, 3 + index * 2), 16)), color));
      }
    }
  }
});
