# Independent marsh route and art review — archived pre-visual evidence

**Archived functional playthrough — before the visual revision.** The user
rejected the art and typography captured here. This report preserves the actual
inputs, gameplay findings, build identities, and replay results of that earlier
presentation. Its screenshots and readability judgments do not certify the
replacement based on the original nine-screen SVG, GPT-generated art, and
Atkinson typography. Revised-build visual playthroughs will be added separately.

The marsh-protecting route reached its natural Chapter One ending, recognized
one deed, and returned to the map with optional bridge work unfinished. The
player then saved, restarted, continued on the same day with the same deed,
visited the changed marsh and inspected the changed mill before exiting.

This was a source-free emulator playthrough for narrative and gameplay. The
reviewer had authored the art and read its visual brief, but did not inspect
story data, engine code, tests, hidden state or source-derived solutions.
Every decision came from rendered prose and visible choices. Native frames
were inspected throughout. No physical hardware was tested.

## Build and reproduction

Archived pre-visual emulator SHA-256:
`f5a1d7b8c4acbcd015c632ed0de9a9dcd2a49b5fdea800cd575f3e745336246d`.

The observed run began on `72bc418b...`, restarted into the prose-corrected
`85451edd...`, and restarted into the quiet-caption build `c3cd4b82...` for the
post-ending visit. The entire trace was then replayed from fresh campaign state
on that build. **217 of 220 captures were pixel-identical**. The only differences
were the three previously blank quiet-visit captions, now explicitly labeled
“A quiet visit.” All three changed archived frames were inspected at native size.
See [`replay-caption.json`](replay-caption.json) for that comparison. A final
internal screen-table naming fix makes the existing test runner discover the
Kingdom modes, without changing returned names or behavior. Replaying once more
on the archived pre-visual binary reproduced **220/220 post-caption frames identically**; see
[`replay.json`](replay.json). All selected screenshots match that archived output.

- [Complete visible-input trace](input.txt)
- [Archived-build replay metadata](replay.json)

This historical reproduction requires the recorded pre-visual
`build/host/snap`; a revised presentation can change tap coordinates and frames.
From `apps/porthole/`:

```sh
python3 docs/living-kingdom/blackbox/kingdom_replay.py \
  docs/living-kingdom/blackbox/art-review/input.txt \
  build/kingdom-marsh-reproduction
```

Setup was a fresh isolated directory, `build/kingdom-art-playtest`. The only
fixture was the shell parent policy: `build/host/sys-limits.sav` contained the
little-endian unsigned 16-bit pair `(20, 120)` for a 20-minute session and
120-minute daily allowance. There was no profile or story-progress fixture.
The Ada profile, face, age, skipped PIN and launcher navigation were entered
through visible controls. Input was limited to touch, ordinary elapsed-time
steps, screenshots and process restarts.

## Route and interruptions

1. Asked the village about the trouble, listened to Pippa and recorded her safe
   paths. Visited Fen, heard the cost to the nests and promised to return.
2. Inspected the mill with Lucas, copied Elric's corrected accounts, and sought
   substitute food instead of promising nonexistent flour. Examined the disabled
   trader option's explanation, then earned a flour parcel through carrying work.
3. Posted the rumor correction, inspected the bridge from safe ground with Lucas,
   and recorded a repair plan. The optional channel investigation remained open.
4. Read the full council discussion and selected marsh flow. From the confirmation
   screen, reread from page one, moved forward and backward, traversed the entire
   passage again, and returned to the unchanged choices before confirming once.
5. Traveled through the intervening world days. The mill's changed vignette and
   “A quiet oven” encounter showed the reduced milling capacity. Exchanged the
   previously earned flour parcel and spent a day helping with food relief.
6. Read the deed recognition and complete “The longer lunch table” epilogue.
   The ending acknowledged fewer bakery hours alongside protected nests, rather
   than presenting a cost-free resolution. Day 10 showed one deed; the later
   marsh revisit reached day 11 and retained it.

The first manual interruption occurred at “A repair that held,” page 3/6.
After exiting and restarting through the profile picker and launcher,
Continue restored that exact page: [resumed page](065-resumed3.png).

The shell later interrupted at the last page of “The price of waiting”:
[rest screen](087-trader-choice.png). After a process restart to load the
prose-corrected build, the same page returned before any trader choice:
[resumed page](090-trader-resume.png). The simulator restart also removed the
visible rest gate; no claim is made that ten minutes of rest elapsed. This is
an emulator-clock/restart limitation, not physical-device rest verification.

The trace has four process segments, with approximately 13.7, 6.6, 26.4 and
0.1 minutes of requested elapsed-time stepping. These are recorded emulator
inputs, not a claim about physical play duration. The final save/quit and
restart restored [day 10 and one deed](215-final-resumed-map.png).

## Findings and evidence

No gameplay blocker, clipped prose, unreadable choice, lost reading cursor or
unexpected commitment was observed. The main follow-through was discoverable
through the mill preview after the two-day interval. Disabled choices explained
why they were unavailable: [trader explanation](092-why-help.png).

The blank activity label on locations with no new encounter was a minor clarity
issue. The archived build fixed it with [“A quiet visit”](166-village-preview.png),
without changing navigation or progression. Go and Back were exercised again
on that archived build during the post-ending visit.

The humans initially shared much of their facial silhouette. Before this route,
Lucas's art was refined with short swept hair, a visible ear and a quieter
sideways expression. Princess retains her long bob, crown, armor and direct
smile. Dribble is immediately distinct. That earlier art was checked in the native renderer, including outcome-specific
marsh and mill changes. The user subsequently rejected that treatment; this
paragraph records its review history, not approval of the replacement assets.

| Evidence | What it demonstrates |
| --- | --- |
| [Title](009-title.png) | Archived landscape and title composition |
| [Story](010-story1.png) | Stable page navigation and roomy prose |
| [Council choices](137-council-choice.png) | Three readable decisions with understandable costs |
| [Confirmation](138-marsh-confirm.png) | Consequential action separated from reading |
| [Reread](139-council-reread1.png), [returned choices](149-reread-choice.png) | Reversible reading before commitment |
| [Changed mill](170-mill-follow-preview.png) | Visible low-water consequence |
| [Flour exchange](187-flour-used.png), [empty pocket](211-final-items.png) | Earned item used for practical follow-through |
| [Deed recognized](197-marsh-ending-result.png) | Outcome-specific work earns the deed |
| [Ending](204-ending6.png), [completed map](209-completed-map.png) | Full conclusion and continuing world |
| [Protected marsh](216-final-marsh-preview.png), [later mill](218-final-mill-preview.png) | Distinct persistent location changes |
| [Post-ending map](217-post-ending-marsh.png), [exit](220-final-exit.png) | Exploration and exit remain available |

## Historical physical-size reading approximation

[Reading comparison](physical-reading.png) pairs archived native 480px screenshots
with 201px versions. At 96 display pixels per inch, 201px approximates a
2.1-inch diameter. Browser scaling and monitor density vary: this is an
emulator visual check, not physical-device testing.

The tester reported readable quotation marks, apostrophes, open counters and
descenders at the smaller size in that build. The old foreground-to-parchment
contrast measured 10.91:1, and the condensed body had an approximately 7.5px
x-height at the 201px comparison size. These measurements apply only to the
rejected font and layout. The replacement Atkinson typography requires a new
reading check; physical-device readability remains unverified.

The optional tower/woods route, channel construction and alternative endings
were outside this route. The flour-rumor correction was encountered and read on
the corrected build; the other two corrected optional story titles were not
encountered. The archived-build replay verifies that these omissions did not alter any
observed frame or input outcome in this route.
