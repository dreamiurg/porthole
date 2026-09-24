import test from 'node:test';
import assert from 'node:assert/strict';
import { readingPages } from '../src/reading.js';

test('pagination preserves words and punctuation and respects measured line limits', () => {
  const text = 'A curious dog followed the evidence. Then, something unexpected happened in the library.';
  const pages = readingPages(text, value => value.length, 20, 2);
  assert.deepEqual(pages, ['A curious dog followed the', 'evidence. Then, something unexpected', 'happened in the library.']);
  assert.equal(pages.join(' '), text);
  assert.deepEqual(readingPages('One short page.', value => value.length), ['One short page.']);
});
