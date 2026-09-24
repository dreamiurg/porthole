// Small original diagrams: each picture shows the object or mechanism in its card.
function box(d, x, y, w, h, fill) {
  d.rect(x, y, w, h, d.c.ink);
  d.rect(x + 1, y + 1, w - 2, h - 2, fill);
}

function star(d, x, y, color) {
  d.line(x - 3, y, x + 3, y, color);
  d.line(x, y - 3, x, y + 3, color);
  d.rect(x - 1, y - 1, 3, 3, color);
}

function book(d, x, y, w, h, color) {
  box(d, x, y, w, h, color);
  d.rect(x + 3, y + 3, w - 6, h - 5, d.c.cream);
  d.line(x + 4, y + 6, x + w - 5, y + 6, d.c.gold);
  d.line(x + 4, y + 9, x + w - 5, y + 9, d.c.gold);
}

function wave(d, x, y, length, height, step, color) {
  for (let i = 0; i < length; i += step) {
    const end = Math.min(i + step, length);
    d.line(x + i, y + (i / step % 2 ? height : 0),
      x + end, y + (i / step % 2 ? 0 : height), color);
  }
}

function person(d, x, y, color) {
  d.circle(x, y, 3, d.c.ink);
  d.circle(x, y, 2, d.c.cream);
  d.rect(x - 2, y + 4, 5, 7, color);
  d.line(x - 1, y + 11, x - 3, y + 15, d.c.ink);
  d.line(x + 1, y + 11, x + 3, y + 15, d.c.ink);
}

function rune(d, x, y, color) {
  d.triangle(x, y, 5, 4, color);
  d.line(x + 2, y + 3, x + 2, y + 6, color);
  d.line(x + 5, y + 3, x + 8, y + 1, color);
}

function braille(d, x, y, color, pattern = [0, 1, 3, 5]) {
  for (let i = 0; i < 6; i++) d.circle(x + Math.floor(i / 3) * 7,
    y + (i % 3) * 7, pattern.includes(i) ? 2 : 1, color);
}

function qr(d, x, y) {
  d.rect(x - 2, y - 2, 25, 25, d.c.white);
  for (let row = 0; row < 21; row++) for (let col = 0; col < 21; col++) {
    if ((row * 7 + col * 11 + row * col) % 5 < 2) d.rect(x + col, y + row, 1, 1, d.c.ink);
  }
  for (const [cx, cy] of [[0, 0], [14, 0], [0, 14]]) {
    d.rect(x + cx, y + cy, 7, 7, d.c.ink);
    d.rect(x + cx + 1, y + cy + 1, 5, 5, d.c.white);
    d.rect(x + cx + 2, y + cy + 2, 3, 3, d.c.ink);
  }
}

export const culturePictures = {
  'history-01': {
    alt: 'A cut reed presses wedge-shaped marks into a clay tablet.',
    draw(d) {
      const c = d.c;
      d.rect(17, 12, 47, 31, c.gold);
      box(d, 13, 8, 47, 32, c.orange);
      d.rect(16, 11, 40, 25, c.gold);
      for (let row = 0; row < 3; row++) for (let col = 0; col < 3; col++) rune(d, 19 + col * 11, 13 + row * 7, c.orange);
      d.line(78, 5, 54, 28, c.ink);
      d.line(80, 7, 56, 30, c.ink);
      d.line(79, 6, 55, 29, c.green);
      d.triangle(50, 27, 7, 5, c.ink);
      star(d, 67, 32, c.rose);
    },
  },
  'history-02': {
    alt: 'Three bands of writing on a stone are compared with an open book.',
    draw(d) {
      const c = d.c;
      d.rect(14, 9, 33, 34, c.ink);
      d.rect(19, 5, 21, 5, c.ink);
      d.rect(17, 12, 27, 8, c.teal);
      d.rect(17, 22, 27, 7, c.purple);
      d.rect(17, 31, 27, 8, c.blue);
      for (let row = 0; row < 3; row++) for (let col = 0; col < 5; col++) {
        d.line(20 + col * 5, 14 + row * 10, 22 + col * 5, 17 + row * 10, c.cream);
      }
      d.arrow(49, 23, 60, 23, c.gold);
      book(d, 65, 15, 12, 21, c.rose);
      book(d, 76, 15, 12, 21, c.rose);
      d.line(76, 16, 76, 36, c.ink);
    },
  },
  'history-03': {
    alt: 'Coloured khipu cords hang from a main rope with knots at different positions.',
    draw(d) {
      const c = d.c;
      d.line(9, 8, 87, 8, c.ink);
      d.line(9, 9, 87, 9, c.gold);
      const colors = [c.rose, c.teal, c.orange, c.purple, c.blue, c.green];
      colors.forEach((color, i) => {
        const x = 18 + i * 12;
        d.line(x, 9, x, 41 - i % 3, color);
        d.circle(x, 10, 2, color);
        for (let knot = 0; knot <= i % 3; knot++) {
          d.rect(x - 2, 20 + knot * 5 - i % 2 * 5, 5, 3, color);
          d.rect(x, 21 + knot * 5 - i % 2 * 5, 1, 1, c.cream);
        }
      });
    },
  },
  'history-04': {
    alt: 'Handwritten manuscripts show astronomy, patterns and poetry in a library.',
    draw(d) {
      const c = d.c;
      d.rect(7, 37, 82, 4, c.ink);
      [c.teal, c.gold, c.rose, c.purple].forEach((color, i) => {
        box(d, 11 + i * 7, 12 + i % 2 * 3, 6, 25 - i % 2 * 3, color);
        d.rect(13 + i * 7, 17 + i % 2 * 3, 2, 2, c.cream);
      });
      book(d, 47, 13, 18, 24, c.orange);
      book(d, 64, 13, 19, 24, c.orange);
      d.circle(56, 25, 4, c.blue);
      star(d, 57, 25, c.gold);
      d.line(68, 25, 78, 25, c.teal);
      d.line(68, 29, 75, 29, c.teal);
      d.rect(82, 6, 2, 5, c.gold);
    },
  },
  'history-05': {
    alt: 'Canals connect a reservoir, a stepped temple and surrounding fields at Angkor.',
    draw(d) {
      const c = d.c;
      box(d, 7, 10, 26, 25, c.sky);
      d.rect(11, 14, 18, 17, c.blue);
      d.rect(32, 20, 56, 5, c.sky);
      d.rect(43, 21, 4, 22, c.sky);
      d.rect(72, 8, 4, 33, c.sky);
      d.rect(36, 30, 23, 3, c.gold);
      d.rect(39, 26, 17, 4, c.orange);
      d.rect(43, 19, 9, 8, c.gold);
      d.rect(45, 13, 5, 7, c.orange);
      d.triangle(44, 8, 7, 6, c.gold);
      for (let row = 0; row < 3; row++) {
        d.rect(79, 9 + row * 11, 10, 7, c.green);
        d.line(80, 11 + row * 11, 87, 11 + row * 11, c.teal);
      }
      d.arrow(24, 22, 38, 22, c.cream);
    },
  },
  'history-06': {
    alt: 'A voyaging canoe follows ocean swells beneath a pattern of stars.',
    draw(d) {
      const c = d.c;
      for (const [x, y] of [[15, 10], [35, 6], [60, 9], [80, 5]]) star(d, x, y, c.gold);
      d.rect(8, 31, 81, 13, c.sky);
      wave(d, 9, 34, 76, 2, 4, c.blue);
      wave(d, 9, 41, 76, 2, 4, c.teal);
      d.line(48, 12, 48, 36, c.ink);
      d.triangle(34, 13, 14, 17, c.cream);
      d.line(49, 15, 61, 29, c.rose);
      d.rect(33, 32, 32, 3, c.orange);
      d.rect(33, 38, 32, 3, c.gold);
      d.line(39, 34, 39, 38, c.ink);
      d.line(58, 34, 58, 38, c.ink);
      d.arrow(69, 28, 79, 18, c.teal);
    },
  },
  'history-07': {
    alt: 'Seven stepped levels descend through a deep well toward blue water.',
    draw(d) {
      const c = d.c;
      d.rect(6, 7, 84, 4, c.gold);
      for (let step = 0; step < 7; step++) {
        const inset = step * 4;
        d.rect(8 + inset, 11 + step * 4, 7, 4, c.orange);
        d.rect(81 - inset, 11 + step * 4, 7, 4, c.orange);
        d.line(8 + inset, 10 + step * 4, 14 + inset, 10 + step * 4, c.ink);
        d.line(81 - inset, 10 + step * 4, 87 - inset, 10 + step * 4, c.ink);
      }
      d.rect(36, 37, 24, 7, c.blue);
      d.line(39, 39, 47, 39, c.sky);
      d.line(49, 42, 56, 42, c.sky);
      for (const x of [12, 28, 65, 81]) {
        d.rect(x, 3, 2, 5, c.orange);
        d.rect(x - 1, 3, 4, 2, c.gold);
      }
    },
  },
  'history-08': {
    alt: 'A patterned dry-stone wall stands behind a porcelain fragment and a trade coin.',
    draw(d) {
      const c = d.c;
      for (let row = 0; row < 5; row++) for (let col = 0; col < 7; col++) {
        d.rect(8 + col * 11 + row % 2 * 3, 7 + row * 5, 10, 4, row % 2 ? c.gold : c.orange);
      }
      wave(d, 11, 12, 70, 3, 5, c.cream);
      d.rect(21, 30, 22, 11, c.cream);
      d.rect(25, 27, 14, 3, c.cream);
      d.rect(25, 41, 15, 3, c.cream);
      wave(d, 24, 33, 16, 2, 4, c.blue);
      d.line(25, 39, 38, 39, c.blue);
      d.circle(66, 36, 8, c.ink);
      d.circle(66, 36, 7, c.gold);
      d.circle(66, 36, 4, c.orange);
      d.rect(65, 33, 2, 6, c.cream);
    },
  },
  'humanity-01': {
    alt: 'Whistled sound patterns cross a valley between two people.',
    draw(d) {
      const c = d.c;
      d.triangle(5, 17, 35, 27, c.green);
      d.triangle(61, 21, 30, 23, c.teal);
      person(d, 22, 12, c.purple);
      person(d, 77, 16, c.orange);
      d.line(25, 15, 30, 14, c.ink);
      wave(d, 33, 12, 28, 3, 4, c.blue);
      d.arrow(65, 15, 71, 18, c.blue);
      d.line(39, 35, 46, 38, c.sky);
      d.line(46, 38, 55, 36, c.sky);
    },
  },
  'humanity-02': {
    alt: 'A face with raised eyebrows and two expressive hands shows visual grammar.',
    draw(d) {
      const c = d.c;
      d.circle(48, 20, 13, c.ink);
      d.circle(48, 20, 12, c.cream);
      d.line(40, 13, 44, 12, c.ink);
      d.line(52, 12, 56, 13, c.ink);
      d.rect(42, 17, 2, 3, c.ink);
      d.rect(53, 17, 2, 3, c.ink);
      d.line(45, 26, 51, 26, c.rose);
      d.rect(39, 34, 18, 9, c.teal);
      for (const x of [20, 70]) {
        box(d, x, 24, 9, 10, c.cream);
        for (let finger = 0; finger < 3; finger++) d.rect(x + 1 + finger * 3, 17 + finger % 2 * 2, 2, 8, c.cream);
        d.line(x + 4, 34, x + 4, 40, c.ink);
      }
      d.arrow(30, 10, 35, 7, c.gold);
      d.arrow(64, 10, 60, 7, c.gold);
    },
  },
  'humanity-03': {
    alt: 'An enlarged six-position braille cell sits beside a fingertip reading a page.',
    draw(d) {
      const c = d.c;
      box(d, 10, 7, 25, 34, c.white);
      braille(d, 19, 17, c.ink);
      d.arrow(39, 23, 49, 23, c.gold);
      box(d, 55, 9, 30, 32, c.cream);
      for (let row = 0; row < 3; row++) for (let col = 0; col < 5; col++) {
        d.rect(59 + col * 4, 14 + row * 7, 1, 2, c.ink);
        if ((row + col) % 2) d.rect(60 + col * 4, 17 + row * 7, 1, 1, c.ink);
      }
      d.circle(68, 33, 4, c.ink);
      d.rect(68, 29, 22, 9, c.ink);
      d.circle(68, 33, 3, c.gold);
      d.rect(68, 30, 22, 7, c.gold);
    },
  },
  'humanity-04': {
    alt: 'Braided ropes support a suspension bridge across a gorge, with a fresh rope beside it.',
    draw(d) {
      const c = d.c;
      d.rect(5, 17, 16, 27, c.green);
      d.rect(75, 17, 16, 27, c.teal);
      d.line(17, 10, 33, 21, c.orange);
      d.line(33, 21, 61, 21, c.orange);
      d.line(61, 21, 79, 10, c.orange);
      d.line(17, 23, 34, 30, c.gold);
      d.line(34, 30, 61, 30, c.gold);
      d.line(61, 30, 79, 23, c.gold);
      for (let i = 0; i < 7; i++) {
        const x = 23 + i * 8;
        d.line(x, 18 + (i > 0 && i < 6 ? 3 : 0), x, 26 + (i > 0 && i < 6 ? 4 : 0), c.ink);
      }
      d.circle(49, 39, 5, c.orange);
      d.circle(49, 39, 3, c.paper);
      d.line(54, 39, 67, 39, c.orange);
      wave(d, 57, 39, 12, 2, 2, c.gold);
    },
  },
  'humanity-05': {
    alt: 'A storyteller’s voice carries a journey, a mountain and a star through speech.',
    draw(d) {
      const c = d.c;
      person(d, 16, 22, c.teal);
      d.line(18, 30, 26, 25, c.ink);
      d.circle(23, 15, 1, c.gold);
      d.circle(28, 12, 2, c.gold);
      box(d, 34, 5, 54, 31, c.cream);
      d.triangle(41, 16, 17, 14, c.blue);
      d.triangle(46, 16, 7, 6, c.white);
      d.line(55, 30, 65, 26, c.orange);
      d.line(65, 26, 76, 29, c.orange);
      star(d, 77, 13, c.gold);
      d.line(39, 41, 82, 41, c.purple);
      for (let i = 0; i < 6; i++) d.rect(42 + i * 7, 38 - i % 2 * 2, 2, 5 + i % 2 * 2, c.purple);
    },
  },
  'humanity-06': {
    alt: 'One voice produces a low wave and a higher overtone, drawn as two sound patterns.',
    draw(d) {
      const c = d.c;
      d.circle(19, 23, 10, c.ink);
      d.circle(19, 23, 9, c.cream);
      d.rect(22, 20, 2, 2, c.ink);
      d.rect(26, 25, 5, 3, c.rose);
      d.line(34, 25, 42, 16, c.gold);
      d.line(34, 25, 42, 34, c.gold);
      wave(d, 43, 12, 40, 5, 2, c.blue);
      wave(d, 43, 31, 40, 7, 10, c.purple);
      d.text('HIGH', 62, 5, c.blue);
      d.text('LOW', 66, 41, c.purple);
    },
  },
  'humanity-07': {
    alt: 'A shared canal branches through gates into three green rice fields.',
    draw(d) {
      const c = d.c;
      d.rect(8, 7, 80, 5, c.blue);
      for (let field = 0; field < 3; field++) {
        const x = 11 + field * 27;
        d.rect(x + 9, 11, 4, 12, c.sky);
        box(d, x, 23, 22, 19, c.teal);
        d.rect(x + 2, 25, 18, 15, c.green);
        for (let row = 0; row < 3; row++) for (let col = 0; col < 4; col++) {
          d.line(x + 4 + col * 4, 28 + row * 5, x + 5 + col * 4, 26 + row * 5, c.gold);
        }
        d.rect(x + 7, 16, 8, 2, c.orange);
        d.arrow(x + 11, 18, x + 11, 23, c.cream);
      }
    },
  },
  'humanity-08': {
    alt: 'Moon phases sit above a twelve-month calendar with an extra month being added.',
    draw(d) {
      const c = d.c;
      for (let i = 0; i < 3; i++) {
        d.circle(24 + i * 18, 9, 5, c.gold);
        if (i < 2) d.circle(21 + i * 18, 7, i ? 3 : 5, c.paper);
      }
      box(d, 12, 18, 54, 25, c.white);
      for (let row = 0; row < 3; row++) for (let col = 0; col < 4; col++) box(d, 16 + col * 12, 22 + row * 6, 9, 5, c.sky);
      box(d, 76, 24, 12, 12, c.rose);
      d.rect(81, 26, 2, 8, c.white);
      d.rect(78, 29, 8, 2, c.white);
      d.arrow(73, 30, 67, 30, c.gold);
    },
  },
  'art-01': {
    alt: 'Three coloured woodblocks align to print one flower using corner guide marks.',
    draw(d) {
      const c = d.c;
      [c.teal, c.rose, c.gold].forEach((color, i) => {
        box(d, 7, 6 + i * 13, 19, 10, c.orange);
        d.rect(11, 9 + i * 13, 11, 4, color);
        d.arrow(29, 11 + i * 13, 39, 22, color);
      });
      box(d, 45, 6, 41, 37, c.cream);
      d.line(66, 24, 66, 36, c.teal);
      d.line(66, 31, 57, 27, c.teal);
      for (const [x, y] of [[61, 19], [71, 19], [61, 27], [71, 27]]) d.circle(x, y, 4, c.rose);
      d.circle(66, 23, 4, c.gold);
      d.line(49, 9, 54, 9, c.ink);
      d.line(49, 9, 49, 14, c.ink);
      d.line(77, 39, 82, 39, c.ink);
      d.line(82, 34, 82, 39, c.ink);
    },
  },
  'art-02': {
    alt: 'A handscroll reveals a mountain landscape a section at a time, from right to left.',
    draw(d) {
      const c = d.c;
      box(d, 18, 12, 62, 29, c.cream);
      d.triangle(24, 19, 22, 18, c.teal);
      d.triangle(40, 24, 17, 13, c.green);
      d.line(56, 36, 73, 31, c.blue);
      d.circle(64, 21, 4, c.gold);
      box(d, 12, 8, 9, 36, c.orange);
      box(d, 77, 8, 9, 36, c.orange);
      d.line(16, 11, 16, 41, c.gold);
      d.line(81, 11, 81, 41, c.gold);
      d.arrow(67, 6, 30, 6, c.purple);
    },
  },
  'art-03': {
    alt: 'Repeated interlacing squares and diamonds make a geometric tile pattern.',
    draw(d) {
      const c = d.c;
      box(d, 7, 5, 82, 39, c.cream);
      for (let row = 0; row < 2; row++) for (let col = 0; col < 4; col++) {
        const x = 18 + col * 20, y = 15 + row * 18;
        d.line(x - 8, y, x, y - 8, c.teal);
        d.line(x, y - 8, x + 8, y, c.teal);
        d.line(x + 8, y, x, y + 8, c.teal);
        d.line(x, y + 8, x - 8, y, c.teal);
        box(d, x - 5, y - 5, 11, 11, c.paper);
        d.rect(x - 2, y - 2, 5, 5, c.gold);
        d.rect(x, y, 1, 1, c.purple);
      }
    },
  },
  'art-04': {
    alt: 'Narrow woven kente strips with geometric rhythms are joined into a wider cloth.',
    draw(d) {
      const c = d.c, colors = [c.gold, c.teal, c.rose, c.green, c.gold];
      colors.forEach((color, col) => {
        const x = 16 + col * 13;
        d.rect(x, 7, 12, 33, color);
        for (let row = 0; row < 4; row++) {
          d.rect(x + 2, 10 + row * 7, 8, 2, col % 2 ? c.gold : c.purple);
          d.rect(x + 5, 12 + row * 7, 2, 3, c.cream);
        }
        for (let seam = 0; seam < 8; seam++) d.rect(x + 12, 8 + seam * 4, 1, 2, c.ink);
        for (let fringe = 0; fringe < 4; fringe++) d.rect(x + 1 + fringe * 3, 40, 1, 4, color);
      });
    },
  },
  'art-05': {
    alt: 'A strip of inner bark is beaten with a wooden tool into a broad sheet of siapo.',
    draw(d) {
      const c = d.c;
      box(d, 9, 8, 12, 31, c.orange);
      d.line(13, 12, 13, 35, c.gold);
      d.line(17, 13, 17, 34, c.cream);
      d.arrow(25, 25, 36, 25, c.gold);
      d.rect(39, 28, 18, 9, c.cream);
      d.line(46, 30, 52, 13, c.ink);
      d.line(47, 30, 53, 13, c.orange);
      box(d, 43, 8, 17, 7, c.gold);
      d.arrow(59, 28, 65, 28, c.gold);
      box(d, 70, 12, 20, 29, c.cream);
      for (let i = 0; i < 4; i++) d.line(73, 17 + i * 6, 86, 15 + i * 6, c.gold);
    },
  },
  'art-06': {
    alt: 'Small metal caps joined with copper wire drape into a flexible hanging artwork.',
    // biome-ignore lint/complexity/noExcessiveCognitiveComplexity: hand-placed pixel illustration
    draw(d) {
      const c = d.c;
      for (let col = 0; col < 9; col++) for (let row = 0; row < 4; row++) {
        const x = 14 + col * 8, y = 9 + row * 8 + (col % 4 < 2 ? 0 : 3);
        if (col < 8) d.line(x, y, x + 8, 9 + row * 8 + ((col + 1) % 4 < 2 ? 0 : 3), c.orange);
        if (row < 3) d.line(x, y, x, y + 8, c.orange);
        d.circle(x, y, 3, (col + row) % 3 ? c.gold : c.rose);
        d.line(x - 1, y - 1, x + 1, y - 1, c.cream);
      }
      d.line(12, 43, 37, 43, c.gold);
      d.line(43, 43, 84, 43, c.gold);
    },
  },
  'art-07': {
    alt: 'A painted portrait and an X-ray view reveal two different arm positions.',
    draw(d) {
      const c = d.c;
      box(d, 7, 5, 32, 38, c.gold);
      d.rect(10, 8, 26, 32, c.sky);
      d.circle(23, 18, 5, c.cream);
      d.rect(18, 24, 11, 15, c.purple);
      d.line(19, 27, 13, 33, c.cream);
      d.line(28, 27, 33, 34, c.cream);
      d.arrow(43, 23, 52, 23, c.rose);
      box(d, 57, 5, 32, 38, c.ink);
      d.rect(60, 8, 26, 32, c.blue);
      d.circle(73, 18, 5, c.sky);
      d.line(73, 23, 73, 36, c.sky);
      d.line(69, 26, 64, 32, c.sky);
      d.line(77, 26, 83, 32, c.sky);
      d.line(69, 26, 65, 18, c.rose);
      d.line(77, 26, 82, 17, c.rose);
      d.line(67, 25, 79, 25, c.sky);
    },
  },
  'art-08': {
    alt: 'Light casts the outline of a flat, jointed puppet onto a screen while rods move its arms.',
    draw(d) {
      const c = d.c;
      d.circle(12, 23, 5, c.gold);
      d.line(18, 20, 35, 9, c.gold);
      d.line(18, 26, 35, 38, c.gold);
      box(d, 35, 5, 54, 37, c.orange);
      d.rect(38, 8, 48, 31, c.cream);
      d.circle(63, 16, 4, c.ink);
      d.rect(59, 21, 9, 10, c.ink);
      d.line(59, 24, 51, 19, c.ink);
      d.line(51, 19, 47, 25, c.ink);
      d.line(67, 24, 75, 19, c.ink);
      d.line(75, 19, 79, 14, c.ink);
      d.line(61, 30, 57, 36, c.ink);
      d.line(66, 30, 71, 36, c.ink);
      d.line(47, 25, 46, 44, c.teal);
      d.line(79, 14, 82, 44, c.teal);
      d.circle(51, 19, 1, c.gold);
      d.circle(75, 19, 1, c.gold);
    },
  },
  'inventions-01': {
    alt: 'Individual metal printing pieces are rearranged to make a new printed page.',
    draw(d) {
      const c = d.c;
      for (let row = 0; row < 3; row++) for (let col = 0; col < 3; col++) {
        const x = 8 + col * 10, y = 8 + row * 11;
        box(d, x, y, 8, 9, c.sky);
        d.line(x + 2, y + 3, x + 5, y + 3, c.ink);
        d.line(x + 3, y + 2, x + 3, y + 6, c.ink);
        if ((row + col) % 2) d.line(x + 2, y + 6, x + 5, y + 6, c.ink);
      }
      d.arrow(40, 24, 52, 24, c.gold);
      book(d, 59, 7, 28, 35, c.rose);
      for (let row = 0; row < 3; row++) for (let col = 0; col < 3; col++) {
        const x = 64 + col * 7, y = 19 + row * 6;
        d.line(x, y, x + 3, y, c.ink);
        d.line(x + 1, y - 1, x + 1, y + 2, c.ink);
      }
      d.arrow(21, 43, 31, 43, c.teal);
    },
  },
  'inventions-02': {
    alt: 'A mesh mould lifts watery paper pulp from a vat before it becomes a dry sheet.',
    draw(d) {
      const c = d.c;
      box(d, 7, 23, 36, 19, c.orange);
      d.rect(9, 25, 32, 8, c.sky);
      for (let i = 0; i < 7; i++) d.line(12 + i * 4, 28 + i % 2 * 2, 14 + i * 4, 29 + i % 2 * 2, c.cream);
      box(d, 13, 11, 26, 11, c.gold);
      d.rect(16, 14, 20, 5, c.cream);
      for (let i = 0; i < 3; i++) d.rect(19 + i * 7, 23, 1, 4, c.blue);
      d.arrow(46, 24, 57, 24, c.teal);
      d.rect(63, 15, 25, 25, c.gold);
      box(d, 60, 11, 25, 25, c.white);
      for (let i = 0; i < 5; i++) d.line(65 + i % 3 * 5, 17 + i * 3, 68 + i % 3 * 5, 18 + i * 3, c.cream);
    },
  },
  'inventions-03': {
    alt: 'A nearby horseshoe magnet draws a compass needle away from its north mark.',
    draw(d) {
      const c = d.c;
      d.circle(28, 25, 18, c.ink);
      d.circle(28, 25, 16, c.cream);
      d.text('N', 27, 10, c.ink);
      d.line(28, 25, 40, 20, c.rose);
      d.line(28, 24, 39, 20, c.rose);
      d.line(28, 25, 17, 30, c.teal);
      d.circle(28, 25, 2, c.ink);
      d.rect(65, 12, 19, 25, c.ink);
      d.rect(69, 12, 11, 19, c.paper);
      d.rect(66, 13, 3, 18, c.rose);
      d.rect(80, 13, 3, 18, c.blue);
      d.rect(69, 31, 11, 5, c.gold);
      d.arrow(50, 22, 61, 18, c.purple);
      d.line(56, 9, 61, 12, c.gold);
      d.line(55, 34, 60, 30, c.gold);
    },
  },
  'inventions-04': {
    alt: 'A chain of punched cards controls the threads of a loom and its woven pattern.',
    draw(d) {
      const c = d.c;
      box(d, 8, 6, 38, 37, c.orange);
      d.rect(12, 10, 30, 29, c.paper);
      for (let col = 0; col < 8; col++) d.line(13 + col * 4, 9, 13 + col * 4, 39, c.gold);
      for (let row = 0; row < 5; row++) d.line(13, 25 + row * 3, 41, 25 + row * 3, row % 2 ? c.teal : c.rose);
      d.arrow(59, 22, 48, 22, c.blue);
      for (let card = 0; card < 3; card++) {
        box(d, 66, 5 + card * 13, 22, 11, c.cream);
        for (let hole = 0; hole < 4; hole++) d.rect(70 + hole * 4, 8 + card * 13 + (card + hole) % 2 * 3, 2, 2, c.ink);
        if (card < 2) {
          d.rect(69, 16 + card * 13, 1, 2, c.ink);
          d.rect(85, 16 + card * 13, 1, 2, c.ink);
        }
      }
    },
  },
  'inventions-05': {
    alt: 'Fold lines on a flat sheet become the folded base of an upright paper bag.',
    draw(d) {
      const c = d.c;
      box(d, 8, 9, 29, 31, c.cream);
      for (let i = 0; i < 5; i++) {
        d.rect(16, 13 + i * 5, 1, 2, c.orange);
        d.rect(28, 13 + i * 5, 1, 2, c.orange);
      }
      d.line(12, 32, 33, 32, c.orange);
      d.line(12, 32, 21, 38, c.orange);
      d.line(33, 32, 25, 38, c.orange);
      d.arrow(42, 24, 54, 24, c.teal);
      box(d, 63, 11, 24, 31, c.gold);
      d.rect(65, 13, 20, 4, c.ink);
      d.rect(66, 14, 18, 2, c.orange);
      d.line(64, 35, 75, 40, c.orange);
      d.line(86, 35, 75, 40, c.orange);
      d.line(75, 17, 75, 34, c.cream);
    },
  },
  'inventions-06': {
    alt: 'A T-shaped manual traffic signal holds cars in every direction for a clearing pause.',
    draw(d) {
      const c = d.c;
      d.rect(8, 19, 80, 13, c.ink);
      d.rect(42, 5, 13, 39, c.ink);
      d.rect(13, 22, 13, 6, c.teal);
      d.rect(69, 22, 13, 6, c.rose);
      d.rect(45, 6, 6, 9, c.gold);
      d.rect(45, 35, 6, 9, c.blue);
      d.rect(16, 23, 4, 4, c.sky);
      d.rect(75, 23, 4, 4, c.cream);
      d.rect(45, 18, 5, 15, c.cream);
      box(d, 32, 16, 31, 10, c.gold);
      d.text('STOP', 39, 19, c.ink);
      d.line(45, 32, 55, 34, c.orange);
      d.circle(56, 34, 2, c.orange);
    },
  },
  'inventions-07': {
    alt: 'A burr caught on a puppy’s fur is enlarged to show hooks catching loops.',
    draw(d) {
      const c = d.c;
      d.rect(9, 23, 25, 13, c.gold);
      d.circle(32, 20, 9, c.gold);
      d.rect(23, 13, 6, 12, c.orange);
      d.rect(35, 19, 8, 6, c.gold);
      d.rect(41, 20, 3, 3, c.ink);
      d.rect(34, 16, 2, 2, c.ink);
      d.rect(13, 35, 4, 7, c.orange);
      d.rect(29, 35, 4, 7, c.orange);
      d.line(10, 27, 6, 20, c.gold);
      star(d, 18, 23, c.green);
      d.circle(18, 23, 3, c.teal);
      d.line(22, 21, 57, 10, c.teal);
      d.circle(72, 23, 15, c.ink);
      d.circle(72, 23, 13, c.cream);
      for (let i = 0; i < 3; i++) {
        const x = 63 + i * 7;
        d.line(x, 31, x, 21, c.teal);
        d.line(x, 21, x + 3, 18, c.teal);
        d.line(x + 3, 18, x + 5, 21, c.teal);
        d.line(x + 5, 21, x + 4, 24, c.teal);
        d.circle(x + 3, 27, 3, c.rose);
      }
    },
  },
  'inventions-08': {
    alt: 'A damaged QR-style pattern points to a restored pattern using error correction.',
    draw(d) {
      const c = d.c;
      qr(d, 11, 13);
      qr(d, 63, 13);
      d.rect(21, 22, 7, 9, c.paper);
      d.line(22, 22, 26, 29, c.rose);
      d.arrow(39, 23, 54, 23, c.teal);
      star(d, 75, 7, c.gold);
      d.line(65, 40, 69, 43, c.teal);
      d.line(69, 43, 77, 36, c.teal);
    },
  },
};
