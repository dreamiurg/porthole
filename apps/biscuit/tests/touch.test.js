import test from 'node:test';
import assert from 'node:assert/strict';
import { singleTouch } from '../src/touch.js';

test('primary touch clicks work even with default isPrimary=false; secondary contacts do not', () => {
  const screen = new EventTarget();
  singleTouch(screen);
  let actions = 0;
  screen.addEventListener('click', () => actions++);
  const fire = (type, id, primary = false) => screen.dispatchEvent(Object.assign(new Event(type, { cancelable: true }), { pointerId: id, pointerType: 'touch', isPrimary: primary }));
  fire('pointerdown', 1, true);
  fire('pointerdown', 2);
  fire('click', 2);
  fire('click', 1);
  assert.equal(actions, 1);
  fire('pointerdown', 2, true); // Browsers may reuse a pointer ID.
  fire('click', 2);
  screen.dispatchEvent(new Event('click')); // Keyboard activation.
  assert.equal(actions, 3);
});
