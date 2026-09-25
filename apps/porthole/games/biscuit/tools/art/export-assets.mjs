#!/usr/bin/env node
// Biscuit's scene and discovery art -> games/biscuit/generated/. Node 22+, no npm packages, no Canvas.
//   node games/biscuit/tools/art/export-assets.mjs           regenerate
//   node games/biscuit/tools/art/export-assets.mjs --check   fail if the committed files are stale
// art.js and discovery-art*.js are the source of truth; they draw whole-pixel rectangles into the tiny raster below.
// generated/manifest.json records a SHA-256 per scene and per picture (RGB565, little endian): the same hashes the
// legacy Biscuit firmware shipped, so any change to the pixels shows up there by name, not as a 20 MB header diff.
// ponytail: two languages for art tooling; port art.js to Python if Biscuit's art starts changing often (the manifest
// hashes are an exact oracle for such a port).
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { readFile, writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { renderScene } from './art.js';
import { discoveryPictures, renderDiscoveryArt } from './discovery-art.js';

const output = new URL('../../generated/', import.meta.url);
const checkOnly = process.argv.includes('--check');
assert(process.argv.slice(2).every((arg) => arg === '--check'), 'Only --check is supported');
// Order is the C++ index order: SCENES[stage][time][activity][frame]. Append only.
const activities = ['idle', 'feed', 'pet', 'read', 'play', 'sleep', 'trick-sit', 'trick-paw', 'trick-spin', 'trick-bow',
  'trick-jump', 'trick-roll', 'shelf', 'fern'];
const times = ['day', 'evening', 'night'];
const stages = [{ daysTogether: 1, friendship: 0 }, { daysTogether: 3, friendship: 20 }, { daysTogether: 7, friendship: 60 }];
const frameTimes = [0, 230, 460, 690];
const sleepFrameTimes = [0, 600, 1200, 1800];
const hash = (value) => createHash('sha256').update(value).digest('hex');

function raster(width, height) {
  const pixels = new Uint16Array(width * height);
  const painted = new Uint8Array(pixels.length);
  let color = 0;
  const stack = [];
  return {
    set fillStyle(value) {
      assert.match(value, /^#[0-9a-f]{6}$/i, `Unsupported color ${value}`);
      const rgb = Number.parseInt(value.slice(1), 16);
      color = ((rgb >> 19) << 11) | (((rgb >> 10) & 63) << 5) | ((rgb >> 3) & 31);
    },
    fillRect(x, y, w, h) {
      assert([x, y, w, h].every(Number.isInteger), 'The source art must use whole pixels');
      assert(w >= 0 && h >= 0, 'The source art must use positive rectangles');
      const left = Math.max(0, x), right = Math.min(width, x + w);
      const top = Math.max(0, y), bottom = Math.min(height, y + h);
      for (let row = top; row < bottom; row++) {
        pixels.fill(color, row * width + left, row * width + right);
        painted.fill(1, row * width + left, row * width + right);
      }
    },
    save() { stack.push(color); },
    restore() { assert(stack.length, 'Unbalanced restore'); color = stack.pop(); },
    finish() {
      assert.equal(stack.length, 0, 'Unbalanced save');
      assert(painted.every(Boolean), 'Renderer did not cover the full image');
      return pixels;
    },
  };
}

function littleEndian(pixels) {
  const bytes = Buffer.alloc(pixels.length * 2);
  pixels.forEach((color, i) => bytes.writeUInt16LE(color, i * 2));
  return bytes;
}

// Alternating (count, color) pairs; the decoder in the game trusts that counts sum to width*height.
function encode(pixels) {
  const runs = [];
  for (let i = 0; i < pixels.length;) {
    let count = 1;
    while (count < 65535 && i + count < pixels.length && pixels[i + count] === pixels[i]) count++;
    runs.push(count, pixels[i]);
    i += count;
  }
  return runs;
}

// Decimal, no padding: about half the size of 0x-hex and compiles faster.
const array = (name, values) => {
  const rows = [];
  for (let start = 0; start < values.length; start += 32) rows.push(Array.from(values.slice(start, start + 32)).join(','));
  return `alignas(4) const uint16_t ${name}[] = {\n${rows.join(',\n')}};\n`;
};

// The DiscoveryId order: saves keep one bit per discovery by this index, so it never changes. Append only.
const topicOrder = ['space', 'earth', 'physics', 'nature', 'history', 'humanity', 'art', 'inventions', 'math', 'animals',
  'language', 'body'];
const ids = topicOrder.flatMap((topic) => Array.from({ length: 8 }, (_, i) => `${topic}-0${i + 1}`));
assert.deepEqual([...ids].sort(), Object.keys(discoveryPictures).sort(), 'Every picture needs a place in the order');
const pictures = [];
const pictureHashes = {};
ids.forEach((id, index) => {
  const ctx = raster(96, 48);
  renderDiscoveryArt(ctx, id);
  const pixels = ctx.finish();
  pictures.push(array(`picture_${index}`, pixels));
  pictureHashes[id] = hash(littleEndian(pixels));
});

const unique = new Map();
const sceneArrays = [];
const sceneHashes = [];
const table = stages.map((state, stage) => times.map((timeOfDay, time) => activities.map((activity, act) => frameTimes.map((t, frame) => {
  const ctx = raster(160, 160);
  const sleeping = activity === 'sleep';
  renderScene(ctx, { state: { ...state, sleeping }, time: sleeping ? sleepFrameTimes[frame] : t, activity, timeOfDay });
  const pixels = ctx.finish();
  const sha256 = hash(littleEndian(pixels));
  if (!unique.has(sha256)) {
    const runs = encode(pixels);
    unique.set(sha256, { name: `scene_${unique.size}`, runs });
    sceneArrays.push(array(unique.get(sha256).name, runs));
  }
  sceneHashes.push({ stage, time, activity: act, frame, sha256 });
  const { name, runs } = unique.get(sha256);
  return `{160,160,${name},${runs.length / 2}}`;
}))));

const enumName = (id) => `DISC_${id.toUpperCase().replace(/-/g, '_')}`;
const banner = '// AUTO-GENERATED by games/biscuit/tools/art/export-assets.mjs. Edit the generator, not this file.\n';
const scenesH = `${banner}#pragma once
#include <stdint.h>
#include "gfx565.h"

namespace biscuit {
// SCENES[stage][time][activity][frame]: 160x160 RGB565 runs, drawn at 3x. stage 0 Puppy, 1 Young pup, 2 Story dog;
// time 0 day, 1 evening, 2 night. Frames step every 230 ms (600 ms asleep). Play frames omit the movable ball.
constexpr int SCENE_STAGES = 3, SCENE_TIMES = 3, SCENE_FRAMES = 4, SCENE_W = 160, SCENE_H = 160;
enum SceneActivity : uint8_t {
${activities.map((a) => `  SCENE_${a.replace('trick-', '').toUpperCase()},`).join('\n')}
  SCENE_ACTIVITIES
};
extern const gfx565::RleImage SCENES[SCENE_STAGES][SCENE_TIMES][SCENE_ACTIVITIES][SCENE_FRAMES];
}  // namespace biscuit
`;
const discoveryH = `${banner}#pragma once
#include <stdint.h>
#include "gfx565.h"

namespace biscuit {
// 96x48 RGB565 discovery pictures, drawn at 3x. Same order as DISCOVERIES in content_discoveries.h; append only.
enum DiscoveryId : uint8_t {
${ids.map((id) => `  ${enumName(id)},`).join('\n')}
  DISCOVERY_COUNT
};
constexpr int DISCOVERY_W = 96, DISCOVERY_H = 48;
extern const gfx565::Image565 DISCOVERY_ART[DISCOVERY_COUNT];
}  // namespace biscuit
`;
const sceneRows = table.map((stage) => `{\n${stage.map((time) => `{\n${time.map((act) => `{${act.join(',')}}`).join(',\n')}}`).join(',\n')}}`).join(',\n');
const dataInc = `${banner}// Included once, by games/biscuit/art.cpp, so only one translation unit parses it. const: it stays in flash.
namespace biscuit {
namespace {
${pictures.join('')}${sceneArrays.join('')}}  // namespace
const gfx565::Image565 DISCOVERY_ART[DISCOVERY_COUNT] = {
${ids.map((_, i) => `{96,48,picture_${i}}`).join(',\n')}};
const gfx565::RleImage SCENES[SCENE_STAGES][SCENE_TIMES][SCENE_ACTIVITIES][SCENE_FRAMES] = {
${sceneRows}};
}  // namespace biscuit
`;
const pictureBytes = ids.length * 96 * 48 * 2;
const sceneBytes = [...unique.values()].reduce((sum, { runs }) => sum + runs.length * 2, 0);
const manifest = {
  generator: 'node games/biscuit/tools/art/export-assets.mjs',
  counts: { discoveries: ids.length, sceneReferences: sceneHashes.length, uniqueScenes: unique.size },
  storage: { pixelFormat: 'RGB565', byteOrder: 'little-endian', discoveryBytes: pictureBytes, sceneBytes },
  discoveries: pictureHashes,
  scenes: sceneHashes,
};

for (const [name, contents] of [['scenes.h', scenesH], ['discovery_art.h', discoveryH], ['art_data.inc', dataInc],
  ['manifest.json', `${JSON.stringify(manifest, null, 2)}\n`]]) {
  const destination = new URL(name, output);
  if (checkOnly) {
    const current = await readFile(destination, 'utf8').catch(() => '');
    assert(current === contents, `${fileURLToPath(destination)} is stale: run make -C apps/porthole art`);
  } else await writeFile(destination, contents);
}
console.log(`${checkOnly ? 'Verified' : 'Generated'} ${fileURLToPath(output)}: 96 pictures, ${sceneHashes.length} scenes `
  + `(${unique.size} unique), ${((pictureBytes + sceneBytes) / 1048576).toFixed(2)} MiB of pixels.`);
