// Palette expanded from RGB565 levels; art remains a 160px grid enlarged exactly 3×.
const INK = '#5a3d42';

function rect(ctx, color, x, y, w, h) {
  ctx.fillStyle = color;
  ctx.fillRect(Math.round(x), Math.round(y), w, h);
}

function heart(ctx, x, y, color = '#de6d8c') {
  rect(ctx, color, x + 1, y, 2, 2);
  rect(ctx, color, x + 4, y, 2, 2);
  rect(ctx, color, x, y + 2, 7, 2);
  rect(ctx, color, x + 1, y + 4, 5, 1);
  rect(ctx, color, x + 2, y + 5, 3, 1);
  rect(ctx, color, x + 3, y + 6, 1, 1);
}

function room(ctx, timeOfDay, { activity = 'idle', phase = 0, sleeping = false } = {}) {
  const r = (color, ...box) => rect(ctx, color, ...box);
  const night = timeOfDay === 'night';
  const evening = timeOfDay === 'evening';
  r(night ? '#635d84' : evening ? '#efd2d6' : '#efe3de', 0, 0, 160, 98);
  for (let x = 5; x < 160; x += 13) {
    r(night ? '#6b658c' : '#e6d2d6', x, 28, 1, 66);
    for (let y = 33; y < 94; y += 17) r(night ? '#8c759c' : '#d6b6c5', x + 5, y, 2, 2);
  }
  r(night ? '#524963' : '#b58aa5', 0, 94, 160, 4);
  r(night ? '#9c7d8c' : evening ? '#e6b29c' : '#f7c6a5', 0, 98, 160, 62);
  for (let y = 106; y < 160; y += 13) {
    r(night ? '#8c7184' : '#deaa8c', 0, y, 160, 1);
    for (let x = (y % 2) * 23; x < 160; x += 46) r(night ? '#8c7184' : '#deaa8c', x, y - 12, 1, 12);
  }

  // A little library, with alternating book heights and bright cloth spines.
  r('#84617b', 15, 48, 36, 48);
  r('#ad7d8c', 17, 50, 32, 43);
  r('#6b4d6b', 19, 53, 28, 17);
  r('#6b4d6b', 19, 74, 28, 17);
  const books = ['#94bead', '#efba84', '#b5a2ce', '#ef9e94', '#bdc68c'];
  for (let shelf = 0; shelf < 2; shelf++) {
    for (let i = 0; i < 5; i++) {
      const h = 11 + ((i + shelf) % 3) * 2;
      const x = 20 + i * 5;
      const y = 69 + shelf * 21 - h;
      r(books[(i + shelf * 2) % books.length], x, y, 4, h);
      r('#f7e3c5', x + 1, y + 2, 2, 1);
    }
  }
  r('#deb6ad', 15, 70, 36, 3);
  r('#deb6ad', 15, 91, 36, 3);
  r('#84617b', 18, 94, 4, 4);
  r('#84617b', 44, 94, 4, 4);
  if (activity === 'shelf') {
    const slide = [0, 2, 6, 9][phase];
    r(INK, 38 + slide, 50, 7, 20);
    r('#b5a2ce', 39 + slide, 51, 5, 18);
    r('#fff3d6', 40 + slide, 54, 3, 2);
  } else if (activity === 'idle' && phase === 2 && !sleeping) {
    r('#fff3d6', 35, 53, 2, 4);
    r('#fff3d6', 34, 54, 4, 2);
  }

  // Stepped arch window: no antialiased curves in this little world.
  r('#a586a5', 109, 42, 32, 38);
  r('#a586a5', 113, 38, 24, 4);
  r('#fff3d6', 112, 44, 26, 33);
  r('#fff3d6', 116, 41, 18, 3);
  r(night ? '#424d7b' : evening ? '#e6b6b5' : '#addbd6', 114, 45, 22, 30);
  r(night ? '#424d7b' : evening ? '#e6b6b5' : '#addbd6', 117, 43, 16, 2);
  if (night) {
    r('#ffdf9c', 128, 49, 5, 7);
    r('#424d7b', 126, 48, 5, 5);
    r('#f7e3bd', 119, 52, 1, 1);
    r('#f7e3bd', 132, 63, 1, 1);
  } else {
    r('#ffe3a5', 126, evening ? 66 : 49, 6, 6);
    r('#f7f7de', 116, 58, 10, 3);
    r('#f7f7de', 120, 56, 5, 2);
    r('#8cb6a5', 114, 69, 22, 6);
    r('#8cb6a5', 119, 66, 9, 3);
  }
  r('#fff3d6', 124, 43, 2, 34);
  r('#fff3d6', 112, 61, 26, 2);
  r('#b596b5', 106, 43, 5, 33);
  r('#b596b5', 139, 43, 5, 33);
  r('#ceb2c5', 107, 44, 2, 28);
  r('#ceb2c5', 140, 44, 2, 28);
  r('#fff3d6', 108, 78, 34, 3);
  if (sleeping) {
    const sway = [0, 1, 1, 0][phase];
    r('#84617b', 111, 44, 11 + sway, 32);
    r('#84617b', 130 - sway, 44, 11 + sway, 32);
    r('#ad7d8c', 112, 45, 3, 28);
    r('#ad7d8c', 136, 45, 3, 28);
  } else if (activity === 'idle' && phase === 1) {
    r('#fff3d6', 120, 49, 1, 5);
    r('#fff3d6', 118, 51, 5, 1);
  }

  // Patchwork rug and a fern keep the room cozy even on sleepy days.
  r(night ? '#73617b' : '#bd92ad', 45, 107, 72, 13);
  r(night ? '#73617b' : '#bd92ad', 50, 104, 62, 19);
  r(night ? '#94829c' : '#e6bece', 50, 109, 62, 9);
  r(night ? '#94829c' : '#e6bece', 55, 107, 52, 14);
  for (let x = 54; x < 112; x += 9) r('#cea2bd', x, 116, 4, 1);
  const sway = activity === 'fern' ? [0, 2, -2, 1][phase] : activity === 'idle' && phase === 3 ? 1 : 0;
  r('#637963', 132, 85, 2, 11);
  r('#73967b', 125 + sway, 86, 7, 3);
  r('#94b28c', 122 + sway, 83, 7, 3);
  r('#94b28c', 134 + sway, 85, 7, 3);
  r('#73967b', 136 + sway, 82, 6, 3);
  r('#73967b', 131 + sway, 80, 3, 6);
  r('#b57573', 126, 93, 14, 3);
  r('#d68e84', 128, 96, 10, 9);
  r('#e6aa94', 129, 97, 2, 6);
}

function meadow(ctx, timeOfDay) {
  const r = (color, ...box) => rect(ctx, color, ...box);
  const night = timeOfDay === 'night';
  r(night ? '#636d94' : timeOfDay === 'evening' ? '#e6bebd' : '#b5dfd6', 0, 0, 160, 98);
  r('#f7f3d6', 26, 47, 24, 5);
  r('#f7f3d6', 31, 43, 14, 4);
  r('#f7f3d6', 110, 61, 26, 5);
  r('#f7f3d6', 117, 57, 12, 4);
  r('#a5c69c', 0, 82, 160, 18);
  r('#a5c69c', 0, 76, 25, 6);
  r('#a5c69c', 126, 77, 34, 5);
  r(night ? '#94a68c' : '#c5d7a5', 0, 98, 160, 62);
  r('#b5ca8c', 0, 98, 160, 2);
  for (let x = 15; x < 153; x += 19) {
    r('#f7ebce', x, 84, 3, 20);
    r('#efd7bd', x, 84, 1, 20);
  }
  r('#f7ebce', 0, 89, 160, 3);
  for (const [x, y] of [[26, 110], [119, 114], [39, 100], [144, 98]]) {
    r('#94b67b', x, y, 1, 5);
    r('#f7ebb5', x - 2, y - 2, 5, 1);
    r('#f7ebb5', x, y - 4, 1, 5);
    r('#d6a663', x, y - 2, 1, 1);
  }
}

function puppy(ctx, x, y, { sleeping, blink, wag, activity, stage, pose, phase }) {
  let headX = 0;
  let headY = 0;
  const r = (color, a, b, w, h) => {
    a += headX;
    b += headY;
    if (pose === 'spin') {
      if (phase >= 2) a = -a - w;
      if (phase % 2) { a = Math.round(a / 2); w = Math.max(1, Math.round(w / 2)); }
    }
    // Quarter turns keep the roll crisp on the pixel grid.
    if (pose === 'roll') {
      if (phase === 1) [a, b, w, h] = [10 - b - h, a + 10, h, w];
      if (phase === 2) [a, b] = [-a - w, 20 - b - h];
      if (phase === 3) [a, b, w, h] = [b - 10, 10 - a - w, h, w];
    }
    rect(ctx, color, x + a, y + b, w, h);
  };
  const fur = '#e6aa63';
  const light = '#f7c684';
  // Tail and body sit behind the head, so floppy ears read clearly.
  r(INK, 15, wag ? 10 : 16, 5, 13);
  r(INK, 19, wag ? 6 : 13, 5, 9);
  r(fur, 17, wag ? 12 : 18, 2, 8);
  r(light, 20, wag ? 8 : 15, 2, 5);
  r(INK, pose === 'sit' ? -11 : -13, pose === 'sit' ? 8 : 12, pose === 'sit' ? 24 : 28, pose === 'sit' ? 22 : 18);
  r(INK, -16, 23, 34, 7);
  r(fur, -11, 13, 24, 15);
  r(light, -5, 16, 12, 13);
  r('#ffdfa5', -3, 18, 8, 8);
  const leftPaw = pose === 'sit' ? -11 : pose === 'bow' ? -25 : -15;
  const rightPaw = pose === 'sit' ? 2 : 5;
  r(INK, leftPaw, 27, 13, 5);
  r(INK, rightPaw, 27, 13, 5);
  r(light, leftPaw + 2, 26, 9, 4);
  r(light, rightPaw + 2, 26, 9, 4);
  r('#c58a52', leftPaw + 7, 28, 1, 2);
  r('#c58a52', rightPaw + 7, 28, 1, 2);
  if (stage === 2) {
    r(INK, 6, 17, 13, 13);
    r('#b57952', 7, 18, 11, 11);
    r('#dea273', 7, 18, 11, 4);
    r('#ffdfa5', 12, 22, 2, 3);
    r('#94614a', 3, 15, 3, 3);
    r('#94614a', 5, 18, 3, 4);
  }

  headX = activity === 'feed' ? [0, 4, 8, 5][phase]
    : activity === 'fern' ? [0, 3, 7, 4][phase]
      : activity === 'shelf' ? [0, -2, -5, -3][phase] : pose === 'bow' ? -6 : 0;
  headY = activity === 'feed' ? [0, 2, 5, 1][phase] : pose === 'bow' ? 5 + phase * 3 : pose === 'sit' ? -3 : 0;

  r(INK, -13, -14, 26, 3);
  r(INK, -17, -11, 34, 25);
  r(INK, -13, 14, 26, 3);
  r(fur, -14, -9, 28, 22);
  r(light, -11, -11, 22, 25);
  r('#ffdb94', -7, -10, 14, 6);
  r('#ffdb94', -9, -7, 18, 5);
  for (const side of [-1, 1]) {
    const ear = side < 0 ? -21 : 12;
    r(INK, ear, -7, 9, 23);
    r(INK, ear + 2, 16, 6, 4);
    r('#a5694a', ear + 2, -5, 5, 20);
    r('#bd8252', ear + 2, -5, 2, 16);
    r('#845542', ear + 4, 6, 2, 10);
  }
  const facingBack = pose === 'spin' && phase === 2;
  if (!facingBack && (sleeping || blink)) {
    r(INK, -9, 3, 5, 2);
    r(INK, 5, 3, 5, 2);
  } else if (!facingBack) {
    r(INK, -9, 0, 5, 6);
    r(INK, 5, 0, 5, 6);
    r('#fff3ce', -8, 0, 2, 2);
    r('#fff3ce', 6, 0, 2, 2);
  }
  if (!facingBack) {
    r('#e68e73', -12, 7, 5, 3);
    r('#e68e73', 8, 7, 5, 3);
    r('#ffe3ad', -8, 7, 17, 6);
    r('#ffe3ad', -5, 12, 11, 3);
    r(INK, -3, 6, 7, 3);
    r(INK, -1, 9, 3, 2);
    r(INK, -4, 12, 4, 1);
    r(INK, 2, 12, 4, 1);
    if (activity === 'feed') {
      r(INK, -3, 12, 6, phase % 2 ? 2 : 1);
      if (phase === 1 || phase === 3) r('#d67584', -1, 13, 3, 2);
    } else if (!sleeping && activity !== 'read') r('#d67584', 0, 12, 2, 3);
  }
  r(stage ? '#a58ec5' : '#528e8c', -10, 16, 21, 3);
  if (stage) {
    r('#c5aee6', -5, 19, 11, 2);
    r('#c5aee6', -3, 21, 7, 2);
    r('#c5aee6', -1, 23, 3, 2);
  }
  r('#f7d78c', -2, 18, 5, 3);
  if (stage === 2) {
    r('#ffefb5', 0, 16, 1, 7);
    r('#ffefb5', -3, 18, 7, 1);
    r('#ffefb5', -2, 21, 1, 1);
    r('#ffefb5', 2, 21, 1, 1);
  }
  headX = 0;
  headY = 0;
  if (pose === 'paw') {
    const lift = phase * 3;
    r(INK, 14, 20, 9, 4);
    r(INK, 19, 10 - lift, 6, 13 + lift);
    r(INK, 19, 7 - lift, 9, 10);
    r(light, 21, 9 - lift, 5, 7);
    r('#d6927b', 22, 12 - lift, 3, 3);
    r('#d6927b', 21, 10 - lift, 1, 1);
    r('#d6927b', 25, 10 - lift, 1, 1);
  }
}

/** Original 160px pixel art; the app supplies the round mask and touch controls. */
export function renderScene(ctx, { time = 0, state = {}, activity = 'idle', fetchTarget = null, fetchScore = 0, reducedMotion = false, timeOfDay = 'day' } = {}) {
  ctx.save();
  ctx.imageSmoothingEnabled = false;
  const tick = reducedMotion ? 0 : time;
  const sleeping = Boolean(state.sleeping);
  const playing = activity === 'play' && !sleeping;
  const feeding = activity === 'feed' && !sleeping;
  const roomPhase = reducedMotion ? 2 : Math.floor(tick / (sleeping ? 600 : 230)) % 4;
  if (playing) meadow(ctx, timeOfDay);
  else room(ctx, sleeping ? 'night' : timeOfDay, { activity, phase: roomPhase, sleeping });
  const bob = !sleeping && !feeding && tick % 1600 > 800 ? 1 : 0;
  const stage = state.daysTogether >= 7 && state.friendship >= 60 ? 2 : state.daysTogether >= 3 && state.friendship >= 20 ? 1 : 0;
  const pose = !sleeping && activity.startsWith('trick-') ? activity.slice(6) : '';
  const turning = pose === 'spin' || pose === 'roll';
  const phase = feeding || activity === 'shelf' || activity === 'fern' ? roomPhase
    : reducedMotion ? (pose === 'roll' ? 2 : 1) : Math.floor(tick / (turning ? 230 : 400)) % (turning ? 4 : 2);
  const jump = pose === 'jump' ? (reducedMotion ? 11 : [0, 3, 7, 11, 7, 3][Math.floor(tick / 130) % 6]) : 0;
  rect(ctx, '#947d7b', 64 + Math.floor(jump / 3), feeding ? 95 : 110, 35 - Math.floor(jump / 3) * 2, 3);
  puppy(ctx, 80, (feeding ? 65 : 80) + bob - jump, { sleeping, blink: (feeding || activity === 'fern') && phase === 2 || tick % 5300 > 5100, wag: !sleeping && tick % 600 < 300, activity, stage, pose, phase });
  if (pose === 'spin') {
    rect(ctx, '#b58ea5', 51, 86, 7, 1);
    rect(ctx, '#b58ea5', 51, 86, 1, 4);
    rect(ctx, '#b58ea5', 50, 88, 3, 1);
    rect(ctx, '#b58ea5', 105, 96, 7, 1);
    rect(ctx, '#b58ea5', 111, 93, 1, 4);
  }
  if (jump > 5) {
    rect(ctx, '#f7e3c5', 59, 99, 1, 5);
    rect(ctx, '#f7e3c5', 105, 98, 1, 5);
  }

  if (sleeping) {
    const lift = tick % 1800 > 900 ? 1 : 0;
    const r = (x, y, w, h) => rect(ctx, '#e6d7ef', x, y - lift, w, h);
    r(102, 61, 7, 1); r(107, 62, 2, 1); r(105, 63, 2, 1);
    r(103, 64, 2, 1); r(102, 65, 7, 1);
    r(112, 53, 5, 1); r(115, 54, 1, 1); r(114, 55, 1, 1); r(112, 56, 5, 1);
  } else if (activity === 'read') {
    rect(ctx, INK, 66, 103, 29, 12);
    rect(ctx, '#8471a5', 67, 105, 27, 11);
    rect(ctx, '#fff3ce', 68, 102, 11, 10);
    rect(ctx, '#fff3ce', 81, 102, 11, 10);
    rect(ctx, '#decaa5', 79, 103, 2, 11);
    for (let y = 105; y < 111; y += 3) {
      rect(ctx, '#bd9a8c', 70, y, 7, 1);
      rect(ctx, '#bd9a8c', 83, y, 7, 1);
    }
  } else if (activity === 'feed') {
    // A side bowl stays visible beside the muzzle, above the speech bubble.
    rect(ctx, INK, 98, 84, 30, 3);
    rect(ctx, '#ce8e52', 103, 81, feeding && phase < 2 ? 20 : 12, 3);
    rect(ctx, '#f7c684', 106, 80, 3, 2);
    rect(ctx, '#f7c684', 116, 79, 3, 2);
    rect(ctx, '#7baaa5', 99, 87, 28, 5);
    rect(ctx, '#5a8e8c', 102, 92, 22, 2);
    rect(ctx, '#b5d7c5', 100, 87, 26, 1);
  } else if (activity === 'pet') {
    const lift = tick % 900 > 450 ? 2 : 0;
    heart(ctx, 53, 61 - lift);
    heart(ctx, 102, 56 + lift, '#bd82a5');
    heart(ctx, 82, 47 - lift, '#ef9e94');
  } else if (activity === 'fern') {
    const drift = [0, 2, 4, 2][phase];
    rect(ctx, '#94b28c', 106 + drift, 76, 3, 2);
    rect(ctx, '#73967b', 108 + drift, 78, 2, 1);
    rect(ctx, '#fff3d6', 102 + drift, 81, 2, 1);
  }

  if (playing && fetchTarget) {
    const x = Math.round(fetchTarget.x) - 5;
    const y = Math.round(fetchTarget.y) - 5;
    rect(ctx, '#6b8252', x + 1, y + 9, 9, 2);
    rect(ctx, '#73824a', x + 2, y, 6, 10);
    rect(ctx, '#73824a', x, y + 2, 10, 6);
    rect(ctx, '#deeb84', x + 2, y + 1, 6, 8);
    rect(ctx, '#cedb6b', x + 1, y + 3, 8, 4);
    rect(ctx, '#ffffce', x + 3, y + 1, 1, 3);
    rect(ctx, '#ffffce', x + 4, y + 4, 2, 2);
    rect(ctx, '#ffffce', x + 6, y + 6, 1, 3);
  }
  if (playing && fetchScore > 0) {
    for (let i = 0; i < Math.min(fetchScore, 5); i++) heart(ctx, 63 + i * 8, 48, '#de9a84');
  }
  ctx.restore();
}
