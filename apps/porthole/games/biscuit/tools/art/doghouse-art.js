// The doghouse: the pup's own room, and the things a kid buys for it with stars. The room is three 160x160 bases (the
// pup at each stage, nothing bought); each thing is a picture of its box in the room with that thing in it, so drawn
// over a base it looks like it was always there. The pup's pixels never reach a thing's box (export-assets checks).
import { puppy, rect } from './art.js';

const INK = '#5a3d42';
const WALL = '#efe3de', STRIPE = '#e6d2d6', DOT = '#d6b6c5', SKIRTING = '#b58aa5', FLOOR = '#f7c6a5', PLANK = '#deaa8c';

export function doghouseRoom(ctx) {
  const r = (color, ...box) => rect(ctx, color, ...box);
  r(WALL, 0, 0, 160, 94);
  for (let x = 5; x < 160; x += 13) {
    r(STRIPE, x, 28, 1, 66);
    for (let y = 33; y < 94; y += 17) r(DOT, x + 5, y, 2, 2);
  }
  r(SKIRTING, 0, 94, 160, 4);
  r(FLOOR, 0, 98, 160, 62);
  for (let y = 106; y < 160; y += 13) {
    r(PLANK, 0, y, 160, 1);
    for (let x = (y % 2) * 23; x < 160; x += 46) r(PLANK, x, y - 12, 1, 12);
  }
}

export function doghousePup(ctx, stage) {
  rect(ctx, '#947d7b', 64, 108, 35, 3);
  puppy(ctx, 80, 78, { sleeping: false, blink: false, wag: true, activity: 'idle', stage, pose: '', phase: 0 });
}

// Save bits: append only. box is where it sits in the room, in the room's own pixels (drawn at 3x on the glass).
export const decor = [
  { id: 'ball', box: [136, 100, 14, 14], draw(r) {
    r('#73824a', 138, 101, 10, 12); r('#73824a', 137, 103, 12, 8);
    r('#deeb84', 139, 102, 8, 10); r('#cedb6b', 138, 104, 10, 6);
    r('#ffffce', 140, 103, 1, 3); r('#ffffce', 141, 106, 2, 2); r('#ffffce', 144, 108, 1, 3);
    r('#c5a284', 138, 113, 10, 1);
  } },
  { id: 'bowl', box: [110, 100, 24, 12], draw(r) {
    r(INK, 111, 103, 22, 2);
    r('#ce8e52', 114, 101, 16, 2); r('#f7c684', 116, 100, 3, 1); r('#f7c684', 124, 100, 3, 1);
    r('#7baaa5', 111, 105, 22, 4); r('#5a8e8c', 114, 109, 16, 2); r('#b5d7c5', 112, 105, 20, 1);
  } },
  { id: 'flags', box: [50, 32, 60, 14], draw(r) {
    r(INK, 50, 33, 60, 1);
    const colors = ['#de6d8c', '#efba84', '#94bead', '#b5a2ce', '#de6d8c', '#efba84', '#94bead'];
    colors.forEach((color, i) => {
      const x = 52 + i * 8;
      for (let row = 0; row < 8; row++) r(color, x + Math.floor(row / 2), 34 + row, Math.max(1, 7 - Math.floor(row / 2) * 2), 1);
    });
  } },
  { id: 'photo', box: [18, 40, 28, 22], draw(r) {
    r(INK, 19, 41, 26, 20); r('#efba84', 20, 42, 24, 18); r('#addbd6', 22, 44, 20, 14);
    r('#8cb6a5', 22, 53, 20, 5);
    r('#e6aa63', 25, 47, 6, 6); r(INK, 26, 49, 1, 1); r(INK, 29, 49, 1, 1); r('#a5694a', 24, 48, 2, 3); r('#a5694a', 30, 48, 2, 3);
    r('#f7c6a5', 34, 46, 5, 5); r('#84617b', 34, 45, 5, 2); r('#de6d8c', 33, 51, 7, 5);
  } },
  { id: 'plant', box: [140, 72, 16, 26], draw(r) {
    r('#637963', 147, 78, 2, 12);
    r('#73967b', 141, 79, 6, 3); r('#94b28c', 143, 75, 5, 3); r('#94b28c', 149, 77, 6, 3); r('#73967b', 148, 73, 4, 4);
    r('#73967b', 150, 82, 5, 3);
    r('#b57573', 142, 88, 12, 3); r('#d68e84', 143, 91, 10, 7); r('#e6aa94', 144, 92, 2, 5);
  } },
  { id: 'clock', box: [116, 34, 24, 24], draw(r) {
    r(INK, 120, 35, 16, 22); r(INK, 117, 38, 22, 16);
    r('#fff3d6', 121, 37, 14, 18); r('#fff3d6', 119, 40, 18, 12);
    r(INK, 127, 39, 2, 8); r(INK, 128, 45, 6, 2);
    for (const [x, y] of [[127, 37], [127, 53], [119, 46], [135, 46]]) r('#b58aa5', x, y, 2, 1);
    r('#de6d8c', 120, 34, 3, 2); r('#de6d8c', 133, 34, 3, 2);
  } },
  { id: 'poster', box: [112, 60, 26, 30], draw(r) {
    r(INK, 113, 61, 24, 28); r('#424d7b', 114, 62, 22, 26);
    const star = (x, y, c) => { r(c, x + 2, y, 1, 5); r(c, x, y + 2, 5, 1); r(c, x + 1, y + 1, 3, 3); };
    star(122, 66, '#ffdf9c'); star(116, 74, '#f7e3bd'); star(129, 76, '#ffdf9c'); star(120, 81, '#f7e3bd');
    r('#ffe3a5', 130, 64, 4, 4); r('#424d7b', 131, 64, 3, 3);
  } },
  { id: 'shelf', box: [10, 66, 38, 26], draw(r) {
    r('#84617b', 11, 86, 36, 3); r('#ad7d8c', 12, 85, 34, 1);
    r('#84617b', 14, 89, 3, 3); r('#84617b', 41, 89, 3, 3);
    r('#de6d8c', 13, 77, 7, 8); r('#ffdf9c', 15, 75, 3, 2);
    r('#94bead', 22, 73, 8, 12); r(INK, 24, 76, 1, 1); r(INK, 27, 76, 1, 1); r('#73967b', 22, 71, 2, 3); r('#73967b', 28, 71, 2, 3);
    r('#efba84', 32, 79, 6, 6); r('#b5a2ce', 39, 79, 6, 6); r('#f7e3c5', 34, 81, 2, 2); r('#f7e3c5', 41, 81, 2, 2);
  } },
  { id: 'bed', box: [10, 98, 46, 22], draw(r) {
    r(INK, 11, 104, 44, 14); r(INK, 13, 101, 40, 3);
    r('#b5a2ce', 12, 105, 42, 12); r('#b5a2ce', 14, 102, 38, 3);
    r('#e6dfef', 16, 106, 34, 7); r('#c5aee6', 16, 113, 34, 2);
    r('#de6d8c', 18, 108, 4, 3); r('#efba84', 30, 108, 4, 3); r('#94bead', 42, 108, 4, 3);
  } },
];
