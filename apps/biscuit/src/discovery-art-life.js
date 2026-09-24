const path = (d, points, color) => points.slice(1).forEach((point, i) => d.line(...points[i], ...point, color));
const dots = (d, x, y, rows, columns, color, step = 7) => {
  for (let row = 0; row < rows; row++) for (let column = 0; column < columns; column++) d.circle(x + column * step, y + row * step, 2, color);
};
function face(d, x, y, color = d.c.gold) {
  d.circle(x, y, 8, color);
  d.rect(x - 4, y - 1, 2, 2, d.c.ink);
  d.rect(x + 3, y - 1, 2, 2, d.c.ink);
  path(d, [[x - 3, y + 4], [x, y + 5], [x + 3, y + 4]], d.c.ink);
}
function person(d, x, y, color) {
  d.circle(x, y, 3, d.c.gold);
  d.rect(x - 3, y + 4, 7, 8, color);
  d.line(x - 2, y + 12, x - 4, y + 17, d.c.ink);
  d.line(x + 2, y + 12, x + 4, y + 17, d.c.ink);
}
function book(d, x, y, color) {
  d.rect(x, y, 27, 21, color);
  d.rect(x + 2, y + 2, 23, 16, d.c.white);
  d.line(x + 13, y + 2, x + 13, y + 19, color);
  for (let line = 0; line < 3; line++) {
    d.line(x + 4, y + 5 + line * 4, x + 10, y + 5 + line * 4, d.c.sky);
    d.line(x + 16, y + 5 + line * 4, x + 22, y + 5 + line * 4, d.c.sky);
  }
}
function braille(d, x, y, raised) {
  for (let dot = 1; dot <= 6; dot++) {
    const cx = x + (dot > 3 ? 11 : 0), cy = y + (dot - 1) % 3 * 10;
    d.circle(cx, cy, 3, raised.includes(dot) ? d.c.ink : d.c.sky);
    if (!raised.includes(dot)) d.circle(cx, cy, 1, d.c.paper);
  }
}
function octopus(d, x, y, color, spots) {
  d.circle(x, y, 10, color);
  d.rect(x - 9, y, 19, 9, color);
  const ends = [[-19, 15], [-15, 20], [-9, 18], [-3, 22], [3, 20], [9, 21], [15, 17], [19, 13]];
  ends.forEach(([dx, dy], i) => path(d, [[x - 8 + i * 2, y + 6], [x + dx, y + dy - 3], [x + dx + (dx < 0 ? -2 : 2), y + dy - 3]], color));
  d.circle(x - 4, y - 4, 2, spots); d.circle(x + 4, y + 1, 2, spots);
  d.rect(x - 4, y + 3, 2, 2, d.c.ink); d.rect(x + 3, y + 3, 2, 2, d.c.ink);
}
function lungs(d, x, y, expanded) {
  d.rect(x - 1, y - 15, 3, 15, d.c.ink);
  d.circle(x - 7, y, expanded ? 8 : 6, d.c.rose);
  d.circle(x + 7, y, expanded ? 8 : 6, d.c.rose);
  d.rect(x - (expanded ? 15 : 13), y, expanded ? 12 : 10, expanded ? 12 : 8, d.c.rose);
  d.rect(x + 4, y, expanded ? 12 : 10, expanded ? 12 : 8, d.c.rose);
  d.line(x, y - 5, x - 7, y, d.c.ink); d.line(x, y - 5, x + 7, y, d.c.ink);
}

export const lifePictures = {
  'math-01': { alt: 'Binary 1101 selects the place values 8, 4 and 1 to make 13.', draw(d) {
    [8, 4, 2, 1].forEach((value, i) => {
      const x = 10 + i * 21, on = i !== 2;
      d.text(String(value), x + 4, 4, d.c.ink);
      d.rect(x, 14, 13, 18, on ? d.c.teal : d.c.sky);
      d.text(on ? '1' : '0', x + 5, 21, on ? d.c.white : d.c.ink);
    });
    d.text('8+4+0+1=13', 28, 40, d.c.ink);
  } },
  'math-02': { alt: 'Each of four two-bit patterns branches into two three-bit patterns, making eight.', draw(d) {
    d.text('2 BITS', 4, 1, d.c.ink); d.text('3 BITS', 58, 1, d.c.ink);
    ['00', '01', '10', '11'].forEach((bits, i) => {
      const y = 11 + i * 10;
      d.text(bits, 10, y, d.c.ink);
      d.line(25, y + 2, 36, y + 2, d.c.teal);
      d.arrow(36, y + 2, 47, y, d.c.teal);
      d.arrow(36, y + 2, 70, y + 4, d.c.purple);
      d.text(`0${bits}`, 50, y - 1, d.c.teal);
      d.text(`1${bits}`, 74, y + 2, d.c.purple);
    });
  } },
  'math-03': { alt: 'One changed square makes its row and column odd; the highlighted crossing identifies it.', draw(d) {
    const cells = [[1, 0, 1, 0], [0, 1, 0, 0], [1, 1, 0, 0], [0, 0, 0, 0]];
    d.rect(26, 13, 39, 10, d.c.rose); d.rect(45, 4, 10, 39, d.c.rose);
    cells.forEach((row, r) => row.forEach((value, c) => d.rect(28 + c * 10, 6 + r * 10, 5, 5, value ? d.c.ink : d.c.white)));
    d.arrow(76, 18, 58, 18, d.c.ink); d.arrow(50, 46, 50, 40, d.c.ink);
    d.text('ODD', 78, 16, d.c.ink); d.text('1 FLIP', 1, 1, d.c.ink);
  } },
  'math-04': { alt: 'A triangle of ten dots pairs with a reversed triangle to fill a four-by-five rectangle.', draw(d) {
    for (let row = 0; row < 4; row++) dots(d, 9, 9 + row * 8, 1, row + 1, d.c.teal);
    d.arrow(38, 21, 48, 21, d.c.ink);
    for (let row = 0; row < 4; row++) for (let column = 0; column < 5; column++) d.circle(57 + column * 7, 9 + row * 8, 2, column <= row ? d.c.teal : d.c.rose);
    d.text('10', 15, 40, d.c.teal); d.text('10+10=20', 54, 40, d.c.ink);
  } },
  'math-05': { alt: 'Squares of one, four, nine and sixteen dots grow with differently coloured odd-number borders.', draw(d) {
    [1, 2, 3, 4].forEach((size, i) => {
      const x = [7, 23, 45, 72][i], colors = [d.c.teal, d.c.gold, d.c.rose, d.c.purple];
      for (let row = 0; row < size; row++) for (let column = 0; column < size; column++) d.circle(x + column * 5, 29 - (size - 1 - row) * 5, 2, colors[Math.max(row, column)]);
      d.text(String(size * size), x + (size > 2 ? 4 : 0), 38, d.c.ink);
    });
    d.text('1+3+5+7=16', 27, 3, d.c.ink);
  } },
  'math-06': { alt: 'Six people form fifteen distinct pairs, shown by all the lines joining six dots.', draw(d) {
    const people = [[48, 5], [66, 14], [66, 34], [48, 43], [30, 34], [30, 14]];
    people.forEach((point, i) => people.slice(i + 1).forEach(other => d.line(...point, ...other, d.c.sky)));
    people.forEach(([x, y]) => { d.circle(x, y, 4, d.c.teal); d.rect(x - 1, y - 1, 1, 1, d.c.white); });
    d.text('6', 9, 15, d.c.ink); d.text('PEOPLE', 1, 23, d.c.ink);
    d.text('15', 79, 15, d.c.ink); d.text('PAIRS', 74, 23, d.c.ink);
  } },
  'math-07': { alt: 'A flat paper strip becomes a loop with a half twist; coloured edges cross at the twist.', draw(d) {
    d.rect(3, 20, 25, 10, d.c.sky); d.line(3, 19, 28, 19, d.c.teal); d.line(3, 30, 28, 30, d.c.gold);
    d.arrow(29, 25, 37, 25, d.c.ink);
    path(d, [[42, 24], [49, 12], [67, 8], [81, 17], [74, 31], [66, 39], [49, 36], [42, 24]], d.c.teal);
    path(d, [[47, 24], [53, 18], [66, 15], [74, 19], [82, 31], [68, 44], [46, 40], [37, 25], [47, 24]], d.c.gold);
    d.text('HALF TWIST', 45, 1, d.c.ink);
    d.arrow(88, 21, 81, 25, d.c.rose);
  } },
  'math-08': { alt: 'Towers of seven, eight and nine blocks balance into three equal towers of eight.', draw(d) {
    [[7, 8, 9], [8, 8, 8]].forEach((counts, group) => counts.forEach((count, i) => {
      const x = 7 + group * 56 + i * 10;
      for (let block = 0; block < count; block++) d.rect(x, 33 - block * 3, 6, 2, [d.c.teal, d.c.gold, d.c.purple][i]);
      d.text(String(count), x + 1, 40, d.c.ink);
    }));
    d.arrow(41, 24, 54, 24, d.c.ink);
    d.line(30, 4, 10, 4, d.c.orange); d.arrow(10, 4, 10, 10, d.c.orange);
  } },
  'animals-01': { alt: 'A puppy sniffs scent specks; a magnified nose shows the folded passages inside.', draw(d) {
    d.circle(22, 24, 13, d.c.gold); d.rect(7, 10, 7, 20, d.c.orange); d.rect(31, 10, 7, 20, d.c.orange);
    d.rect(16, 20, 2, 3, d.c.ink); d.rect(27, 20, 2, 3, d.c.ink);
    d.circle(22, 29, 6, d.c.cream); d.rect(19, 26, 7, 4, d.c.ink);
    d.arrow(36, 28, 47, 23, d.c.teal);
    d.circle(70, 23, 19, d.c.sky); d.circle(70, 23, 17, d.c.white);
    path(d, [[55, 18], [77, 18], [77, 23], [60, 23], [60, 28], [80, 28]], d.c.rose);
    path(d, [[59, 12], [81, 12], [84, 16], [84, 32], [77, 36], [59, 36]], d.c.rose);
    [[47, 12], [44, 32], [53, 6]].forEach(([x, y]) => d.circle(x, y, 1, d.c.teal));
  } },
  'animals-02': { alt: 'A honeybee dances a looping route with a waggle in the middle, while an arrow points toward flowers.', draw(d) {
    path(d, [[43, 27], [30, 20], [25, 27], [30, 38], [42, 33], [45, 24], [53, 15], [65, 20], [64, 29], [53, 33], [43, 27]], d.c.orange);
    path(d, [[44, 28], [47, 25], [43, 22], [47, 19]], d.c.ink);
    d.circle(42, 17, 4, d.c.sky); d.circle(50, 17, 4, d.c.sky);
    d.rect(43, 18, 7, 10, d.c.gold); d.rect(43, 21, 7, 2, d.c.ink); d.rect(43, 26, 7, 2, d.c.ink);
    d.circle(46, 16, 3, d.c.ink); d.rect(47, 15, 1, 1, d.c.white);
    d.arrow(59, 11, 74, 6, d.c.teal);
    [[81, 7], [88, 19]].forEach(([x, y]) => { d.line(x, y, x, y + 12, d.c.green); d.circle(x, y, 4, d.c.rose); d.circle(x, y, 1, d.c.gold); });
  } },
  'animals-03': { alt: 'Two octopuses show different skin colours and spots beside pebbles and seaweed.', draw(d) {
    for (let x = 5; x < 44; x += 10) d.circle(x, 43, 3, d.c.sky);
    for (let x = 58; x < 93; x += 12) path(d, [[x, 46], [x - 3, 37], [x + 2, 30]], d.c.green);
    octopus(d, 24, 18, d.c.rose, d.c.orange); octopus(d, 70, 18, d.c.teal, d.c.green);
  } },
  'animals-04': { alt: 'A penguin beside a magnified feather coat, with small air pockets beneath overlapping feathers.', draw(d) {
    d.circle(22, 15, 10, d.c.ink); d.rect(12, 15, 21, 23, d.c.ink); d.circle(22, 37, 10, d.c.ink);
    d.circle(23, 28, 8, d.c.white); d.rect(18, 17, 10, 15, d.c.white);
    d.rect(17, 11, 2, 2, d.c.white); d.rect(26, 11, 2, 2, d.c.white); d.rect(21, 15, 5, 3, d.c.orange);
    d.rect(12, 44, 8, 3, d.c.orange); d.rect(25, 44, 8, 3, d.c.orange);
    d.arrow(36, 27, 47, 27, d.c.ink);
    d.rect(52, 17, 37, 21, d.c.sky); d.rect(52, 35, 37, 5, d.c.gold);
    for (let x = 54; x < 89; x += 8) { d.circle(x + 3, 28, 2, d.c.white); d.line(x, 11, x + 6, 21, d.c.ink); d.line(x, 13, x + 6, 23, d.c.ink); }
    d.text('AIR', 65, 4, d.c.teal);
  } },
  'animals-05': { alt: 'An elephant makes a low rumble drawn as a long sound wave travelling through trees.', draw(d) {
    d.circle(20, 23, 13, d.c.purple); d.rect(10, 23, 22, 12, d.c.purple);
    d.rect(11, 32, 6, 11, d.c.purple); d.rect(25, 32, 6, 11, d.c.purple);
    d.circle(33, 21, 9, d.c.purple); d.circle(26, 21, 7, d.c.rose);
    d.rect(38, 21, 5, 18, d.c.purple); d.rect(35, 36, 7, 4, d.c.purple); d.rect(35, 18, 2, 2, d.c.ink);
    path(d, [[47, 26], [54, 18], [62, 16], [70, 26], [78, 36], [86, 34], [94, 26]], d.c.teal);
    [57, 84].forEach(x => { d.rect(x, 6, 3, 9, d.c.orange); d.triangle(x - 5, 1, 13, 10, d.c.green); });
  } },
  'animals-06': { alt: 'A swimming platypus has closed eyes and a broad sensitive bill near a small river-bottom prey animal.', draw(d) {
    d.rect(0, 2, 96, 2, d.c.sky); d.rect(0, 43, 96, 5, d.c.cream);
    d.rect(6, 18, 18, 12, d.c.orange); d.circle(33, 24, 13, d.c.gold); d.circle(48, 22, 9, d.c.gold);
    d.rect(50, 24, 21, 8, d.c.orange); d.line(48, 18, 52, 18, d.c.ink);
    d.line(29, 33, 21, 38, d.c.orange); d.line(43, 32, 48, 37, d.c.orange);
    d.circle(83, 38, 3, d.c.rose); d.line(79, 38, 74, 41, d.c.rose);
    path(d, [[69, 32], [72, 35], [75, 32]], d.c.teal); path(d, [[74, 28], [79, 31], [82, 28]], d.c.teal);
  } },
  'animals-07': { alt: 'A bat sends a call toward an insect, and a second arrow brings its echo back.', draw(d) {
    d.triangle(3, 14, 23, 17, d.c.purple); d.triangle(27, 14, 23, 17, d.c.purple);
    d.circle(26, 23, 6, d.c.ink); d.triangle(20, 10, 5, 10, d.c.ink); d.triangle(28, 10, 5, 10, d.c.ink);
    d.rect(23, 21, 1, 1, d.c.white); d.rect(28, 21, 1, 1, d.c.white);
    d.arrow(50, 17, 76, 17, d.c.teal); d.arrow(76, 31, 49, 31, d.c.rose);
    d.text('CALL', 52, 7, d.c.teal); d.text('ECHO', 53, 39, d.c.rose);
    d.circle(85, 22, 3, d.c.gold); d.circle(81, 19, 3, d.c.sky); d.circle(88, 19, 3, d.c.sky); d.line(85, 23, 85, 30, d.c.ink);
  } },
  'animals-08': { alt: 'A human hand and a bat wing share finger bones, with much longer fingers spreading the bat wing.', draw(d) {
    d.rect(14, 27, 13, 12, d.c.gold); d.rect(18, 37, 7, 9, d.c.gold);
    [[14, 18], [18, 10], [22, 8], [26, 13], [33, 25]].forEach(([x, y]) => d.line(21, 34, x, y, d.c.ink));
    path(d, [[53, 35], [49, 13], [61, 9], [76, 8], [93, 19], [90, 37], [76, 30], [65, 36], [53, 35]], d.c.purple);
    [[49, 13], [61, 9], [76, 8], [93, 19]].forEach(([x, y]) => d.line(53, 35, x, y, d.c.ink));
    d.line(53, 35, 48, 46, d.c.ink); d.line(53, 35, 46, 28, d.c.ink);
    d.text('HAND', 7, 1, d.c.ink); d.text('WING', 62, 40, d.c.purple);
  } },
  'language-01': { alt: 'A smiling storyteller uses hands, face and space; the picture does not represent a specific sign.', draw(d) {
    face(d, 48, 15); d.rect(39, 24, 19, 21, d.c.teal);
    d.line(40, 28, 29, 21, d.c.teal); d.line(56, 28, 68, 19, d.c.teal);
    d.rect(24, 15, 7, 8, d.c.gold); d.rect(67, 12, 7, 9, d.c.gold);
    [24, 27, 30].forEach(x => d.line(x, 12, x, 18, d.c.gold));
    [67, 70, 73].forEach(x => d.line(x, 9, x, 15, d.c.gold));
    d.rect(3, 4, 15, 11, d.c.sky); d.rect(8, 15, 3, 3, d.c.sky); d.text('HI', 6, 7, d.c.ink);
    d.rect(79, 30, 13, 10, d.c.rose); d.rect(77, 34, 3, 3, d.c.rose); d.text('!', 84, 32, d.c.ink);
  } },
  'language-02': { alt: 'Two signed-language conversations are labelled ASL and BSL, showing that sign languages differ.', draw(d) {
    d.rect(3, 2, 33, 13, d.c.sky); d.text('ASL', 14, 6, d.c.ink);
    d.rect(59, 2, 33, 13, d.c.rose); d.text('BSL', 70, 6, d.c.ink);
    [22, 73].forEach((x, i) => { face(d, x, 26); d.rect(x - 8, 35, 17, 11, i ? d.c.purple : d.c.teal); d.circle(x - 14, 31, 3, d.c.gold); d.circle(x + 14, 32, 3, d.c.gold); });
    d.line(47, 8, 47, 41, d.c.gold);
  } },
  'language-03': { alt: 'Raised eyebrows, eyes and a question bubble highlight the face’s role in signed grammar.', draw(d) {
    d.circle(44, 27, 17, d.c.gold);
    d.line(32, 15, 39, 12, d.c.ink); d.line(48, 12, 55, 15, d.c.ink);
    d.rect(35, 23, 3, 3, d.c.ink); d.rect(50, 23, 3, 3, d.c.ink);
    path(d, [[38, 35], [44, 37], [50, 35]], d.c.ink);
    d.arrow(10, 10, 28, 14, d.c.teal); d.rect(69, 5, 19, 17, d.c.sky); d.rect(66, 20, 6, 4, d.c.sky);
    path(d, [[75, 10], [76, 8], [81, 8], [83, 10], [83, 12], [79, 15], [79, 17]], d.c.ink); d.rect(79, 19, 1, 1, d.c.ink);
  } },
  'language-04': { alt: 'A book and fingertip beside a magnified six-position braille cell, numbered down each column.', draw(d) {
    book(d, 3, 16, d.c.purple); d.arrow(34, 25, 44, 25, d.c.ink);
    braille(d, 57, 11, [1, 2, 3, 4, 5, 6]);
    [1, 2, 3].forEach((dot, i) => d.text(String(dot), 47, 9 + i * 10, d.c.ink));
    [4, 5, 6].forEach((dot, i) => d.text(String(dot), 76, 9 + i * 10, d.c.ink));
    d.circle(68, 42, 5, d.c.gold); d.rect(64, 42, 9, 6, d.c.gold);
  } },
  'language-05': { alt: 'The English word AND becomes its single contracted-braille cell, with dots 1, 2, 3, 4 and 6 raised.', draw(d) {
    ['A', 'N', 'D'].forEach((letter, i) => { d.rect(4 + i * 11, 17, 9, 13, d.c.sky); d.text(letter, 7 + i * 11, 21, d.c.ink); });
    d.arrow(40, 24, 54, 24, d.c.teal);
    d.rect(60, 7, 28, 35, d.c.cream); braille(d, 69, 14, [1, 2, 3, 4, 6]);
    d.text('ONE CELL', 3, 40, d.c.ink);
  } },
  'language-06': { alt: 'A stylized Rosetta Stone has three writing bands labelled hieroglyphs, Demotic and Greek.', draw(d) {
    d.rect(5, 11, 30, 33, d.c.purple); d.rect(10, 4, 22, 10, d.c.purple); d.rect(16, 1, 13, 4, d.c.purple);
    for (let row = 0; row < 3; row++) {
      const y = 14 + row * 11;
      for (let mark = 0; mark < 5; mark++) { d.line(9 + mark * 4, y, 11 + mark * 4, y - 2 + row, d.c.white); d.rect(9 + mark * 4, y + 3, 2, 1, d.c.white); }
      d.line(36, y + 1, 43, y + 1, d.c.ink);
    }
    d.text('HIEROGLYPHS', 46, 12, d.c.ink); d.text('DEMOTIC', 46, 23, d.c.ink); d.text('GREEK', 46, 34, d.c.ink);
  } },
  'language-07': { alt: 'Two people on opposite sides of a valley exchange a whistled message over the gap.', draw(d) {
    d.triangle(0, 19, 40, 29, d.c.green); d.triangle(58, 19, 38, 29, d.c.teal);
    person(d, 15, 10, d.c.purple); person(d, 81, 9, d.c.orange);
    path(d, [[23, 13], [29, 8], [36, 12], [42, 7], [49, 11], [55, 6], [62, 10], [69, 6]], d.c.blue);
    d.arrow(69, 6, 75, 9, d.c.blue); d.rect(44, 40, 7, 8, d.c.sky);
  } },
  'language-08': { alt: 'One speaker switches between speech bubbles reading HOLA and HELLO.', draw(d) {
    d.rect(2, 3, 34, 14, d.c.teal); d.rect(29, 17, 5, 5, d.c.teal); d.text('HOLA', 11, 8, d.c.white);
    d.rect(58, 3, 36, 14, d.c.purple); d.rect(60, 17, 5, 5, d.c.purple); d.text('HELLO', 66, 8, d.c.white);
    d.arrow(39, 9, 54, 9, d.c.gold); face(d, 47, 31); d.rect(39, 40, 17, 8, d.c.rose);
  } },
  'body-01': { alt: 'Sound reaches the eardrum, three tiny bones and a spiral cochlea before signals travel to the brain.', draw(d) {
    path(d, [[5, 19], [8, 14], [11, 19], [14, 14], [17, 19]], d.c.teal);
    path(d, [[30, 37], [22, 33], [19, 23], [21, 11], [28, 6], [35, 10], [37, 21], [31, 27], [30, 33]], d.c.orange);
    d.line(29, 23, 42, 23, d.c.ink); d.line(42, 16, 42, 30, d.c.rose);
    [[47, 21], [53, 23], [59, 21]].forEach(([x, y]) => { d.circle(x, y, 2, d.c.gold); });
    d.line(47, 21, 59, 21, d.c.gold);
    path(d, [[61, 21], [66, 13], [76, 13], [81, 19], [81, 28], [74, 32], [67, 27], [67, 20], [73, 18], [76, 23], [73, 26]], d.c.purple);
    d.arrow(83, 23, 94, 23, d.c.teal); d.text('BRAIN', 75, 39, d.c.ink);
  } },
  'body-02': { alt: 'Three differently oriented inner-ear loops sense turns; an arrow follows one loop.', draw(d) {
    path(d, [[27, 24], [31, 19], [48, 17], [66, 19], [71, 24], [66, 29], [48, 31], [31, 29], [27, 24]], d.c.teal);
    path(d, [[48, 4], [53, 8], [56, 21], [53, 36], [48, 42], [43, 36], [40, 21], [43, 8], [48, 4]], d.c.purple);
    path(d, [[31, 7], [40, 8], [57, 20], [66, 35], [64, 41], [56, 37], [40, 24], [30, 12], [31, 7]], d.c.orange);
    d.circle(48, 25, 3, d.c.gold); d.arrow(75, 15, 78, 25, d.c.teal); d.arrow(78, 25, 73, 33, d.c.teal);
    d.text('TURN', 5, 39, d.c.ink);
  } },
  'body-03': { alt: 'Light focuses through an eye onto the retina, then a nerve carries signals toward the brain.', draw(d) {
    d.circle(42, 24, 19, d.c.sky); d.circle(42, 24, 17, d.c.white);
    d.rect(28, 15, 4, 19, d.c.blue); d.line(57, 15, 57, 33, d.c.rose);
    [17, 24, 31].forEach(y => { d.line(5, y, 30, y, d.c.gold); d.line(30, y, 56, 24, d.c.gold); });
    d.arrow(60, 24, 77, 24, d.c.teal);
    d.circle(85, 19, 6, d.c.rose); d.circle(89, 26, 6, d.c.rose); d.circle(82, 27, 5, d.c.rose);
    d.text('LIGHT', 2, 2, d.c.ink); d.text('SIGNALS', 63, 39, d.c.ink);
  } },
  'body-04': { alt: 'Three different scent molecules activate three different combinations of receptors.', draw(d) {
    const patterns = [[1, 1, 0], [0, 1, 1], [1, 0, 1]], colors = [d.c.gold, d.c.rose, d.c.teal];
    patterns.forEach((row, i) => {
      const y = 9 + i * 14;
      d.circle(8, y, 3, colors[i]); d.circle(12, y + 2, 2, colors[i]); d.arrow(19, y, 32, y, colors[i]);
      row.forEach((on, receptor) => { d.rect(41 + receptor * 15, y - 4, 9, 9, on ? colors[i] : d.c.sky); if (on) d.rect(44 + receptor * 15, y - 1, 3, 3, d.c.ink); });
    });
    d.line(85, 9, 85, 37, d.c.ink); d.arrow(85, 23, 94, 23, d.c.ink);
  } },
  'body-05': { alt: 'Food aroma travels from the mouth up an internal passage toward smell receptors in the nose.', draw(d) {
    d.circle(59, 22, 19, d.c.gold); d.rect(46, 28, 20, 17, d.c.gold);
    d.triangle(33, 17, 14, 12, d.c.gold); d.line(40, 32, 53, 32, d.c.ink);
    d.rect(43, 18, 9, 3, d.c.rose); d.rect(47, 12, 2, 2, d.c.ink);
    path(d, [[48, 33], [58, 33], [58, 24], [51, 24]], d.c.white);
    d.arrow(56, 32, 56, 24, d.c.teal); d.arrow(56, 24, 46, 22, d.c.teal);
    d.circle(19, 35, 7, d.c.orange); d.rect(18, 25, 2, 5, d.c.green); d.arrow(28, 33, 37, 33, d.c.teal);
    d.text('AROMA', 2, 3, d.c.ink);
  } },
  'body-06': { alt: 'Smaller lungs with a raised diaphragm contrast with expanded lungs and a lowered diaphragm during inhaling.', draw(d) {
    lungs(d, 23, 22, false); lungs(d, 72, 22, true);
    path(d, [[7, 38], [14, 33], [23, 31], [32, 33], [39, 38]], d.c.purple);
    path(d, [[55, 39], [64, 37], [72, 37], [81, 37], [90, 39]], d.c.purple);
    d.arrow(31, 13, 31, 4, d.c.teal); d.arrow(81, 4, 81, 14, d.c.teal); d.arrow(72, 40, 72, 46, d.c.purple);
    d.text('OUT', 1, 1, d.c.ink); d.text('IN', 55, 1, d.c.ink);
  } },
  'body-07': { alt: 'Oxygen crosses from an air sac into a blood vessel while carbon dioxide moves the opposite way.', draw(d) {
    d.circle(22, 24, 17, d.c.sky); d.circle(22, 24, 14, d.c.white); d.rect(19, 1, 6, 11, d.c.sky);
    d.rect(64, 4, 21, 40, d.c.rose); d.rect(67, 4, 15, 40, d.c.cream);
    [11, 24, 37].forEach(y => { d.circle(74, y, 4, d.c.rose); d.rect(73, y - 1, 3, 2, d.c.cream); });
    d.arrow(38, 17, 62, 17, d.c.teal); d.text('O2', 46, 7, d.c.teal);
    d.arrow(62, 30, 38, 30, d.c.purple); d.text('CO2', 44, 36, d.c.purple);
  } },
  'body-08': { alt: 'Two friendly bone cells illustrate removing old bone and adding new bone along the same living structure.', draw(d) {
    d.rect(18, 19, 60, 15, d.c.cream);
    [[18, 19], [18, 33], [78, 19], [78, 33]].forEach(([x, y]) => d.circle(x, y, 7, d.c.cream));
    d.line(24, 34, 72, 34, d.c.gold);
    d.circle(31, 10, 6, d.c.rose); d.rect(28, 9, 1, 1, d.c.ink); d.rect(33, 9, 1, 1, d.c.ink);
    d.rect(27, 17, 4, 4, d.c.paper); d.rect(34, 17, 4, 4, d.c.paper); d.rect(23, 10, 3, 3, d.c.gold);
    d.circle(66, 10, 6, d.c.teal); d.rect(63, 9, 1, 1, d.c.ink); d.rect(68, 9, 1, 1, d.c.ink);
    d.rect(59, 17, 5, 4, d.c.gold); d.rect(65, 17, 5, 4, d.c.gold); d.rect(71, 17, 5, 4, d.c.gold);
    d.text('REMOVE', 6, 42, d.c.ink); d.text('BUILD', 63, 42, d.c.ink);
  } },
};
