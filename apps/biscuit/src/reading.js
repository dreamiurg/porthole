// Layout uses the same bundled font and logical pixels as the device preview.
export function readingPages(text, measure, width = 352, lineLimit = 6) {
  const pages = [];
  let page = [], line = '', lines = 1;
  for (const word of text.trim().split(/\s+/)) {
    const next = line ? `${line} ${word}` : word;
    if (line && measure(next) > width) {
      if (lines === lineLimit) { pages.push(page.join(' ')); page = []; lines = 1; }
      else lines++;
      line = word;
    } else line = next;
    page.push(word);
  }
  if (page.length) pages.push(page.join(' '));
  return pages;
}
