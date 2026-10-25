# Original raster concept art

**The actual PNG source images have been preserved without resizing or alteration** in one binary ZIP archive on Google Drive (owner-access):

[Download original concept-art ZIP](https://drive.google.com/file/d/19LMZvQIAtuDZLikyJDwhP6wl3t7hC-3O/view)

Archive: `toy-town-life-original-design-images.zip`
Size: **14,251,694 bytes**, SHA-256 `2eba78f075f1d998a396d05b5f5a2139608466b2a3486d823d552b4aff89ea47`
**Access:** currently private to the user's connected Drive account. This URL **is not an anonymous/public download link**. Codex must authenticate using an authorized Drive connection or the user must explicitly change sharing permissions before using a bare `curl`/gdown download. If Codex cannot access it directly, attach the ZIP to its task.

## Contents: original images (all 1448 × 1086 PNG)

| Archive path under `assets/reference/` | Purpose | SHA-256 |
| --- | --- | --- |
| `toy_town_life_six_worlds.png` | Six radically different art directions; **#4 Anime Slice-of-Life is approved** | `4f1a8402a9c823b4aa3a5ccd2a19a6627b52f8c0da72acf3d785bdfe336b2147` |
| `doll_neighborhood_game_flow_board.png` | Initial doll-neighborhood options and character customization | `d8199c45289b57d4a7b25e8946e4a8dd9b7930c43b71a88802ff158089b91879` |
| `my_little_world_toy_life_adventure.png` | Original child's toy-town location/activity concepts | `ae40d8999fa01d8135bc339a8dac116cd542fce361772348f5b9191554f8bc2b` |
| `toy_town_life_cozy_game_plan.png` | Educational main loop, outing scenarios and anti-compulsion review | `ca353e502eeafd1eab630da21ee3ad83deb59bc38d91a0b3c0aead634cd6e912` |
| `toy_town_life_storyboard.png` | **Selected 10-screen scenario storyboard** from Home to School, Cafe, Park and Journal | `dcb9a61d6926f6884d257296d5aa0908444b83cffae38e19197610c7929d59d2` |

## Import to the design or implementation branch

Extract originals to `docs/superpowers/specs/toy-town-life/assets/reference/`. Keep the 5 PNG files **binary-identical** and commit them as real images. No need to commit the ZIP once the five PNGs have landed. Update the pull request Markdown to embed the two selected boards, with the other three accessible under an expandable reference section.

The implementation retrieved the archive through the authenticated Drive connection. All five PNGs are now present in `reference/`, byte-identical to the SHA-256 values above. Runtime artwork is authored separately under `apps/porthole/games/toy-town-life/art/`.

The moodboards are not gameplay sprite sheets and their tiny baked-in captions are not appropriate for 480×480 device screens. The production agent should create smaller original sprites/backgrounds with readable native Porthole fonts; preserve each board as visual reference and provenance.
