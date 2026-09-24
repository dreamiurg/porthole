// Original 96×48 pixel diagrams. Relationships are illustrative, not to scale.
function sun(d, x, y, r = 8) {
  const { gold, orange } = d.c;
  for (const [dx, dy] of [[1, 0], [-1, 0], [0, 1], [0, -1]]) {
    d.line(x + dx * (r + 2), y + dy * (r + 2), x + dx * (r + 4), y + dy * (r + 4), gold);
  }
  d.circle(x, y, r, gold);
  d.circle(x - 2, y - 2, Math.max(2, r - 4), orange);
}

function earth(d, x, y, r = 9) {
  d.circle(x, y, r, d.c.blue);
  d.rect(x - 4, y - 5, 5, 5, d.c.green);
  d.rect(x - 1, y - 1, 5, 4, d.c.green);
  d.rect(x + 2, y + 3, 2, 4, d.c.green);
}

function cloud(d, x, y) {
  d.circle(x + 5, y + 7, 6, d.c.sky);
  d.circle(x + 13, y + 4, 8, d.c.sky);
  d.circle(x + 22, y + 7, 6, d.c.sky);
  d.rect(x + 3, y + 6, 21, 8, d.c.sky);
}

function orbit(d, x, y, rx, ry) {
  const points = [[1, 0], [.7, .7], [0, 1], [-.7, .7], [-1, 0], [-.7, -.7], [0, -1], [.7, -.7], [1, 0]];
  for (let i = 1; i < points.length; i++) {
    d.line(Math.round(x + points[i - 1][0] * rx), Math.round(y + points[i - 1][1] * ry),
      Math.round(x + points[i][0] * rx), Math.round(y + points[i][1] * ry), d.c.purple);
  }
}

function mushroom(d, x, y) {
  d.rect(x - 2, y, 5, 13, d.c.cream);
  d.line(x - 3, y + 1, x - 3, y + 12, d.c.orange);
  d.circle(x, y, 9, d.c.rose);
  d.rect(x - 9, y + 1, 19, 5, d.c.paper);
  d.rect(x - 2, y + 1, 5, 6, d.c.cream);
  d.rect(x - 5, y - 4, 3, 2, d.c.white);
  d.rect(x + 2, y - 6, 2, 2, d.c.white);
}

function leaf(d, x, y, color = d.c.green) {
  d.rect(x - 5, y - 3, 10, 6, color);
  d.rect(x - 3, y - 5, 6, 10, color);
  d.line(x - 5, y + 5, x + 3, y - 3, d.c.ink);
}

function flytrap(d, x, y, closed = false) {
  d.rect(x - 1, y + 4, 3, 12, d.c.green);
  if (closed) {
    d.circle(x, y, 9, d.c.green);
    d.line(x, y - 7, x, y + 7, d.c.ink);
    for (let dy = -6; dy <= 6; dy += 4) d.line(x - 4, y + dy, x + 4, y + dy, d.c.teal);
  } else {
    for (let offset = 0; offset < 4; offset++) {
      d.line(x - 13, y - 7 + offset, x, y + 4 + offset, d.c.green);
      d.line(x, y + 4 + offset, x + 13, y - 7 + offset, d.c.green);
    }
    for (const side of [-1, 1]) {
      for (let i = 0; i < 3; i++) d.line(x + side * (5 + i * 4), y - i * 3, x + side * (5 + i * 4), y - i * 3 - 4, d.c.green);
      d.line(x + side * 4, y + 2, x + side * 3, y - 4, d.c.rose);
    }
  }
}

export const sciencePictures = {
  'space-01': {
    alt: 'Inside the Sun, four hydrogen nuclei combine into helium and release energy.',
    draw(d) {
      sun(d, 15, 24, 10);
      d.circle(15, 24, 4, d.c.rose);
      d.line(19, 20, 31, 12, d.c.ink);
      d.line(19, 28, 31, 36, d.c.ink);
      for (const [x, y] of [[37, 20], [44, 20], [37, 27], [44, 27]]) d.circle(x, y, 2, d.c.rose);
      d.text('4 H', 35, 9, d.c.ink);
      d.arrow(49, 24, 59, 24, d.c.ink);
      for (const [x, y] of [[66, 22], [70, 22], [66, 26], [70, 26]]) d.circle(x, y, 2, d.c.orange);
      d.text('HE', 65, 9, d.c.ink);
      d.arrow(76, 24, 90, 24, d.c.gold);
      d.line(85, 17, 88, 13, d.c.gold);
      d.line(85, 31, 88, 35, d.c.gold);
      d.text('FUSION', 36, 40, d.c.ink);
    },
  },
  'space-02': {
    alt: 'Bands on the gaseous Sun move at different rates; the equatorial arrow is longest.',
    draw(d) {
      sun(d, 45, 24, 18);
      d.line(34, 13, 56, 13, d.c.cream);
      d.line(28, 24, 62, 24, d.c.cream);
      d.line(34, 35, 56, 35, d.c.cream);
      d.arrow(58, 12, 71, 12, d.c.ink);
      d.arrow(65, 24, 91, 24, d.c.ink);
      d.arrow(58, 36, 71, 36, d.c.ink);
      d.text('GAS', 3, 22, d.c.ink);
    },
  },
  'space-03': {
    alt: 'A beam travels from the Sun to Earth, with a clock marking about eight minutes.',
    draw(d) {
      sun(d, 13, 24, 8);
      earth(d, 82, 24, 10);
      d.arrow(27, 24, 68, 24, d.c.gold);
      for (let x = 31; x < 67; x += 8) d.rect(x, 21, 2, 2, d.c.orange);
      d.circle(46, 9, 7, d.c.ink);
      d.circle(46, 9, 5, d.c.paper);
      d.line(46, 9, 46, 5, d.c.ink);
      d.line(46, 9, 49, 11, d.c.ink);
      d.text('8 MIN', 37, 36, d.c.ink);
    },
  },
  'space-04': {
    alt: 'Four Moon positions orbit Earth; the same marked face points inward at each position.',
    draw(d) {
      orbit(d, 48, 24, 28, 18);
      earth(d, 48, 24, 8);
      for (const [x, y, dx, dy] of [[20, 24, 3, 0], [48, 6, 0, 3], [76, 24, -3, 0], [48, 42, 0, -3]]) {
        d.circle(x, y, 5, d.c.cream);
        d.circle(x + dx, y + dy, 2, d.c.rose);
      }
      d.arrow(66, 9, 72, 15, d.c.ink);
      d.arrow(30, 39, 24, 33, d.c.ink);
    },
  },
  'space-05': {
    alt: 'The Sun lights the Moon’s far side while Earth faces its dark half in this orbital position.',
    draw(d) {
      sun(d, 11, 24, 7);
      d.arrow(24, 24, 34, 24, d.c.gold);
      d.circle(49, 24, 10, d.c.ink);
      for (let dx = -10; dx <= 0; dx++) {
        const h = Math.floor(Math.sqrt(100 - dx * dx));
        d.rect(49 + dx, 24 - h, 1, h * 2 + 1, d.c.gold);
      }
      earth(d, 82, 24, 8);
      d.text('FAR', 35, 3, d.c.ink);
      d.text('LIT', 35, 10, d.c.ink);
      d.line(42, 16, 43, 19, d.c.orange);
      d.text('EARTH', 72, 38, d.c.ink);
    },
  },
  'space-06': {
    alt: 'Venus’s 225-day orbit is shown by a shorter bar than its 243-day full rotation.',
    draw(d) {
      d.circle(17, 24, 11, d.c.orange);
      d.rect(9, 21, 17, 3, d.c.gold);
      d.rect(13, 28, 13, 2, d.c.cream);
      d.text('VENUS', 7, 4, d.c.ink);
      d.text('DAYS', 9, 41, d.c.ink);
      d.text('YEAR 225', 39, 7, d.c.ink);
      d.rect(39, 16, 44, 7, d.c.gold);
      d.text('SPIN 243', 39, 29, d.c.ink);
      d.rect(39, 38, 48, 7, d.c.orange);
    },
  },
  'space-07': {
    alt: 'Sunlight enters Venus’s thick atmosphere, and some outgoing heat is returned toward the planet.',
    draw(d) {
      sun(d, 12, 12, 6);
      d.circle(61, 27, 18, d.c.sky);
      d.circle(61, 27, 13, d.c.orange);
      d.circle(58, 25, 5, d.c.gold);
      d.arrow(24, 12, 49, 24, d.c.gold);
      d.arrow(67, 19, 80, 5, d.c.rose);
      d.arrow(80, 8, 71, 20, d.c.rose);
      d.text('THICK', 3, 31, d.c.ink);
      d.text('AIR', 7, 39, d.c.ink);
      d.text('HEAT', 76, 39, d.c.rose);
    },
  },
  'space-08': {
    alt: 'A close-up of Mars shows rusty red dust coating rocks of several colors.',
    draw(d) {
      d.circle(17, 25, 13, d.c.orange);
      d.rect(9, 19, 10, 4, d.c.rose);
      d.rect(17, 28, 8, 5, d.c.gold);
      d.line(29, 19, 40, 12, d.c.ink);
      d.line(29, 31, 40, 41, d.c.ink);
      d.rect(42, 30, 49, 11, d.c.orange);
      d.triangle(47, 23, 16, 10, d.c.ink);
      d.triangle(70, 25, 17, 9, d.c.gold);
      for (const [x, y] of [[46, 26], [54, 20], [61, 28], [72, 23], [81, 18], [86, 29]]) d.rect(x, y, 2, 2, d.c.rose);
      d.text('RUSTY DUST', 43, 5, d.c.ink);
      d.text('MARS', 9, 42, d.c.ink);
    },
  },
  'earth-01': {
    alt: 'One tectonic plate carries both land and ocean floor; its edge is beyond the coastline.',
    draw(d) {
      d.rect(4, 20, 88, 5, d.c.blue);
      d.rect(4, 14, 29, 11, d.c.gold);
      d.rect(4, 13, 29, 3, d.c.green);
      d.rect(4, 25, 50, 7, d.c.cream);
      d.rect(58, 25, 34, 7, d.c.cream);
      d.rect(4, 32, 88, 5, d.c.orange);
      d.line(4, 26, 53, 26, d.c.ink);
      d.line(58, 26, 91, 26, d.c.ink);
      d.line(4, 38, 53, 38, d.c.ink);
      d.line(4, 36, 4, 39, d.c.ink);
      d.line(53, 36, 53, 39, d.c.ink);
      d.text('LAND', 9, 4, d.c.ink);
      d.text('SEA', 66, 10, d.c.ink);
      d.text('ONE PLATE', 9, 42, d.c.ink);
    },
  },
  'earth-02': {
    alt: 'An oceanic plate bends and descends beneath a continent at a subduction boundary.',
    draw(d) {
      d.rect(4, 14, 44, 8, d.c.blue);
      d.rect(4, 27, 88, 18, d.c.orange);
      d.rect(49, 20, 43, 11, d.c.gold);
      d.rect(49, 19, 43, 3, d.c.green);
      d.rect(4, 22, 39, 5, d.c.cream);
      for (let i = 0; i < 5; i++) d.line(42, 22 + i, 68, 41 + i, d.c.cream);
      d.triangle(71, 5, 16, 16, d.c.gold);
      d.rect(78, 15, 3, 13, d.c.rose);
      d.arrow(14, 24, 35, 24, d.c.ink);
      d.arrow(49, 32, 61, 41, d.c.ink);
      d.text('DOWN', 8, 36, d.c.ink);
    },
  },
  'earth-03': {
    alt: 'Oceanic plates move apart while new crust forms along a rising central ridge.',
    draw(d) {
      d.rect(4, 5, 88, 15, d.c.blue);
      d.rect(4, 22, 35, 14, d.c.gold);
      d.rect(57, 22, 35, 14, d.c.gold);
      d.rect(4, 32, 88, 6, d.c.orange);
      d.triangle(35, 17, 26, 21, d.c.orange);
      d.line(4, 21, 38, 21, d.c.ink);
      d.line(58, 21, 91, 21, d.c.ink);
      d.arrow(34, 27, 13, 27, d.c.ink);
      d.arrow(62, 27, 83, 27, d.c.ink);
      d.arrow(48, 35, 48, 22, d.c.rose);
      d.text('NEW CRUST', 30, 41, d.c.ink);
    },
  },
  'earth-04': {
    alt: 'Two converging continental plates compress rock layers upward into a mountain ridge.',
    draw(d) {
      d.rect(5, 27, 86, 8, d.c.gold);
      d.triangle(31, 5, 34, 27, d.c.gold);
      d.triangle(42, 5, 12, 10, d.c.white);
      for (let y = 28; y <= 34; y += 3) {
        d.line(5, y, 33, y, d.c.orange);
        d.line(33, y, 48, y - 11, d.c.orange);
        d.line(48, y - 11, 63, y, d.c.orange);
        d.line(63, y, 91, y, d.c.orange);
      }
      d.arrow(7, 39, 31, 39, d.c.ink);
      d.arrow(89, 39, 65, 39, d.c.ink);
      d.text('COLLIDE', 34, 41, d.c.ink);
    },
  },
  'earth-05': {
    alt: 'Sunlight supplies energy as water leaves a shrinking puddle and enters the air as vapor.',
    draw(d) {
      sun(d, 80, 12, 7);
      d.rect(5, 36, 61, 4, d.c.blue);
      d.rect(11, 33, 49, 4, d.c.sky);
      d.arrow(73, 22, 59, 31, d.c.gold);
      for (const [x, y] of [[17, 24], [27, 14], [43, 22], [52, 11]]) d.circle(x, y, 1, d.c.blue);
      d.arrow(19, 31, 19, 17, d.c.blue);
      d.arrow(43, 30, 43, 15, d.c.blue);
      d.text('VAPOR', 30, 3, d.c.ink);
      d.text('LIQUID', 8, 42, d.c.ink);
    },
  },
  'earth-06': {
    alt: 'Evaporating water carries energy away from a wet surface, leaving the surface cooler.',
    draw(d) {
      d.rect(5, 31, 62, 8, d.c.cream);
      d.rect(5, 29, 62, 3, d.c.sky);
      d.arrow(29, 25, 29, 9, d.c.orange);
      d.arrow(46, 24, 46, 7, d.c.orange);
      for (const [x, y] of [[17, 23], [20, 14], [38, 19], [55, 21]]) d.circle(x, y, 1, d.c.blue);
      d.text('HEAT OUT', 19, 1, d.c.ink);
      d.rect(77, 13, 7, 22, d.c.ink);
      d.rect(79, 15, 3, 17, d.c.paper);
      d.rect(79, 25, 3, 10, d.c.blue);
      d.circle(80, 36, 5, d.c.blue);
      d.text('COOLER', 20, 42, d.c.ink);
    },
  },
  'earth-07': {
    alt: 'Water fills connected spaces between rock grains rather than one large underground tunnel.',
    draw(d) {
      d.rect(6, 10, 84, 28, d.c.blue);
      for (const [x, y] of [[14, 17], [30, 17], [46, 17], [62, 17], [79, 17], [21, 31], [38, 31], [55, 31], [72, 31], [88, 32]]) {
        d.circle(x, y, 6, d.c.gold);
        d.rect(x - 2, y - 2, 3, 2, d.c.orange);
      }
      d.arrow(12, 25, 32, 25, d.c.white);
      d.arrow(48, 25, 68, 25, d.c.white);
      d.text('WATER IN GAPS', 21, 2, d.c.ink);
      d.rect(15, 42, 4, 4, d.c.gold);
      d.text('ROCK', 22, 42, d.c.ink);
      d.rect(57, 42, 4, 4, d.c.blue);
      d.text('WATER', 64, 42, d.c.ink);
    },
  },
  'earth-08': {
    alt: 'Rainwater has a short route to a stream and a much longer route through deep groundwater.',
    draw(d) {
      cloud(d, 4, 4);
      d.rect(5, 20, 87, 25, d.c.gold);
      d.rect(5, 18, 68, 3, d.c.green);
      d.rect(75, 16, 17, 5, d.c.blue);
      d.arrow(19, 17, 19, 25, d.c.blue);
      d.arrow(22, 26, 78, 26, d.c.blue);
      d.arrow(78, 25, 81, 20, d.c.blue);
      d.line(23, 28, 29, 41, d.c.teal);
      d.arrow(29, 41, 67, 41, d.c.teal);
      d.arrow(68, 40, 84, 22, d.c.teal);
      d.text('SHORT', 44, 10, d.c.ink);
      d.text('LONG', 40, 33, d.c.ink);
    },
  },
  'physics-01': {
    alt: 'Air scatters blue light in several directions away from a beam of sunlight.',
    draw(d) {
      d.rect(2, 2, 92, 38, d.c.sky);
      sun(d, 12, 15, 6);
      d.arrow(23, 15, 44, 15, d.c.gold);
      d.circle(48, 15, 3, d.c.paper);
      d.circle(48, 15, 1, d.c.ink);
      d.arrow(51, 17, 72, 32, d.c.blue);
      d.arrow(49, 11, 62, 4, d.c.blue);
      d.arrow(46, 18, 29, 33, d.c.blue);
      d.arrow(53, 15, 88, 15, d.c.rose);
      d.circle(78, 34, 5, d.c.white);
      d.circle(78, 34, 2, d.c.ink);
      d.text('SCATTER', 33, 42, d.c.ink);
    },
  },
  'physics-02': {
    alt: 'A long, low path through air scatters blue light away while red-orange light reaches the observer.',
    draw(d) {
      d.rect(2, 4, 92, 33, d.c.sky);
      d.rect(2, 37, 92, 3, d.c.green);
      sun(d, 11, 29, 6);
      d.arrow(22, 29, 84, 29, d.c.orange);
      d.arrow(31, 26, 26, 8, d.c.blue);
      d.arrow(48, 26, 55, 10, d.c.blue);
      d.arrow(64, 26, 72, 6, d.c.blue);
      d.circle(89, 28, 3, d.c.white);
      d.circle(89, 28, 1, d.c.ink);
      d.text('LONG PATH', 30, 42, d.c.ink);
    },
  },
  'physics-03': {
    alt: 'A prism separates a white beam into a fan of colors, with violet bending more than red.',
    draw(d) {
      d.triangle(37, 7, 28, 30, d.c.sky);
      d.line(37, 36, 51, 7, d.c.blue);
      d.line(51, 7, 65, 36, d.c.blue);
      d.line(37, 36, 65, 36, d.c.blue);
      d.rect(4, 19, 40, 4, d.c.ink);
      d.rect(4, 20, 42, 2, d.c.white);
      d.line(46, 21, 57, 24, d.c.white);
      for (const [dy, color] of [[0, 'rose'], [4, 'orange'], [8, 'gold'], [12, 'green'], [16, 'blue'], [20, 'purple']]) {
        d.line(57, 24, 91, 24 + dy, d.c[color]);
      }
      d.text('WHITE', 7, 9, d.c.ink);
    },
  },
  'physics-04': {
    alt: 'Light from a lightning event reaches an eye before the slower sound wave reaches an ear.',
    draw(d) {
      cloud(d, 2, 4);
      d.line(20, 16, 14, 24, d.c.gold);
      d.line(14, 24, 21, 24, d.c.gold);
      d.line(21, 24, 15, 33, d.c.gold);
      d.arrow(25, 22, 79, 22, d.c.gold);
      d.circle(87, 22, 5, d.c.ink);
      d.circle(87, 22, 3, d.c.white);
      d.circle(87, 22, 1, d.c.ink);
      for (let x = 28; x <= 46; x += 6) {
        d.line(x, 32, x + 3, 35, d.c.purple);
        d.line(x + 3, 35, x, 38, d.c.purple);
      }
      d.text('LIGHT', 52, 11, d.c.ink);
      d.text('SOUND', 28, 42, d.c.ink);
      d.line(86, 32, 90, 32, d.c.ink);
      d.line(90, 32, 90, 39, d.c.ink);
      d.line(90, 39, 85, 42, d.c.ink);
    },
  },
  'physics-05': {
    alt: 'One lightning bolt stays inside a cloud and another connects two clouds without reaching the ground.',
    draw(d) {
      cloud(d, 7, 8);
      cloud(d, 59, 8);
      for (const [x1, y1, x2, y2] of [[24, 8, 19, 15], [19, 15, 25, 15], [25, 15, 20, 22], [33, 16, 44, 20], [44, 20, 43, 13], [43, 13, 61, 18]]) {
        d.line(x1, y1, x2, y2, d.c.gold);
      }
      d.rect(3, 43, 90, 3, d.c.green);
      d.text('IN CLOUDS', 29, 33, d.c.ink);
    },
  },
  'physics-06': {
    alt: 'The same six water molecules are close together in liquid and spread into an open structure in ice.',
    draw(d) {
      d.text('LIQUID', 7, 2, d.c.ink);
      d.text('ICE', 67, 2, d.c.ink);
      d.rect(4, 11, 33, 29, d.c.sky);
      for (const [x, y] of [[11, 20], [21, 18], [29, 23], [14, 31], [24, 31], [31, 35]]) d.circle(x, y, 2, d.c.blue);
      const ring = [[64, 14], [77, 14], [84, 26], [77, 38], [64, 38], [57, 26]];
      for (let i = 0; i < ring.length; i++) {
        const next = ring[(i + 1) % ring.length];
        d.line(ring[i][0], ring[i][1], next[0], next[1], d.c.sky);
      }
      for (const [x, y] of ring) d.circle(x, y, 2, d.c.blue);
      d.arrow(41, 26, 50, 26, d.c.ink);
      d.text('MORE SPACE', 52, 43, d.c.ink);
    },
  },
  'physics-07': {
    alt: 'A small iceberg tip sits above the waterline while a much larger body extends below it.',
    draw(d) {
      d.rect(3, 14, 90, 31, d.c.blue);
      d.triangle(44, 4, 10, 11, d.c.white);
      for (const [x, y, w, h] of [[33, 15, 33, 5], [28, 20, 40, 6], [32, 26, 33, 7], [39, 33, 20, 5], [45, 38, 8, 5]]) d.rect(x, y, w, h, d.c.sky);
      d.line(3, 14, 43, 14, d.c.teal);
      d.line(55, 14, 92, 14, d.c.teal);
      d.text('TIP', 13, 4, d.c.ink);
      d.arrow(27, 8, 43, 9, d.c.ink);
      d.text('HIDDEN', 68, 36, d.c.white);
      d.arrow(76, 31, 65, 27, d.c.white);
    },
  },
  'physics-08': {
    alt: 'A space station and its occupant orbit Earth while gravity pulls both inward and sideways motion carries them onward.',
    draw(d) {
      orbit(d, 47, 25, 32, 17);
      earth(d, 47, 26, 11);
      d.rect(69, 7, 19, 13, d.c.ink);
      d.rect(71, 9, 15, 9, d.c.sky);
      d.rect(63, 9, 5, 9, d.c.blue);
      d.rect(90, 9, 5, 9, d.c.blue);
      d.circle(77, 12, 2, d.c.white);
      d.line(77, 14, 77, 16, d.c.white);
      d.line(82, 12, 84, 10, d.c.gold);
      d.arrow(74, 23, 61, 27, d.c.rose);
      d.arrow(77, 4, 62, 4, d.c.ink);
      d.text('FALL TOGETHER', 23, 43, d.c.ink);
    },
  },
  'nature-01': {
    alt: 'A green plant receives sunlight, while a fungus draws nutrients from decaying wood.',
    draw(d) {
      sun(d, 11, 9, 5);
      d.rect(22, 22, 2, 15, d.c.green);
      leaf(d, 25, 24);
      d.arrow(13, 18, 19, 22, d.c.gold);
      d.rect(53, 32, 36, 8, d.c.orange);
      d.line(57, 35, 84, 35, d.c.ink);
      mushroom(d, 73, 23);
      d.arrow(61, 33, 69, 28, d.c.gold);
      d.text('PLANT', 12, 42, d.c.ink);
      d.text('FUNGUS', 60, 42, d.c.ink);
    },
  },
  'nature-02': {
    alt: 'A small mushroom rises above soil while a much wider branching mycelium network spreads underneath.',
    draw(d) {
      d.rect(3, 27, 90, 18, d.c.cream);
      d.line(3, 26, 92, 26, d.c.green);
      mushroom(d, 47, 15);
      for (const [x1, y1, x2, y2] of [[47, 27, 47, 33], [47, 33, 26, 34], [47, 33, 66, 34], [26, 34, 9, 30], [26, 34, 17, 43], [26, 34, 35, 43], [66, 34, 83, 30], [66, 34, 77, 43], [66, 34, 54, 43], [9, 30, 6, 36], [17, 43, 7, 40], [83, 30, 89, 37], [54, 43, 45, 40]]) {
        d.line(x1, y1, x2, y2, d.c.purple);
      }
      d.text('MYCELIUM', 60, 3, d.c.ink);
      d.line(74, 10, 82, 29, d.c.ink);
    },
  },
  'nature-03': {
    alt: 'Fungi break down a fallen leaf, releasing nutrients that can support a growing seedling.',
    draw(d) {
      d.rect(3, 33, 90, 7, d.c.cream);
      leaf(d, 14, 24, d.c.orange);
      d.rect(25, 27, 3, 2, d.c.orange);
      d.rect(28, 31, 2, 2, d.c.orange);
      mushroom(d, 48, 23);
      d.arrow(31, 31, 40, 31, d.c.gold);
      d.arrow(59, 32, 72, 30, d.c.gold);
      d.rect(79, 20, 2, 17, d.c.green);
      leaf(d, 75, 21);
      leaf(d, 84, 15);
      d.line(80, 35, 76, 39, d.c.green);
      d.text('LEAF', 6, 42, d.c.ink);
      d.text('SOIL', 41, 42, d.c.ink);
      d.text('GROW', 76, 42, d.c.ink);
    },
  },
  'nature-04': {
    alt: 'A daisy head is magnified to reveal many small florets clustered in its central disc.',
    draw(d) {
      d.rect(23, 27, 2, 12, d.c.green);
      for (const [x, y] of [[18, 8], [18, 28], [8, 18], [28, 18], [11, 11], [25, 11], [11, 25], [25, 25]]) d.rect(x, y, 8, 8, d.c.cream);
      d.circle(22, 22, 8, d.c.gold);
      d.circle(22, 22, 3, d.c.orange);
      d.line(31, 16, 57, 10, d.c.ink);
      d.line(31, 28, 57, 35, d.c.ink);
      d.circle(73, 23, 17, d.c.purple);
      d.circle(73, 23, 15, d.c.cream);
      for (const [x, y] of [[67, 15], [77, 15], [63, 24], [73, 24], [83, 24], [68, 33], [78, 33]]) {
        d.circle(x, y, 4, d.c.gold);
        d.circle(x, y, 1, d.c.orange);
      }
      d.text('MANY FLOWERS', 24, 42, d.c.ink);
    },
  },
  'nature-05': {
    alt: 'A bee entering a Salvia flower pushes a lever that places pollen on its back.',
    draw(d) {
      d.rect(71, 28, 3, 18, d.c.green);
      d.rect(50, 10, 27, 5, d.c.purple);
      d.rect(74, 13, 9, 16, d.c.purple);
      d.rect(56, 30, 22, 5, d.c.purple);
      d.circle(64, 20, 2, d.c.orange);
      d.line(64, 20, 64, 29, d.c.gold);
      d.line(64, 20, 50, 18, d.c.gold);
      d.line(50, 18, 46, 22, d.c.gold);
      d.circle(41, 22, 4, d.c.sky);
      d.rect(34, 25, 20, 8, d.c.gold);
      d.rect(39, 25, 3, 8, d.c.ink);
      d.rect(46, 25, 3, 8, d.c.ink);
      d.circle(56, 28, 4, d.c.ink);
      d.rect(43, 22, 3, 2, d.c.orange);
      d.arrow(8, 29, 28, 29, d.c.ink);
      d.text('POLLEN', 7, 3, d.c.ink);
      d.line(31, 9, 44, 20, d.c.orange);
    },
  },
  'nature-06': {
    alt: 'A flytrap remains open after one touch, then closes after a second touch within roughly twenty seconds.',
    draw(d) {
      flytrap(d, 22, 26);
      flytrap(d, 73, 26, true);
      d.circle(23, 15, 2, d.c.ink);
      d.line(21, 13, 18, 11, d.c.ink);
      d.line(25, 13, 28, 11, d.c.ink);
      d.text('1', 8, 2, d.c.ink);
      d.text('2', 59, 2, d.c.ink);
      d.arrow(40, 25, 53, 25, d.c.ink);
      d.text('20 SEC', 36, 42, d.c.ink);
    },
  },
  'nature-07': {
    alt: 'A small region in the southeastern United States connects to a flytrap growing in a wet, nutrient-poor habitat.',
    draw(d) {
      // A locator sketch, not a map of state boundaries or the exact range.
      d.rect(6, 12, 32, 16, d.c.cream);
      d.rect(10, 10, 22, 7, d.c.cream);
      d.rect(13, 25, 25, 6, d.c.cream);
      d.rect(31, 16, 13, 11, d.c.cream);
      d.rect(36, 27, 5, 7, d.c.cream);
      d.rect(39, 32, 3, 4, d.c.cream);
      d.circle(41, 24, 2, d.c.rose);
      d.arrow(47, 25, 57, 25, d.c.ink);
      d.rect(59, 38, 34, 6, d.c.cream);
      d.rect(61, 39, 12, 3, d.c.blue);
      flytrap(d, 77, 26);
      d.text('CAROLINAS', 5, 2, d.c.ink);
      d.text('SMALL HOME', 4, 42, d.c.ink);
    },
  },
  'nature-08': {
    alt: 'Dry seeds are stored in jars on cold shelves as a backup collection for the future.',
    draw(d) {
      d.rect(8, 5, 43, 40, d.c.ink);
      d.rect(10, 7, 39, 36, d.c.sky);
      for (const y of [18, 34]) {
        d.rect(11, y + 7, 37, 2, d.c.ink);
        for (const x of [16, 32]) {
          d.rect(x, y - 4, 9, 10, d.c.white);
          d.rect(x - 1, y - 6, 11, 3, d.c.teal);
          d.circle(x + 3, y + 1, 1, d.c.orange);
          d.circle(x + 6, y + 3, 1, d.c.gold);
        }
      }
      d.text('DRY', 69, 5, d.c.ink);
      d.circle(76, 17, 3, d.c.gold);
      d.circle(85, 20, 2, d.c.orange);
      d.arrow(72, 26, 56, 26, d.c.ink);
      d.line(70, 31, 70, 41, d.c.blue);
      d.line(65, 33, 75, 39, d.c.blue);
      d.line(65, 39, 75, 33, d.c.blue);
      d.text('BANK', 80, 39, d.c.ink);
    },
  },
};
