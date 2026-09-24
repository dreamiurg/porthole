# Biscuit visual direction

A lavender pocket toy on a quiet, dotted cream desk. Pixel artwork is the central attraction: a golden floppy-eared puppy in a warm reading nook. Book spines and a taped character note frame the emulator. On phones the device leads and books follow.

The user requested retro pixel graphics and delegated design decisions. The shell represents a pocket toy on a reading desk, with no physical buttons. Every game state and control stays inside the round display.

Palette: warm paper #fbf8f1, aubergine ink #453657, lavender shell #dbceeb, golden puppy, peach floor, sage and rose needs. Use chunky pixel forms for game art, bundled Montserrat for the physical screen and monospace for the browser surround. Screen colors use RGB565 channel levels, flat fills, and no decorative shadows. Runtime art is original Canvas pixels; a separately generated concept sheet explores the visual direction. There are no external font or asset requests at runtime.

Signature interaction: touch Biscuit to wag and receive a heart; investigate a branching mystery together to add a book to the shared shelf. Motion is small, slow, and disabled by reduced-motion preference. No urgency or failure state.

All game actions, library, stories, rewards, and fetch work in the round screen. Browser surround is optional. Keep readable text, keyboard-operable DOM controls, live status messages, a muted-by-default sound toggle, and touch targets within the display. Reading uses 24px text with measured six-line pagination. Pixel art updates at most ten times per second. Physical-size preview has a ruler calibration slider. These are conservative design constraints; the emulator does not measure firmware compatibility.
