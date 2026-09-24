#!/usr/bin/env node
// Run from any directory: node firmware/tools/export-assets.mjs [--check]
// Browser content and renderers remain the source of truth. No Canvas dependency.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { stories } from '../../src/stories.js';
import { discoveries, topics } from '../../src/discoveries.js';
import { TRICKS, STICKERS, dailyAdventure } from '../../src/game.js';
import { renderScene } from '../../src/art.js';
import { discoveryPictures, renderDiscoveryArt } from '../../src/discovery-art.js';

const output = new URL('../generated/', import.meta.url);
const checkOnly = process.argv.includes('--check');
assert(process.argv.slice(2).every((arg) => arg === '--check'), 'Only --check is supported');
const actionBits = { feed: 1, play: 2, pet: 4, read: 8, train: 16, rest: 32 };
const cues = ['up', 'down', 'left', 'right', 'paw'];
const activities = ['idle', 'feed', 'pet', 'read', 'play', 'sleep', ...TRICKS.map(({ id }) => `trick-${id}`)];
const times = ['day', 'evening', 'night'];
const stages = [{ daysTogether: 1, friendship: 0 }, { daysTogether: 3, friendship: 20 }, { daysTogether: 7, friendship: 60 }];
const frameTimes = [0, 230, 460, 690];
const sleepFrameTimes = [0, 600, 1200, 1800];
const flashBudget = 8 * 1024 * 1024;
const hash = (value) => createHash('sha256').update(value).digest('hex');
const strings = new Set();

function normalize(value) {
  assert.equal(typeof value, 'string');
  return value.replace(/[\u2018\u2019\u201a\u201b]/g, "'")
    .replace(/[\u201c\u201d\u201e\u201f]/g, '"')
    .replace(/[\u2010-\u2015\u2212]/g, '-')
    .replace(/\u2026/g, '...').replace(/\u00a0/g, ' ').replace(/\u00b7/g, '-');
}

function string(value) {
  const normalized = normalize(value);
  // biome-ignore lint/suspicious/noControlCharactersInRegex: deliberately detects control characters
  assert(!/[\x00-\x08\x0b\x0c\x0e-\x1f]/.test(normalized), 'Unsupported control character');
  strings.add(normalized);
  return JSON.stringify(normalized);
}

function raster(width, height) {
  const pixels = new Uint16Array(width * height);
  const painted = new Uint8Array(pixels.length);
  let color = 0;
  const stack = [];
  return {
    pixels,
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

function encode(pixels) {
  const runs = [];
  for (let i = 0; i < pixels.length;) {
    const color = pixels[i];
    let count = 1;
    while (count < 65535 && i + count < pixels.length && pixels[i + count] === color) count++;
    runs.push(count, color);
    i += count;
  }
  const decoded = [];
  for (let i = 0; i < runs.length; i += 2) {
    assert(runs[i] > 0 && runs[i] <= 65535);
    for (let n = 0; n < runs[i]; n++) decoded.push(runs[i + 1]);
  }
  assert.deepEqual(new Uint16Array(decoded), pixels, 'RLE must round-trip every pixel');
  return runs;
}

function array(name, values) {
  const rows = [];
  for (let start = 0; start < values.length; start += 16) {
    rows.push(`  ${Array.from(values.slice(start, start + 16), (value) => `0x${value.toString(16).padStart(4, '0')}`).join(', ')},`);
  }
  return `alignas(4) const uint16_t ${name}[] = {\n${rows.join('\n')}\n};\n`;
}

assert.equal(stories.length, 7);
assert.equal(discoveries.length, 96);
assert.equal(topics.length, 12);
assert.equal(TRICKS.length, 6);
assert.equal(STICKERS.length, 12);
assert.equal(new Set(stories.map(({ id }) => id)).size, 7);
assert.equal(new Set(discoveries.map(({ id }) => id)).size, 96);
assert.equal(new Set(topics.map(({ id }) => id)).size, 12);
for (const topic of topics) assert.equal(discoveries.filter((fact) => fact.topic === topic.id).length, 8);
for (const story of stories) {
  assert.equal(story.choices.length, 2);
  assert(story.pages.length && story.choices.every((choice) => choice.ending.length));
}
for (const fact of discoveries) {
  assert.equal(fact.pages.length, 2);
  assert(discoveryPictures[fact.id], `Missing picture: ${fact.id}`);
  assert.match(fact.source.url, /^https:\/\//);
}
for (const trick of TRICKS) {
  assert.equal(trick.lessons.length, 3);
  for (const lesson of trick.lessons) {
    assert(lesson.length >= 3 && lesson.length <= 6);
    assert(lesson.every((cue) => cues.includes(cue)));
  }
}

// Local noon constructs the requested calendar date in every host timezone.
// 1970-01-01 has ordinal zero, matching the browser's adventure rotation.
const adventures = Array.from({ length: 7 }, (_, day) => dailyAdventure(new Date(1970, 0, day + 1, 12).getTime()));
assert.equal(new Set(adventures.map(({ title }) => title)).size, 7);
for (const adventure of adventures) {
  assert.equal(new Set(adventure.actions).size, 3);
  assert(adventure.actions.every((action) => action in actionBits));
}

const storyData = stories.map((story) => `  {${string(story.id)}, ${string(story.title)}, ${story.unlockDay}, ${string(story.pages.join('\n\n'))}, ${string(story.prompt)}, {${story.choices.map((choice) => `{${string(choice.label)}, ${string(choice.ending.join('\n\n'))}}`).join(', ')}}},`).join('\n');
const discoveryData = discoveries.map((fact) => `  {${[fact.id, fact.topic, fact.title, fact.pages.join('\n\n'), fact.wonder, fact.source.name, fact.source.url].map(string).join(', ')}},`).join('\n');
const topicData = topics.map((topic) => `  {${string(topic.id)}, ${string(topic.name)}},`).join('\n');
const trickData = TRICKS.map((trick) => `  {${string(trick.id)}, ${string(trick.name)}, ${trick.day}, {${trick.lessons.map((lesson) => lesson.length).join(', ')}}, {${trick.lessons.map((lesson) => `{${lesson.map((cue) => `Cue::${cue[0].toUpperCase()}${cue.slice(1)}`).join(', ')}}`).join(', ')}}},`).join('\n');
const stickerData = STICKERS.map((sticker) => `  ${string(sticker)},`).join('\n');
const adventureData = adventures.map((adventure) => `  {${[adventure.title, adventure.description, adventure.word, adventure.meaning].map(string).join(', ')}, ${adventure.actions.reduce((bits, action) => bits | actionBits[action], 0)}},`).join('\n');

const imageDefinitions = [];
const discoveryMetadata = [];
const imageHashes = {};
for (const [index, fact] of discoveries.entries()) {
  const ctx = raster(96, 48);
  renderDiscoveryArt(ctx, fact.id);
  const pixels = ctx.finish();
  imageDefinitions.push(array(`discovery_${index}`, pixels));
  discoveryMetadata.push(`  {96, 48, discovery_${index}},`);
  imageHashes[fact.id] = hash(littleEndian(pixels));
}

const uniqueScenes = new Map();
const sceneHashes = [];
const sceneMetadata = stages.map((state, stageIndex) => times.map((timeOfDay, timeIndex) => activities.map((activity, activityIndex) => frameTimes.map((time, frame) => {
  const ctx = raster(160, 160);
  const sleeping = activity === 'sleep';
  renderScene(ctx, { state: { ...state, sleeping }, time: sleeping ? sleepFrameTimes[frame] : time, activity, timeOfDay });
  const pixels = ctx.finish();
  const digest = hash(littleEndian(pixels));
  if (!uniqueScenes.has(digest)) {
    const name = `scene_${uniqueScenes.size}`;
    const runs = encode(pixels);
    uniqueScenes.set(digest, { name, runs });
    imageDefinitions.push(array(name, runs));
  }
  const { name, runs } = uniqueScenes.get(digest);
  sceneHashes.push({ stage: stageIndex, time: timeIndex, activity: activityIndex, frame, sha256: digest });
  return `{160, 160, ${name}, ${runs.length / 2}}`;
}))));
assert.equal(sceneHashes.length, 432);
const sceneData = sceneMetadata.map((stage) => `  {\n${stage.map((time) => `    {\n${time.map((activity) => `      {${activity.join(', ')}},`).join('\n')}\n    },`).join('\n')}\n  },`).join('\n');

const header = `// Generated by firmware/tools/export-assets.mjs. Do not edit.\n#pragma once
#include <stddef.h>
#include <stdint.h>

namespace biscuitassets {
inline constexpr size_t StoryCount = 7, DiscoveryCount = 96, TopicCount = 12;
inline constexpr size_t TrickCount = 6, StickerCount = 12, AdventureCount = 7;
inline constexpr size_t StageCount = 3, TimeCount = 3, ActivityCount = 12, FrameCount = 4;
inline constexpr size_t ScenePixelCount = 160 * 160, DiscoveryPixelCount = 96 * 48;
enum class Cue : uint8_t { Up, Down, Left, Right, Paw };
enum class Stage : uint8_t { Puppy, YoungPup, StoryDog };
enum class TimeOfDay : uint8_t { Day, Evening, Night };
enum class Activity : uint8_t { Idle, Feed, Pet, Read, Play, Sleep, Sit, Paw, Spin, Bow, Jump, Roll };
enum Action : uint8_t { Feed = 1, Play = 2, Pet = 4, Read = 8, Train = 16, Rest = 32 };
struct Choice { const char* label; const char* ending; };
struct Story { const char* id; const char* title; uint8_t unlockDay; const char* opening; const char* prompt; Choice choices[2]; };
struct Discovery { const char* id; const char* topic; const char* title; const char* text; const char* wonder; const char* sourceName; const char* sourceUrl; };
struct Topic { const char* id; const char* name; };
struct Trick { const char* id; const char* name; uint8_t unlockDay; uint8_t lessonLengths[3]; Cue lessons[3][6]; };
struct Adventure { const char* title; const char* description; const char* word; const char* meaning; uint8_t actions; };
// Native RGB565 words, little endian on ESP32; use LV_COLOR_DEPTH=16 and LV_COLOR_16_SWAP=0.
struct Image565 { uint16_t width; uint16_t height; const uint16_t* pixels; };
// Runs are alternating nonzero count,color uint16_t pairs; runCount counts pairs.
struct RleImage { uint16_t width; uint16_t height; const uint16_t* runs; uint32_t runCount; };
extern const Story stories[StoryCount];
extern const Discovery discoveries[DiscoveryCount];
extern const Topic topics[TopicCount];
extern const Trick tricks[TrickCount];
extern const char* const stickers[StickerCount];
// Index = normalized local calendar-day ordinal modulo 7; sticker index is ordinal modulo 12.
extern const Adventure adventures[AdventureCount];
extern const Image565 discoveryImages[DiscoveryCount];
// Indexed [Stage][TimeOfDay][Activity][frame]. Play frames omit the movable tennis ball.
extern const RleImage scenes[StageCount][TimeCount][ActivityCount][FrameCount];
bool decodeScene(const RleImage& image, uint16_t* destination, size_t pixelCapacity);
} // namespace biscuitassets
`;

const cpp = `// Generated by firmware/tools/export-assets.mjs. Do not edit.
#include "assets.h"
namespace biscuitassets {
const Story stories[StoryCount] = {\n${storyData}\n};
const Discovery discoveries[DiscoveryCount] = {\n${discoveryData}\n};
const Topic topics[TopicCount] = {\n${topicData}\n};
const Trick tricks[TrickCount] = {\n${trickData}\n};
const char* const stickers[StickerCount] = {\n${stickerData}\n};
const Adventure adventures[AdventureCount] = {\n${adventureData}\n};
namespace {\n${imageDefinitions.join('\n')}\n} // namespace
const Image565 discoveryImages[DiscoveryCount] = {\n${discoveryMetadata.join('\n')}\n};
const RleImage scenes[StageCount][TimeCount][ActivityCount][FrameCount] = {\n${sceneData}\n};

bool decodeScene(const RleImage& image, uint16_t* destination, size_t pixelCapacity) {
  const size_t total = static_cast<size_t>(image.width) * image.height;
  if (!destination || !image.runs || !total || pixelCapacity < total || !image.runCount || image.runCount > total) return false;
  size_t offset = 0;
  for (uint32_t run = 0; run < image.runCount; ++run) {
    const uint16_t count = image.runs[run * 2];
    const uint16_t color = image.runs[run * 2 + 1];
    if (!count || count > total - offset) return false;
    for (uint16_t n = 0; n < count; ++n) destination[offset++] = color;
  }
  return offset == total;
}
} // namespace biscuitassets
`;

const discoveryBytes = 96 * 96 * 48 * 2;
const sceneBytes = [...uniqueScenes.values()].reduce((sum, { runs }) => sum + runs.length * 2, 0);
const textBytes = [...strings].reduce((sum, value) => sum + Buffer.byteLength(value, 'utf8') + 1, 0);
// ESP32 pointers are 32 bits; round structs to 4-byte alignment.
const tableBytes = 7 * 40 + 96 * 28 + 12 * 8 + 6 * 32 + 12 * 4 + 7 * 20 + 96 * 8 + 432 * 12;
const estimatedFlashBytes = discoveryBytes + sceneBytes + textBytes + tableBytes;
assert(estimatedFlashBytes < flashBudget, `Asset payload exceeds 8 MiB: ${estimatedFlashBytes}`);
const manifest = {
  format: 1,
  generator: 'node firmware/tools/export-assets.mjs',
  counts: { stories: 7, discoveries: 96, topics: 12, tricks: 6, stickers: 12, adventures: 7, sceneReferences: 432, uniqueScenes: uniqueScenes.size },
  dimensions: { scene: [160, 160], discovery: [96, 48] },
  sceneOrder: { stages: ['Puppy', 'YoungPup', 'StoryDog'], times, activities, frameTimes, sleepFrameTimes },
  storage: { pixelFormat: 'RGB565', byteOrder: 'little-endian', discoveryBytes, sceneBytes, textBytes, tableBytes, estimatedFlashBytes, budgetBytes: flashBudget },
  // biome-ignore lint/suspicious/noControlCharactersInRegex: deliberately detects control characters
  retainedNonAscii: [...new Set([...strings].join('').match(/[^\x00-\x7f]/gu) ?? [])].sort(),
  files: { 'assets.h': hash(header), 'assets.cpp': hash(cpp) },
  discoveries: imageHashes,
  scenes: sceneHashes,
};

if (!checkOnly) await mkdir(output, { recursive: true });
for (const [name, contents] of [['assets.h', header], ['assets.cpp', cpp], ['manifest.json', `${JSON.stringify(manifest, null, 2)}\n`]]) {
  const destination = new URL(name, output);
  const existing = await readFile(destination, 'utf8').catch(() => null);
  if (checkOnly) assert.equal(existing, contents, `${name} is stale; regenerate firmware assets`);
  // Leave unchanged files alone so the PlatformIO pre-build hook does not force a 15 MB recompile.
  else if (existing !== contents) await writeFile(destination, contents);
}
console.log(`${checkOnly ? 'Verified' : 'Generated'} ${fileURLToPath(output)}: 7 stories, 96 discoveries, 432 scene references / ${uniqueScenes.size} unique scenes; estimated ${(estimatedFlashBytes / 1024 / 1024).toFixed(2)} MiB flash.`);
