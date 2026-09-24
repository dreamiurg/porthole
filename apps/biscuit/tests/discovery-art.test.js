import test from 'node:test';
import assert from 'node:assert/strict';
import { discoveries } from '../src/discoveries.js';
import { discoveryPictures, renderDiscoveryArt } from '../src/discovery-art.js';

test('every discovery has a described picture rendered only in bounded RGB565 pixels', () => {
  assert.deepEqual(Object.keys(discoveryPictures).sort(), discoveries.map(fact => fact.id).sort());
  for (const fact of discoveries) {
    let pixels = 0;
    const ctx = { fillStyle: '', fillRect(x, y, w, h) {
      assert.ok([x, y, w, h].every(Number.isInteger), fact.id);
      assert.ok(x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= 96 && y + h <= 48, fact.id);
      assert.match(this.fillStyle, /^#[a-f\d]{6}$/i, fact.id);
      const rgb = this.fillStyle.slice(1).match(/../g).map(value => parseInt(value, 16));
      rgb.forEach((value, channel) => {
        const steps = channel === 1 ? 63 : 31;
        assert.equal(value, Math.round(Math.round(value * steps / 255) * 255 / steps), fact.id);
      });
      pixels++;
    } };
    assert.ok(discoveryPictures[fact.id].alt.length >= 15, fact.id);
    renderDiscoveryArt(ctx, fact.id);
    assert.ok(pixels > 10, `${fact.id}: missing illustration`);
  }
});
