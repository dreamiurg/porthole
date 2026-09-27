# Porthole promo video: creative brief

Status: draft for the author. Not a storyboard, script or shot list; those come after the open
questions at the end are answered.

Everything the video shows must exist in the games today. Every mechanic, screen and string below
was checked against `apps/porthole/` (`games/pets-club/pet.cpp`, `game.cpp`, `games/biscuit/pet.h`,
`screens_*.cpp`, `shell/shell.cpp`, `host/sim.cpp`, `tests/playtests/`). Where a fact is an
assumption, it says so.

## 1. Audience

One master cut, four posts. The footage is the same for everyone; what changes per subreddit is
the title, the lead still, and the first paragraph of the post. Four separate videos would cost
four times the render work to say the same thing, and the research says the title and lead media
do the work anyway.

| Audience | Where | What they need to see in the first 5 s | What kills it |
| --- | --- | --- | --- |
| Makers, ESP32 people | r/esp32, r/maker | A physical round gadget, then proof it's real firmware: board name, price, "C++, no libraries", MIT | A commercial. No code, no board. |
| Virtual-pet fans | r/tamagotchi | A pup with a face and a name, growing over real days, a gift parcel, a party hat | "Nothing dies" read as "no stakes"; three growth stages read as thin next to a real Tamagotchi |
| Claude builders | r/ClaudeAI, r/ClaudeCode, r/SideProject | The finished thing, working; disclosure up front and specific | Vague "built with AI"; anything that looks like a demo of a prompt rather than a product |
| Parents | r/daddit | A kid reading to a dog; a device that says "Back tomorrow!" on its own | Looks like another screen; kids' faces; hype |

The master cut carries one beat per audience (hardware, growth, reading, the daily cap). Each
post's title points at its own beat.

## 2. The message

A puppy on a $40 round screen that grows over real days, loves being read to, and can never die:
free firmware you flash from a browser.

## 3. Tone and look

- Tone: a kid's Saturday morning. Calm, plain, a little funny. No "epic", no "revolutionary", no
  exclamation marks in captions except where the game itself uses one.
- Palette comes from the games, not from us. Pets Club's 32-color PICO-8-plus palette and Biscuit's
  cream-and-plum full-color look are the video's colors. The frame background is a warm paper
  off-white (sample Biscuit's page color); it makes both games look like they belong together and
  matches a table-top phone shot later. No dark "tech" background: that reads r/esp32 only.
- The bezel: a flat charcoal ring around the 480 px circle, matte, one soft inner shadow, nothing
  else. No reflections, no fake phone, no rendered PCB. If the author's real case has a color, match
  it (open question 1). The ring exists so feed viewers read "gadget", not "app".
- Pixel art: nearest-neighbor at integer scale only. Pets Club is 160 logical px x3 = 480; in a
  1080 frame render the screen at 960 (exactly 2x). Biscuit's 480 native goes 2x the same way; its
  anti-aliased text survives integer nearest scaling. Never 1.5x, never bilinear.
- Frame rate: the sim steps 40 ms frames (25 fps). Render at 25 fps, or duplicate to 50. Not 30:
  a 25-to-30 pulldown judders on every scroll and sprite step.
- Type: captions in Montserrat (already bundled with Biscuit under the OFL), regular weight, one
  line, below the circle. Captions are not part of the screen: they never overlap the bezel. No
  pixel font for captions; the game's 8x8 font is the game's voice, the captions are ours.
- Pacing: 2-4 s shots in the first half, 1-1.5 s in the second, cut on the beat. One screen per
  shot; no split screens, no picture-in-picture. Transitions are hard cuts except one: the growth
  montage, which cuts on each day change.
- No sound from the game. The buzzer is harsh and the muted-autoplay assumption makes it moot.

## 4. Hook: three options for the first 2 seconds

A. "Knock knock. A package for you." The adoption screen, then the puppy pops out (`SC_INTRO`:
   "Knock knock!", "A package for you.", "A puppy!"). Real gameplay from frame one, and the puppy's
   face fills the circle at the moment a feed viewer decides whether to keep watching.

B. A week of a puppy's life in 8 seconds. Day 0 puppy, day 4 "Not a puppy anymore!", day 7 party
   hat, day 10 "All grown up." Only a simulator can shoot this. Evaluation below.

C. "You read. The pup listens." A story page in big pixel text, the dog sitting under it. The one
   thing no Tamagotchi clone has. But two seconds of text on a muted feed is two seconds of nothing.

Pick A, with B as beat 2. Reasons: A puts a face and a physical gadget on screen before the viewer
has finished scrolling; it needs no caption to be understood; it is what a kid actually sees first.
B is a strong idea but a weak first frame: three sprite changes only land after the viewer knows
the pup was a baby. Right after A, though, B is the best beat in the video.

On idea B, accept with fixes:
- It is real. `stageFor()` derives the stage from `adoptedAt` and the clock: day 4 = dog, day 10 =
  grown; the day-7 gift is the party (a hat, a sticker). A script of seven to ten `skip 86400` lines
  reproduces a real week exactly, and `--now` pins the clock so every run is identical.
- Skip one day at a time, not one big jump. A single 10-day skip fires one stage-up celebrate and
  skips the day-4 one; the gift parcel is one per day; the streak needs consecutive days.
- Each skipped day decays needs by up to 12 hours (`OFFLINE_CAP_SEC`). After a few days the pup sits
  at the floors with a "want" bubble. Either feed between skips (cleaner) or leave it and caption
  "Miss a week. It waits." (more honest, and the wellbeing message parents liked). Recommend the
  second: it turns a production wrinkle into the point.
- Growing is puppy, dog, grown dog: three sprites. Don't sell it as more. The party hat on day 7
  and the parcel on the window sill are what make the montage feel like days passing.
- Biscuit can't do this: his stage counts visits (`daysTogether`), not calendar days. Keep the
  montage Pets Club only.
- Don't call it a "time-lapse" in captions; call it days. "Day 4." "Day 7." "Day 10."

## 5. Beat sheet (target 36 s, range 30-40)

| # | Beat | Duration | What's on screen | Caption |
| --- | --- | --- | --- | --- |
| 1 | Adopt | 0-4 s | "Knock knock!" parcel, tap, "A puppy!", the name keyboard, a name typed | none, then "A puppy on a $40 round screen." |
| 2 | Real days | 4-12 s | Home; the window parcel appears; day counter advances; day 4 celebrate; day 7 party hat; day 10 "All grown up." | "It grows over real days." then "Miss a week. It waits." |
| 3 | Care | 12-18 s | Feed (bowl), pet (tap the dog), muddy paws to bath (rub the mud off), Fetch: slide to catch bones, "Ouch, a bee!" | "Feed. Play. Bath." (or none) |
| 4 | Read | 18-25 s | Bookshelf, library, a story page, the end-of-story question, a trick lesson "Learn: Sit, round 1 of 3, Watch..." | "It loves being read to." then "30 stories. Eight tricks." |
| 5 | Biscuit | 25-31 s | Biscuit home, speech bubble; a story's two-choice page; a discovery (the penguin), "I wonder..." and its Source | "Biscuit: stories with two endings. 96 things to find out." |
| 6 | Whose turn | 31-34 s | "Who's playing?" picker with four faces; the rest screen "Back tomorrow!" | "Four kids, one board. 25 minutes a day, then it says goodnight." |
| 7 | End card | 34-38 s | Still: the circle with the puppy, the repo URL, board name | "Free. MIT. Flash it from Chrome." and one small line: "Made by a dad and his kids, with Claude Code." |

Notes on the beats:
- Beat 2 is the GIF. Shoot it once at loop length and reuse.
- Beat 4 is the emotional center and gets the most room. Show the question after the story: it says
  "reading, not just tapping" without a caption.
- Beat 5 exists so Biscuit isn't a footnote, but the video is Pets Club's. Biscuit is full-color
  and calmer; three shots is enough to show the second game is a different mood.
- Beat 6 is the r/daddit beat and the one live footage should replace or extend (section 8).
- Every beat starts and ends on a Home screen or a picker so phone clips can be spliced between them.

Loop GIF (6-10 s): beat 2 alone, cropped to the circle plus a thin bezel, at 480x480. Day 0
puppy, parcel on the sill, day 4 dog with "grew up!" celebrate, day 7 party hat, day 10 grown, hold
one second, cut back to the puppy. The cut back is the loop; no fade. Day captions inside the GIF as
a small counter below the circle ("Day 1" ... "Day 10"), nothing else. This is the lead media for
r/tamagotchi and the pinned comment on r/daddit; r/esp32 gets a still (section 9).

## 6. Caption voice

Plain present tense, the way a parent describes it to another parent. Short enough to read in
one glance at feed size. No hype adjectives, no "seamless", "delightful", "unleash", "journey",
"magical". No "AI-powered" anything. Nothing a seven-year-old couldn't read. Numbers are fine when
they are facts.

Examples:
- "A puppy on a $40 round screen."
- "It grows over real days."
- "Miss a week. It waits."
- "You read. The pup listens."
- "Four kids, one board. 25 minutes a day, then it says goodnight."
- "Free. Flash it from your browser."

Don't caption what the screen already says. If the game shows "A puppy!", the caption stays empty.

## 7. Music

Target: 100-120 BPM so a beat is 0.5-0.6 s and a two-beat cut is 1-1.2 s (the late-video shot
length). Warm, light, small: ukulele, glockenspiel, soft synth or a gentle chiptune lead. Not
orchestral, not "corporate upbeat", not a 90s Tamagotchi-ad jingle. Should sound fine at low volume
and mean nothing if muted; the cut must work without it, which is why captions carry the message.

Source: Kevin MacLeod's catalogue at incompetech.com is verified CC BY 4.0. Required credit,
in the post body and the video description, exactly: `"<Title>" Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 4.0 https://creativecommons.org/licenses/by/4.0/`.
Individual track pages there render by script, so I could not verify specific titles or tempos
from here; the sound role picks the track from the catalogue's "Bright"/"Calm" feel filters against
the target above and confirms BPM by ear. FreePD (MacLeod's CC0 site) is closed; don't plan on it.
No AI-generated music, no AI voice, per constraints. Music first, then cut to it.

## 8. Phone footage and the seam

The author's real-device clips are the "photo" that made the Pokeball post work. Plan for them now;
the rendered cut must also stand alone. Hard rule: the kids never appear, not even their hands. Only
the author's own hands (and, if he chooses, his face).

Reserved slots, in priority order:
1. Before beat 1, 1.5-2 s: the gadget on a table, a thumb taps it awake. Optional. If used, it
   becomes the first frame, and the case decides the bezel color.
2. Extending beat 6, 3 s: the author's hand sets the puck down on a shelf or charging spot at
   "Back tomorrow!". No second hand: nobody else is ever in frame.
3. Inside beat 4, 2 s: a hand turning a story page. Only if the shot is clean; text on a phone
   camera at feed size is usually mush.

Seam rules so the two sources cut together:
- The rendered bezel matches the real case: color, ring width relative to the screen. Shoot the
  phone clips first if possible; otherwise leave the bezel color as a single variable in the
  composition.
- Captions run across cuts in the same font and position; the caption is what tells the viewer it
  is the same video.
- Same time of day on screen: the sim clock defaults to 16:00 (day tint). Shoot phone clips in the
  same in-game daytime; never cut from a rendered day room to a phone-shot night room.
- Music does not change at the seam; the phone clips are silent.
- Phone clips at the rendered frame's scale: the screen's circle should occupy roughly the same
  fraction of the frame in both. Crop the phone clip, don't scale the render.
- No kids on camera at all (faces or hands), no real names on screen. Profile names are invented.

## 9. Deliverables

| File | Spec | For |
| --- | --- | --- |
| Master | 1:1, 1080x1080, 25 fps, H.264 MP4, captions burned in, 30-40 s | Reddit native video everywhere; GitHub README link |
| Vertical | 9:16, 1080x1920, same timeline, screen larger, captions below | Anyone who reposts to a phone feed; cheap to derive, do it last |
| Loop GIF | 1:1, 480x480 (from the 960 render, 2:1 down, nearest), 6-10 s, plus the same as a silent looping MP4 | r/tamagotchi lead media, r/daddit comment |
| Hero still | 1:1, 1080x1080, the day-7 party frame with bezel, no caption | r/esp32 image post, OpenGraph image |
| Credits text | Music attribution line, board name, license, repo URL | Pasted into every post |

Verify Reddit's current size limits for video and GIF before export; I have not checked them and
they change.

## 10. Risks: how it fails per channel

r/esp32
- Reads as an ad. Fix: the post body opens with the board, the price, and
  three lines of engineering (C++ with no libraries, a 160x160 indexed framebuffer scaled 3x plus a
  native 480x480 RGB565 surface, scripted playtests that audit every screen). The video's end card
  names the board.
- The "AI Content" flair is mandatory; use it. State what the human decided (the games, the rules,
  the no-death contract, the art direction) and what Claude wrote (most of the code) in the first
  paragraph, and credit MatixYo's Plane Radar as the reason this exists.

r/tamagotchi
- "Nothing dies" read as no stakes. Fix: title on the real-days clock and the party, not on the
  absence of death; the wellbeing line goes in the body, framed as "for a 6-year-old".
- Three stages looks thin. Don't compare to Tamagotchi in the title. "Virtual pet" or "pixel pup",
  and let the GIF do the talking.
- They want to hold it. Lead with the still of the case if one exists; otherwise the GIF.

r/daddit
- Looks like another screen for kids. Fix: the daily cap and "Back tomorrow!" must be in the
  video, not just the post; the title is about reading to a dog, not about ESP32 anything.
- Any kid on camera (face or hands) or a real name is a hard failure. Invented profile names only.
- The AI line lands either way here; keep it to one sentence and make it about the kids asking for
  changes and watching them appear.

r/ClaudeAI, r/ClaudeCode, r/SideProject
- Lowest risk. Failure is a video that looks like a demo. The master cut works as-is; the post is
  the story of building it with the kids.

Cross-cutting
- Any claim that isn't in the game. Numbers in this brief are from the code today: 30 stories in
  three levels, eight tricks in three lessons, 96 discoveries in 12 topics, seven Biscuit stories,
  six Biscuit tricks, four profiles, 25 minutes a day, dog at day 4, grown at day 10, party on day 7.
  Re-check before publishing if content lands in the meantime.
- Music attribution missing. Make the credits text a deliverable, not an afterthought.

## 11. Open questions for the author

These change the brief; everything else is a later role's call.

1. Is there a real case? The README says "3D-printed case" but the repo has no case files. If a
   case exists, its color sets the bezel and it becomes the lead still for r/esp32 and r/tamagotchi.
   If not, the bezel is a plain charcoal ring and the phone clips show the bare board, which changes
   beat 6's plan and the r/tamagotchi lead media (GIF instead of still).
2. Pup and kid names for the shoot. The playtests use kid "Sam" and pup "Biscuit", which collides
   with the Biscuit game. Propose kid "Sam", Pets Club pup "Pip", Biscuit pup left as "Biscuit".
   Confirm these are not your kids' names.
3. Does "Made with Claude Code" go in the video (one line on the end card, as planned) or only in
   the post text? My recommendation is both; the end card line is small and the post paragraph is
   specific.
4. Will you shoot the phone clips before or after the render? Before is better (bezel match).
