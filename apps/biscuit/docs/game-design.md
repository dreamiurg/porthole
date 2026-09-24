# A companion who grows with you

The central experience is a persistent dog, not a menu of unrelated mini-games. Biscuit should remember shared adventures, learn visible tricks, and gradually develop from a puppy into a grown story-loving companion.

## Research and decisions

- **Tamagotchi:** Bandai's [Connection manual](https://tamagotchi-official.com/manual/toy/connection/connection_web_manual_IS_EN.pdf) describes care-dependent growth through life stages and more games becoming available as the pet grows. The [Uni manual](https://tamagotchi-official.com/manual/toy/uni/Uni_WEB_IS_EN.pdf) adds individual preferences, familiar items, and care achievements. **Our adaptation:** three permanent growth stages, activity-based personality, and capabilities that unlock across active days.
- **Training as a relationship:** Nintendo's [Nintendogs + Cats manual](https://csassets.nintendo.com/noaext/image/private/t_KA_PDF/manual-3DS-nintendogs-cats-en) introduces bonding and teaching sit before further tricks. **Our adaptation:** short touch-pattern lessons, six distinct animated tricks, three practice steps to mastery, and mastered tricks available forever.
- **Choice and competence:** [Ryan, Rigby, and Przybylski (2006)](https://selfdeterminationtheory.org/SDT/documents/2006_RyanRigbyPrzybylski_MandE.pdf) found associations between autonomy, competence, and game enjoyment. This general game research is not proof of what any particular player will enjoy. **Our adaptation:** choose between caring, reading, playing, and training; give clear progress and gentle retries.
- **Child-centered play:** UNICEF's [RITEC research](https://www.unicef.org/innocenti/projects/responsible-innovation-technology-children) and [design toolbox](https://www.unicef.org/childrightsandbusiness/workstreams/responsible-technology/online-gaming/ritec-design-toolbox) include nurturing, creative choice, progression, and graceful recovery. **Our adaptation:** rotating daily invitations, permanent collectibles, no lost streaks, no pet death, and a warm return after time away.
- **A world that changes:** Nintendo describes passing time, seasonal changes, and daily activities in [Animal Crossing](https://www.nintendo.com/au/games/nintendo-switch/animal-crossing-new-horizons/). **Our adaptation:** local-day adventures, a small word discovery, new stories, and a scrapbook. Missed days never reset earned progress.

These are design inferences from the sources, not claims that these specific mechanics are scientifically validated for this game.

## Implemented loop

Each visit starts at home with the same recognizable dog. Feed, cuddle, read, play fetch, practice a trick, or take a nap. First-time daily activities grow friendship; repeating actions remains playable without endless reward farming. A rotating three-activity invitation earns a permanent sticker.

Growth uses **active days**, not consecutive days: no deadlines or streak loss. Day one offers sit and paw. Later visits unlock spin, bow, jump, and roll over, plus new stories. Puppy, young pup, and story dog stages have visible accessories. Bookworm, Playful, and Cuddlebug tendencies reflect play history without locking out activities.

All navigation and controls are on the round touchscreen. The shell represents a 3D-printed case with **no physical buttons**. Game art uses a 160 × 160 pixel grid scaled three times into a 480 × 480 circular display; menus use bundled Montserrat on the same screen, with 24px paginated prose and a conservative RGB565/10fps profile. See [display constraints](display-profile.md).

## Reading and discoveries

The audience is kids who love dogs and reading, roughly 7 to 11, and strong readers. The stories use competing explanations, uncertainty, and humorous dialogue. Biscuit’s voice stays cute and affectionate: warm companionship, curious questions, and small dog jokes. Reading ability does not call for a formal detective persona. Both choices are valid investigative approaches with distinct endings. Each complete route is over 400 words, shown in short pages measured against the actual bundled font. Training sequences grow from three cues to five or six; there is no timer.

The field notebook contains 96 original, source-backed discoveries: eight each in space, physics, Earth, nature, history, humanity, art, inventions, mathematics, animals, languages, and the human body. Every entry includes a pixel illustration or diagram, an explanation, an open-ended question, and its primary source. Pictures appear in browsing lists and open at three times their pixel size before the reading pages. Mathematical patterns and scientific relationships are drawn explicitly; historical objects and bodies are stylized illustrations, not scale reconstructions. Facts were checked against sources including NASA, USGS, museums, UNESCO, universities, and research institutions. The prompts are invitations to think, not scored claims of mastery.

Three deterministic suggestions span different topic families each local day. All 96 appear across 32 days; the full collection remains freely browsable at any time. Keeping an entry makes it available in the notebook and counts toward the existing daily reading activity. No extra currency is awarded for collecting cards rapidly. Definitions, facts, and stories are bundled locally; only opening an original source requires internet access.

The learning and return-visit benefits are design hypotheses. We have not measured retention, learning outcomes, or enjoyment. English is the interface language.

## Scope and open validation

The browser is a screen/game emulator. It does not emulate the ESP32 CPU or verify firmware performance. The exact Waveshare board variant remains unverified. The official [device docs](https://docs.waveshare.com/ESP32-S3-Touch-LCD-2.1) report single-touch input, 8 MB PSRAM, and 16 MB flash; the [FAQ](https://docs.waveshare.com/ESP32-S3-Touch-LCD-2.1/FAQ) specifies the current RGB565 display path.

Assumptions: English, local saves, a golden puppy named Biscuit, and interest in brief touch-pattern lessons. Enjoyment, reading difficulty, and long-term appeal need feedback from real players. Browser checks can prove flows and persistence, not long-term engagement. No account, analytics, advertisements, purchases, push notifications, or online chat are needed.
