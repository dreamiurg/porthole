import assert from 'node:assert/strict';
import test from 'node:test';
import { stories } from '../src/stories.js';

test('every story offers two substantial routes in pages that fit the round reader', () => {
  assert.equal(stories.length, 7);
  assert.equal(new Set(stories.map(story => story.id)).size, stories.length);
  assert.deepEqual(stories.map(story => story.id), ['moon', 'library', 'dragon', 'seed', 'cloud', 'lighthouse', 'comet']);

  for (const story of stories) {
    assert.ok(story.pages.length >= 8 && story.pages.length <= 10, `${story.id}: opening page count`);
    assert.equal(story.choices.length, 2, `${story.id}: two routes`);
    assert.ok(typeof story.prompt === 'string' && story.prompt.length > 0 && story.prompt.length <= 100);
    assert.notEqual(story.choices[0].label, story.choices[1].label, `${story.id}: distinct choices`);
    assert.notDeepEqual(story.choices[0].ending, story.choices[1].ending, `${story.id}: distinct consequences`);

    for (const choice of story.choices) {
      assert.ok(typeof choice.label === 'string' && choice.label.length > 0 && choice.label.length <= 30);
      assert.ok(Array.isArray(choice.ending), `${story.id}: paginated ending`);
      assert.ok(choice.ending.length >= 3 && choice.ending.length <= 4, `${story.id}: ending page count`);
      const route = [...story.pages, ...choice.ending];
      for (const page of route) {
        assert.equal(typeof page, 'string');
        assert.ok(page.trim().length > 0 && page.length <= 190, `${story.id}: page is ${page.length} characters`);
      }
      const words = route.join(' ').trim().split(/\s+/).length;
      assert.ok(words >= 300, `${story.id}: route has only ${words} words`);
    }
  }
});
