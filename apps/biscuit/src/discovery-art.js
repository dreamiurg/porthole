import { sciencePictures } from './discovery-art-science.js';
import { culturePictures } from './discovery-art-culture.js';
import { lifePictures } from './discovery-art-life.js';

export const discoveryPictures = { ...sciencePictures, ...culturePictures, ...lifePictures };
const c = { ink: '#5a3d42', paper: '#fff7e6', cream: '#f7e3c5', blue: '#424d7b', sky: '#addbd6', teal: '#94bead', green: '#7b966b', gold: '#efba84', orange: '#deaa8c', rose: '#de6d8c', purple: '#b5a2ce', white: '#ffffff' };
const glyphs = {
  A:'010101111101101',B:'110101110101110',C:'011100100100011',D:'110101101101110',E:'111100110100111',F:'111100110100100',G:'011100101101011',H:'101101111101101',I:'111010010010111',J:'001001001101010',K:'101101110101101',L:'100100100100111',M:'101111111101101',N:'101111111111101',O:'010101101101010',P:'110101110100100',Q:'010101101111011',R:'110101110101101',S:'011100010001110',T:'111010010010010',U:'101101101101111',V:'101101101101010',W:'101101111111101',X:'101101010101101',Y:'101101010010010',Z:'111001010100111',
  0:'111101101101111',1:'010110010010111',2:'110001010100111',3:'110001010001110',4:'101101111001001',5:'111100110001110',6:'011100111101111',7:'111001010010010',8:'111101111101111',9:'111101111001110','+':'000010111010000','-':'000000111000000','=':'000111000111000',':':'000010000010000','?':'110001010000010','!':'010010010000010','.':'000000000000010','/':'001001010100100',
};

// Small, static RGB565 scenes: 96×48 pixels, enlarged exactly 3× in the reader.
export function renderDiscoveryArt(ctx, id) {
  const picture = discoveryPictures[id];
  if (!picture) return;
  const rect = (x, y, w, h, color) => {
    const left = Math.max(0, Math.round(x)), top = Math.max(0, Math.round(y));
    const right = Math.min(96, Math.round(x + w)), bottom = Math.min(48, Math.round(y + h));
    if (right <= left || bottom <= top) return;
    ctx.fillStyle = color;
    ctx.fillRect(left, top, right - left, bottom - top);
  };
  // biome-ignore lint/complexity/noExcessiveCognitiveComplexity: Bresenham line algorithm, clearer as one loop
  const line = (x1, y1, x2, y2, color) => {
    x1 = Math.round(x1); y1 = Math.round(y1); x2 = Math.round(x2); y2 = Math.round(y2);
    const dx = Math.abs(x2 - x1), dy = -Math.abs(y2 - y1), sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
    let error = dx + dy;
    while (true) {
      rect(x1, y1, 1, 1, color);
      if (x1 === x2 && y1 === y2) break;
      const twice = error * 2;
      if (twice >= dy) { error += dy; x1 += sx; }
      if (twice <= dx) { error += dx; y1 += sy; }
    }
  };
  const d = { c, rect, line,
    circle(x, y, radius, color) {
      for (let row = -radius; row <= radius; row++) {
        const half = Math.floor(Math.sqrt(radius * radius - row * row));
        rect(x - half, y + row, half * 2 + 1, 1, color);
      }
    },
    triangle(x, y, w, h, color) {
      for (let row = 0; row < h; row++) {
        const width = Math.max(1, Math.round(w * (row + 1) / h));
        rect(x + Math.floor((w - width) / 2), y + row, width, 1, color);
      }
    },
    arrow(x1, y1, x2, y2, color) {
      line(x1, y1, x2, y2, color);
      const angle = Math.atan2(y2 - y1, x2 - x1);
      for (const turn of [-0.6, 0.6]) line(x2, y2, x2 - 4 * Math.cos(angle + turn), y2 - 4 * Math.sin(angle + turn), color);
    },
    text(text, x, y, color) {
      [...String(text).toUpperCase()].forEach((letter, index) => {
        [...(glyphs[letter] ?? '')].forEach((pixel, n) => { if (pixel === '1') rect(x + index * 4 + n % 3, y + Math.floor(n / 3), 1, 1, color); });
      });
    },
  };
  rect(0, 0, 96, 48, c.paper);
  picture.draw(d);
}
