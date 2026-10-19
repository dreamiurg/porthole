# Living Kingdom original PNG concept-art manifest

**Eight original PNGs, generated in the ChatGPT game-design conversation.** Each is 1536×1024, visually distinct, and should be committed as a binary PNG with its original image data. They are not interchangeable with the schematic SVG.

| Filename | Content |
|---|---|
| `01-world-map-and-encounters.png` | Illustrated map, troll encounter, party/inventory, village changes |
| `02-living-kingdom-overview.png` | Persistent-world infographic and quest systems |
| `03-three-visual-directions.png` | Minimal retro terminal direction with actual circular UI |
| `04-colorful-fifteen-screen-ux.png` | Colorful 15-screen UI exploration |
| `05-reading-and-choices-storyboard.png` | 12-screen longer prose / reread decision exploration |
| `06-illustrated-vs-retro-comparison.png` | Comparative 3-style visual exploration |
| `07-chapter-one-initial-flow.png` | 15-screen chapter-opening story/UX storyboard |
| `08-chapter-one-polished-ux.png` | Refined 15-screen Willowbrook story and decision storyboard |

## Status

All eight original PNGs are committed in this directory at their original
1536×1024 resolution. Each file is byte-for-byte identical to its ZIP entry;
none has been regenerated, recompressed, or replaced with SVG.

Source: [original concept-art ZIP on Google Drive](https://drive.google.com/file/d/1ZRhYuGQ8jxlkiqYv4sr6WItBClGclIUx/view).

Verified ZIP SHA-256:

```text
b67913e4383d76c4759755312c051ca82ff8ff800d4b6b9d215201aa2e78d295
```

## Import procedure

1. Download the complete ZIP from the Google Drive link above and verify its SHA-256.
2. In `dreamiurg/porthole`, check out `design/living-kingdom-chapter-one` (or the corresponding implementation branch).
3. Extract the contents of `concept-art/` into this directory, including the eight PNGs. Preserve the original PNG files without recompression or regeneration.
4. Commit the eight binary files and add previews to PR #79 or to the implementation PR. Do not merge.

The images are concept art only; don't ship the boards as game sprites.
