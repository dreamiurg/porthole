# Toy Town visual choices: independent black-box playtest

Date: 2026-10-25 (Pacific). Result: **PASS for the changed customization/title/home flow**.

## Build and method

- Branch: `feat/toy-town-visual-choices`; base revision `bc4c63ad6df4795d90df7c2f6a128239d00e5bc6`, with the final working-tree implementation under review.
- Tested `apps/porthole/build/host/snap` SHA-256: `fba012907cf41968d87ef8c4f99e6c5b673a1236e8e3e8554babc4353d4c20f9`. Hash checked before and after the final pass; the loaded artifact and final artifact match.
- Emulator only, native 480×480 frames, `snap --serve` behind the existing web emulator on port 8767. Independent playtester did not read gameplay source, test solutions, debug state, UI hit-region queries, or existing solution traces to choose actions. Only visible frames and ordinary touch events drove play. No physical board was available.
- Fresh final session in `/tmp/toy-visual-live`, with its own empty `build/host` save directory. Created profile A, face 1, age 8, skipped PIN through visible controls. No game/progress fixture and no shortcut entry. Entered Toy Town from the visible fourth launcher page.
- Final observed input log: [input.jsonl](input.jsonl). Coordinates are logical 160×160; the HTTP touch adapter multiplies by three. Each ordinary tap allowed approximately 600 ms of inspected frames before and after, used 80 ms down/up, and allowed ordinary elapsed game time. Screenshot entries identify the exact observed frame. This is the recorded manual session, not a claim of a separately verified automated trace replay.
- `f00`–`f55` PNGs are the final-build replay evidence. Unprefixed PNGs and [first-pass-input.jsonl](first-pass-input.jsonl) preserve the superseded exploratory pass and must not be used as final acceptance evidence. Initial exploratory setup lacked the save output directory; persistence conclusions use only the fresh final session.

## Flow covered

1. Visible profile creation and launcher paging; title entry, Meet my doll, all four pictured doll materials, Dress up.
2. All nine wardrobe categories and all 32 visible options, including their initially selected defaults: Hair (Loose/Buns/Pony), Skin (As is/Warm/Tan/Deep), Fabric (Plain/Lines/Plaid), Shoes (As is/Rose/Dark), Accessory (None/Specs/Petal), Color (As is/Teal/Lilac/Gold), Name (Emi/Pip/Kai/Lulu), Material (Doll/Plush/Clay/Mini), Hair color (As is/Dark/Gold/Ink). Inspected the selected card, check, thumbnail, and full doll preview after each changed choice.
3. Captured a held press before release; selected fill appears under the finger and the check appears when selected. Dragging from Buns to the central preview cancels the choice. Tapping the decorative central doll does not change the selection. Right category navigation loops Hair color back to Hair; the left Dye tile communicates the reverse direction.
4. Back opened the visible pause page. Save & leave returned to the launcher; reopening showed the title with the same customized Mini doll. Enter Toy Town resumed Hair 1/9 with Pony selected and the same dark skin, gold plaid outfit, dark shoes, petal accessory and ink hair. Done returned to the doll chooser with Mini selected; pause → Town returned to the normal home hub.
5. Walk into town → School destination → Pack my bag. Packed the notebook, observed Bag: 1 of 3, then used the ordinary pause/Town route back to the resting home. Outing mechanics were unchanged and were not replayed in this narrow presentation pass.

## Native evidence

| Check | Frames |
| --- | --- |
| Clear primary title action and direct doll choices | [title](f03-title.png), [Doll](f04-doll-fashion.png), [Plush](f05-doll-plush.png), [Clay](f06-doll-clay.png), [Mini](f07-doll-mini.png) |
| Pictured hair choices and press/cancel feedback | [Loose](f08-hair-loose.png), [pressed](f09-hair-pressed.png), [Buns](f10-hair-buns.png), [Pony](f11-hair-pony.png), [cancelled drag](f12-hair-drag-cancel.png), [preview mis-tap](f13-hair-preview-mistap.png) |
| Swatches, fabric, shoes, accessories | [skin](f17-skin-deep.png), [plaid](f20-fabric-plaid.png), [shoes](f23-shoes-cocoa.png), [specs](f25-extras-glasses.png), [petal](f26-extras-flower.png), [color](f30-color-gold.png) |
| Name/material/dye and category loop | [name](f34-name-lulu.png), [Plush preview](f36-material-plush.png), [Mini preview](f38-material-mini.png), [dye](f42-dye-ink.png), [loop](f43-hair-loop.png) |
| Exit/reentry, completion, normal navigation | [pause](f44-back-from-style.png), [launcher exit](f45-style-exit-launcher.png), [title reentry](f46-style-reentry.png), [resumed wardrobe](f48-enter-resume.png), [doll chooser after Done](f49-home.png), [home hub](f51-town.png), [destination](f52-map.png), [packed notebook](f54-packed-notebook.png), [resting home](f55-resting-home.png) |

`f47-home.png` is an inert tap on the title after reentry, and `f49-home.png` is the doll chooser; filenames alone should not be interpreted as screen-state assertions.

## Findings and resolutions

- First pass: Done wrapped as Don/e, long choice labels escaped their cards, Hair dye escaped the side tile, and the bottom instruction wrapped against the round rim. Final replay confirms a wider one-line Done, short fitting captions, Dye, and one-line Tap to try. Material captions have space above the outline. No remaining clipped labels were observed.
- First pass: primary actions needed stronger emphasis. Final title Enter Toy Town, Dress up, and Done use mint fill; all interactive cards have strong outlines and raised lower edges. Green selected fill plus a check distinguishes the current choice. Clear spacing, arrows, pictures and short labels make the controls easy to identify against the detailed room art.
- An apparent merged Pip/Kai outline was withdrawn after independent inspection and numerical frame-pixel verification showed separate dark outlines and the intended gap in both initial and selected frames. It was a mistaken visual interpretation, not a runtime defect.
- Fashion-doll outfit colors remain concentrated in small trim/bow details, so color changes are subtler than on Mini. The preview matches the resulting doll. The root reviewer accepted this existing material-art limitation for this scope.
- No crashes, dead ends, accidental changes from the exercised mis-taps, or selection-loss on exit/reentry were observed. The final affected flow completed back at the normal resting home.

Automated CI, firmware compilation, save-format migration verification, and code review are separate implementation-owner gates; this report makes no claim to have rerun those checks. It certifies the observed source-free emulator UX pass only.
