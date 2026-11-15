#!/usr/bin/env node
// Biscuit's scene and discovery art -> games/biscuit/generated/. Node 22 (pinned in /.tool-versions: the art is zlib-packed, and a Node on another zlib build packs different bytes), no npm packages, no Canvas.
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
import { deflateSync, inflateSync } from 'node:zlib';
import { rect, renderScene } from './art.js';
import { discoveryPictures, renderDiscoveryArt } from './discovery-art.js';
import { decor, doghousePup, doghouseRoom } from './doghouse-art.js';

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

// A 16 KiB window preserves row-to-row repetition while keeping the compressed payload small.
function encode(pixels) {
  const raw = littleEndian(pixels);
  const packed = deflateSync(raw, { level: 9, windowBits: 14 });
  assert.deepEqual(inflateSync(packed), raw, 'Deflate must round-trip every pixel');
  return packed;
}

// Decimal, no padding: compact source and fast compilation.
const array = (name, values) => {
  const rows = [];
  for (let start = 0; start < values.length; start += 32) rows.push(Array.from(values.slice(start, start + 32)).join(','));
  return `alignas(4) const uint8_t ${name}[] = {\n${rows.join(',\n')}};\n`;
};

// The DiscoveryId order: saves keep one bit per discovery by this index, so it never changes. Append only.
const topicOrder = ['space', 'earth', 'physics', 'nature', 'history', 'humanity', 'art', 'inventions', 'math', 'animals',
  'language', 'body'];
const ids = topicOrder.flatMap((topic) => Array.from({ length: 8 }, (_, i) => `${topic}-0${i + 1}`));
assert.deepEqual([...ids].sort(), Object.keys(discoveryPictures).sort(), 'Every picture needs a place in the order');
const pictures = [];
const pictureBytesById = [];
const pictureHashes = {};
ids.forEach((id, index) => {
  const ctx = raster(96, 48);
  renderDiscoveryArt(ctx, id);
  const pixels = ctx.finish();
  const packed = encode(pixels);
  pictures.push(array(`picture_${index}`, packed));
  pictureBytesById.push(packed.length);
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
    const packed = encode(pixels);
    unique.set(sha256, { name: `scene_${unique.size}`, packed });
    sceneArrays.push(array(unique.get(sha256).name, packed));
  }
  sceneHashes.push({ stage, time, activity: act, frame, sha256 });
  const { name, packed } = unique.get(sha256);
  return `{160,160,${name},${packed.length}}`;
}))));

// The doghouse: a base per stage, then each thing as the room's box around it with the thing in it (doghouse-art.js).
const roomPixels = (draw) => { const ctx = raster(160, 160); doghouseRoom(ctx); draw(ctx); return ctx.finish(); };
const empty = roomPixels(() => {});
const crop = (pixels, [x, y, w, h]) => {
  const out = new Uint16Array(w * h);
  for (let row = 0; row < h; row++) out.set(pixels.subarray((y + row) * 160 + x, (y + row) * 160 + x + w), row * w);
  return out;
};
const same = (a, b) => a.length === b.length && a.every((v, i) => v === b[i]);
const inside = ([x, y, w, h], i) => { const px = i % 160, py = Math.floor(i / 160); return px >= x && px < x + w && py >= y && py < y + h; };
const doghouseArrays = [];
const doghouseHashes = { rooms: [], decor: {} };
const bases = stages.map((_, stage) => roomPixels((ctx) => doghousePup(ctx, stage)));
bases.forEach((pixels, stage) => {
  const packed = encode(pixels);
  doghouseArrays.push(array(`doghouse_room_${stage}`, packed));
  doghouseHashes.rooms.push({ stage, sha256: hash(littleEndian(pixels)), bytes: packed.length });
});
const decorRows = decor.map((item, index) => {
  const [x, y, w, h] = item.box;
  assert(x >= 0 && y >= 0 && x + w <= 160 && y + h <= 160, `${item.id} must sit inside the room`);
  decor.slice(0, index).forEach((other) => {
    const [ox, oy, ow, oh] = other.box;
    assert(x >= ox + ow || ox >= x + w || y >= oy + oh || oy >= y + h, `${item.id} overlaps ${other.id}`);
  });
  const full = roomPixels((ctx) => item.draw((color, ...box) => rect(ctx, color, ...box)));
  assert(full.every((v, i) => inside(item.box, i) || v === empty[i]), `${item.id} draws outside its box`);
  bases.forEach((base, stage) => assert(same(crop(base, item.box), crop(empty, item.box)), `the pup (stage ${stage}) reaches into ${item.id}`));
  const pixels = crop(full, item.box);
  const packed = encode(pixels);
  doghouseArrays.push(array(`decor_${index}`, packed));
  doghouseHashes.decor[item.id] = hash(littleEndian(pixels));
  return `{${x},${y},{${w},${h},decor_${index},${packed.length}}}`;
});

const enumName = (id) => `DISC_${id.toUpperCase().replace(/-/g, '_')}`;
const banner = '// AUTO-GENERATED by games/biscuit/tools/art/export-assets.mjs. Edit the generator, not this file.\n';
const scenesH = `${banner}#pragma once
#include <stdint.h>
#include "packed_image.h"

namespace biscuit {
// SCENES[stage][time][activity][frame]: 160x160 lossless zlib RGB565, drawn at 3x. stage 0 Puppy, 1 Young pup, 2 Story dog;
// time 0 day, 1 evening, 2 night. Frames step every 230 ms (600 ms asleep). Play frames omit the movable ball.
constexpr int SCENE_STAGES = 3, SCENE_TIMES = 3, SCENE_FRAMES = 4, SCENE_W = 160, SCENE_H = 160;
enum SceneActivity : uint8_t {
${activities.map((a) => `  SCENE_${a.replace('trick-', '').toUpperCase()},`).join('\n')}
  SCENE_ACTIVITIES
};
extern const packed565::Image SCENES[SCENE_STAGES][SCENE_TIMES][SCENE_ACTIVITIES][SCENE_FRAMES];
}  // namespace biscuit
`;
const discoveryH = `${banner}#pragma once
#include <stdint.h>
#include "packed_image.h"

namespace biscuit {
// 96x48 lossless zlib RGB565 discovery pictures, drawn at 1x or 3x. Same order as DISCOVERIES; append only.
enum DiscoveryId : uint8_t {
${ids.map((id) => `  ${enumName(id)},`).join('\n')}
  DISCOVERY_COUNT
};
constexpr int DISCOVERY_W = 96, DISCOVERY_H = 48;
extern const packed565::Image DISCOVERY_ART[DISCOVERY_COUNT];
}  // namespace biscuit
`;
const doghouseH = `${banner}#pragma once
#include <stdint.h>
#include "packed_image.h"

namespace biscuit {
// The doghouse: DOGHOUSE_ROOMS[stage] is the room with the pup and nothing bought, 160x160 drawn at 3x like a scene;
// each thing is its box of the room with the thing in it, drawn over the room at (x, y) (room pixels). In Save::decor
// bit order; append only.
enum DecorId : uint8_t {
${decor.map(({ id }) => `  DECOR_${id.toUpperCase()},`).join('\n')}
  DECOR_COUNT
};
struct DecorArt { int16_t x, y; packed565::Image image; };
extern const packed565::Image DOGHOUSE_ROOMS[SCENE_STAGES_DOGHOUSE];
extern const DecorArt DECOR_ART[DECOR_COUNT];
}  // namespace biscuit
`.replace('SCENE_STAGES_DOGHOUSE', String(stages.length));
const sceneRows = table.map((stage) => `{\n${stage.map((time) => `{\n${time.map((act) => `{${act.join(',')}}`).join(',\n')}}`).join(',\n')}}`).join(',\n');
// Keep generated source chunks comfortably below the repository's 500 KB file limit.
const chunks = [];
for (const source of [...pictures, ...sceneArrays, ...doghouseArrays]) {
  if (!chunks.length || chunks[chunks.length - 1].length + source.length > 400000) chunks.push('');
  chunks[chunks.length - 1] += source;
}
const chunkFiles = chunks.map((source, i) => [`art_data_${i}.inc`, `${banner}${source}`]);
const dataInc = `${banner}// Included once, by games/biscuit/art.cpp, so only one translation unit parses it. const: it stays in flash.
namespace biscuit {
namespace {
${chunkFiles.map(([name]) => `#include "${name}"`).join('\n')}
}  // namespace
const packed565::Image DISCOVERY_ART[DISCOVERY_COUNT] = {
${ids.map((_, i) => `{96,48,picture_${i},${pictureBytesById[i]}}`).join(',\n')}};
const packed565::Image SCENES[SCENE_STAGES][SCENE_TIMES][SCENE_ACTIVITIES][SCENE_FRAMES] = {
${sceneRows}};
const packed565::Image DOGHOUSE_ROOMS[${stages.length}] = {
${doghouseHashes.rooms.map(({ stage, bytes }) => `{160,160,doghouse_room_${stage},${bytes}}`).join(',\n')}};
const DecorArt DECOR_ART[DECOR_COUNT] = {
${decorRows.join(',\n')}};
}  // namespace biscuit
`;
const pictureBytes = pictureBytesById.reduce((sum, count) => sum + count, 0);
const sceneBytes = [...unique.values()].reduce((sum, { packed }) => sum + packed.length, 0);
const manifest = {
  generator: 'node games/biscuit/tools/art/export-assets.mjs',
  counts: { discoveries: ids.length, sceneReferences: sceneHashes.length, uniqueScenes: unique.size },
  storage: { pixelFormat: 'RGB565', byteOrder: 'little-endian', compression: 'zlib', windowBytes: 16384, discoveryBytes: pictureBytes, sceneBytes },
  discoveries: pictureHashes,
  scenes: sceneHashes,
  doghouse: { rooms: doghouseHashes.rooms.map(({ stage, sha256 }) => ({ stage, sha256 })), decor: doghouseHashes.decor },
};

for (const [name, contents] of [['scenes.h', scenesH], ['discovery_art.h', discoveryH], ['doghouse_art.h', doghouseH], ['art_data.inc', dataInc],
  ['manifest.json', `${JSON.stringify(manifest, null, 2)}\n`], ...chunkFiles]) {
  const destination = new URL(name, output);
  if (checkOnly) {
    const current = await readFile(destination, 'utf8').catch(() => '');
    assert(current === contents, `${fileURLToPath(destination)} is stale: run make -C apps/porthole art`);
  } else await writeFile(destination, contents);
}
console.log(`${checkOnly ? 'Verified' : 'Generated'} ${fileURLToPath(output)}: 96 pictures, ${sceneHashes.length} scenes `
  + `(${unique.size} unique), ${((pictureBytes + sceneBytes) / 1048576).toFixed(2)} MiB of pixels.`);
