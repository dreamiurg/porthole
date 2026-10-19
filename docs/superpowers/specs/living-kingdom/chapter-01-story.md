# Chapter One: Trouble at Willowbrook — playable story blueprint and sample manuscript

**Original prose** written for this project (not excerpted from the books). Princess Pulverizer, Lucas and Dribble are used as familiar fan-game characters; all new local NPCs and Willowbrook story events are original.

This document specifies **the opening manuscript, causal quest flow and endings**. The implementation agent should expand it into a fully playable 3–5 session chapter, while preserving choices and causal coherence. Sample paragraphs are launch-ready prose, not filler. Avoid replacing prose with summaries. IDs are stable content IDs; align with the structured-world reference and the game's persisted state.

## The cast

- **Princess Pulverizer / Serena:** brave, restless, competitive, impatient with authority, curious when challenged; wants to be a knight and occasionally mistakes boldness for wisdom.
- **Lucas:** thoughtful, observant, loyal; can be nervous, but fear rarely stops him from noticing that something isn't right. Offers tact and careful evidence.
- **Dribble:** kind-hearted dragon, passionate chef, clever and occasionally distractible; hungry when dramatic; recognizes scents, plants, cooking techniques and the small needs others miss.
- **Bram**, troll at Old Bridge: repairs the bridge at his own expense after floods, fears his little sister **Bea** will be hurt if it collapses; gruff but not cruel. Initially hides the damage because villagers once laughed at his warning.
- **Elric**, miller: exhausted, proud, responsible for flour used by several families; believes rerouting river water is urgently necessary. Doesn't know the marsh is suffering.
- **Mira**, riverside caretaker: coordinates emergency food distribution; wants a fair process, knows families' different needs.
- **Tilda**, carpenter: can build an auxiliary channel and reinforce the bridge with labor and scarce lumber; insists on fair payment for her apprentices.
- **Nell**, trader: owns the delivery wagon; keeping it waiting has a real cost; sometimes rumors outpace her evidence.
- **Fen**, marsh guardian: protects nesting creatures and their scarce freshwater; suspicious of the village.
- **Pippa**, village child: follows clues, knows hidden paths, tells stories with exaggerations and unexpected truth.

## Opening sequence: ~15–20 minutes before first natural stopping point

### LK_INTRO_01 — On the road (3–4 reading pages)

> Princess Pulverizer was halfway through an extremely heroic speech when the wheel of their little cart fell off.
>
> It rolled past a tree, bounced over a stone, and disappeared into a ditch with a noise like a knight falling down a staircase.
>
> "I meant to do that," she announced.
>
> Lucas looked at the cart. Then at the empty axle. "Of course. It was a very convincing demonstration."
>
> Dribble poked his head into the ditch. "I could roast the wheel," he offered. "That would make it easier to find in the dark."
>
> "We don't need to roast everything," said the princess.
>
> "Not *everything*," Dribble agreed. "Only the important things."
>
> Beyond the trees stood Willowbrook, a cluster of red-roofed houses around a silver river. Smoke from the bakery curled into the sky, but the market road was strangely empty.
>
> Lucas noticed something that Serena had missed: a row of wagons stopped along the far bank. Their drivers weren't moving. They were arguing.
>
> "Adventure!" Serena declared.
>
> "Or trouble," Lucas replied.
>
> Dribble sniffed. "Definitely bread."
>
> And together they followed the road toward the bridge.

Initial journal: "We reached Willowbrook; wagons are stuck across the river." Unlock Willowbrook, Old Bridge and Miller's Mill. Keep choices light: **Visit the mill / Investigate the bridge / Ask villagers** (all remain available afterward).

### LK_VILLAGE_01 — Welcome to Willowbrook (2–3 pages)

> Willowbrook had all the usual signs of a happy village: flowerboxes, a painted sign advertising apple tarts, and a fountain shaped like an enormous turnip.
>
> Yet the square was quiet. A merchant was counting empty sacks. Two bakers whispered beside their ovens. A little girl in a yellow coat had chalked the words DON'T BLAME BRAM on the cobblestones.
>
> "Bram?" Serena asked.
>
> The girl quickly rubbed out the B with her shoe.
>
> "Nobody," she said. "I mean everybody knows about him, but that doesn't mean I was talking about him."
>
> Lucas crouched beside the chalk. "Who is Bram?"
>
> "The troll under the bridge. The miller says he's stopping the wagons. The trader says he stole a stone. The mayor says the bridge is safe as long as nobody uses it, which doesn't seem very useful."
>
> Dribble gazed toward the bakery. "Does anyone say when lunch is?"
>
> The girl almost smiled. "You can ask at the mill. But if you're here to rescue somebody, you should talk to the people who are actually stuck."

Player can speak to Pippa, Elric, Nell or travel to Old Bridge. Unlock journal fact "Villagers disagree about the bridge."

### LK_MILLER_01 — The worried miller (3–4 pages)

> The miller stood beside a waterwheel that turned so slowly it looked embarrassed.
>
> "There you are," he said when he spotted Serena's sword. "A knight! We've been waiting for a knight."
>
> "Almost a knight," Serena corrected.
>
> "Close enough," said Elric. "We need a bridge and we need water, and preferably before the bread runs out."
>
> He led them past sacks marked for families in the village. Some were full, some half full, and several were empty.
>
> "I could reopen the old millrace," he explained. "More river water for the wheel. More flour. Enough to feed people until the trader's wagons get through. But the marsh folk say the water belongs to them. Water!" He threw his hands up. "It's not a pie. You can't carry it home and shut the door!"
>
> Dribble cleared his throat. "Actually, you can put water in a pie."
>
> Elric stared.
>
> "It's a terrible pie," Dribble added.
>
> Lucas knelt near the wheel. "Someone repaired this recently. The newer boards are holding. It's the current that's weak."
>
> Serena looked at Elric, then at the flour sacks, then at the river. She had come looking for something heroic. She had found a problem that looked as though it would refuse to be struck with a sword.

Options:
- Ask exactly who is short of flour (gain `clue.supply_ledger`, open Mira/Nell).
- Offer to help reopen millrace (record promise, **not** instant solution; opens water dilemma).
- Ask Lucas to inspect the wheel (if Insight/companion ability, gain `clue.recent_repairs`).
- Visit bridge first (leave, no promise).

### LK_BRIDGE_01 — The troll on the bridge (4 pages)

> The bridge was older than Willowbrook and considerably less polite.
>
> Three stones had broken away from its outer wall. Below them the water swirled around a crooked wooden brace. A plank with the words NO CROSSING hung from a rope.
>
> The bridge trembled.
>
> Then a great green head appeared beneath the arch.
>
> "Go away," said the troll.
>
> Serena drew herself up to full princess height. "I am Princess Pulverizer, future knight of the realm, and I have come to demand—"
>
> "Go away," repeated the troll, more quietly.
>
> "—an explanation," Serena finished.
>
> The troll looked tired. He had two scraped knuckles and a strip of wet cloth wrapped around one wrist.
>
> "People keep crossing," he said. "The stones loosen, and I fix them. Then the river rises, and the stones loosen again. One day someone will fall in, and it will be my fault because I live under the bridge."
>
> Dribble sniffed. "I smell boiled turnips."
>
> "My sister's supper," said the troll. "She lives below. She's too little to climb out if the river floods."
>
> Lucas looked at the cracked arch. "He may be telling the truth."
>
> "Of course I'm telling the truth," said the troll. "It's a great nuisance. Lies are much easier to remember."

Choice routes:
- **Challenge Bram to a contest**: he agrees only if Serena will also inspect the bridge; potential embarrassment, not combat.
- **Ask what he needs**: unlock Bea / support clue and relationship.
- **Inspect stones before promising**: reveal shifted foundation and old repair marks.
- **Return later**: no penalty, old bridge remains open for dialogue.

### LK_PIPPA_01 — The girl with the chalk (2 pages)

> Pippa handed Serena a piece of yellow chalk. "Write down everything people tell you. Then cross out the bits that can't all be true."
>
> "Excellent," said Serena. "An investigation."
>
> "You were going to call it an investigation anyway, weren't you?"
>
> "Obviously."
>
> Pippa frowned at the miller's house. "Everybody says Bram broke the bridge. But I watched him carry stones *toward* it, not away from it. Why would somebody steal a bridge one stone at a time and bring the stones back?"
>
> Lucas smiled. "That is a very good question."
>
> "I have six more," said Pippa.

Gain journal clue `clue.bram_repairs`. Pippa may reveal secret path to Watchtower after a promise to keep the path safe.

### LK_FOREST_01 — An unexpected witness (3 pages)

> The North Woods whispered in three different voices: leaves, water, and the very serious opinion of a squirrel.
>
> Serena found a broken wooden marker pointing toward the old watchtower. Dribble found wild thyme. Lucas found boot prints heading toward the marsh, beside smaller tracks that could belong to a child or a fox.
>
> Something shimmered in the creek. It was a copper washer, stamped with the same flower pattern as the miller's replacement waterwheel.
>
> "Evidence," said Lucas.
>
> "Or treasure," said Serena.
>
> "Or a very tiny plate," said Dribble.

Player may pocket washer (inventory), mark location in journal (clue without taking) or follow bootprints (opens tower/marsh). Later scene reuses the clue differently depending on choice.

### LK_MARSH_01 — Fen's side (3–4 pages)

> The marsh looked like a painting somebody had spilled too much green onto. Reeds bent in the wind, frogs argued about absolutely everything, and narrow islands floated in the water like sleeping turtles.
>
> Fen waited near a shallow pool where several spotted eggs lay beneath wet leaves.
>
> "The mill takes more and more of our water," the guardian said. "Last summer we could walk through these reeds with water up to our knees. Today I can see the bottoms of the pools."
>
> Serena looked at the eggs. "But the villagers need bread."
>
> "And our hatchlings need water."
>
> "Can't everyone have enough?"
>
> Fen did not answer immediately. Then the guardian said, "That is what I have been asking, too."
>
> Dribble, who knew a great deal about eggs, became unusually quiet.

Player may agree to protect nests (promise), ask about water history (clue), or admit uncertainty (Fen appreciates honesty). Never imply that knowledge alone solves resource scarcity.

### LK_TOWER_01 — A forgotten design (3 pages)

> At the watchtower Serena found maps too old to be useful—until Lucas noticed two river channels drawn in faded blue.
>
> One channel ran toward the mill; the other curled around the marsh. Between them stood a little wooden gate, marked with the words OPEN ONLY WITH CARE.
>
> Someone had written below: Even a good solution needs someone to look after it.
>
> "That seems suspiciously wise," Serena said.
>
> "Or suspiciously expensive," Lucas replied.
>
> Dribble examined the corner of the parchment. "And it has a recipe on the back."

Gain `clue.old_channel`, item `item.channel_map` (if space; else journal discovery). Opens Tilda infrastructure negotiation; no automatic optimal route.

### LK_MIRA_01 — Who goes hungry? (3 pages)

> Mira's ledger contained thirty-seven names and a great many crossed-out numbers.
>
> "We have twelve bags of flour," she said. "The bakery needs five to stay open, the sick families need three, the traders are waiting for four, and the festival promised two."
>
> Serena counted on her fingers. She ran out of fingers.
>
> "The festival could wait," Lucas suggested.
>
> "Perhaps," Mira replied. "But the traders will stop visiting if we send them away empty, and then we may be poorer next month. The bakery employs people who must eat, too."
>
> "And the families who are sick?" asked Serena.
>
> Mira put down her pen. "That is why I asked you here."

Allow an allocation decision with visible short-term consequences. It must not be disguised as a single correct answer. Choices can be revise-priorities, reserve some for emergency, honor trade obligation, ask to seek more supplies; the hard option has a real tradeoff and new follow-up.

### LK_COUNCIL_01 — When everyone is right about something (4 pages)

> They met at sunset beside the village fountain. Elric came with a sack of flour. Bram arrived with muddy tools. Fen carried a bundle of reeds still damp from the marsh.
>
> "The wheel must turn," Elric began.
>
> "The marsh must live," said Fen.
>
> "And the bridge must not kill somebody," said Bram.
>
> Serena had hoped that someone would say something obviously wicked. Nobody obliged.
>
> Lucas laid out the evidence: the bridge repairs, the water marks, the old watchtower map, and the list of families who depended on the flour.
>
> "We can choose," he said, "but whatever we choose, we should know what it will cost."
>
> Dribble was setting bowls of soup between the arguing grown-ups. "People think more clearly after supper," he observed. "Except when the soup is burned."
>
> "Is it burned?" asked Bram.
>
> "Not yet," said Dribble, watching the pot with concern.
>
> Serena looked from the mill to the marsh. She had rescued goats and chased bandits. This was harder. There was no enemy to defeat, and there might not be a choice that everyone would thank her for.

High-stakes decision: shown **after** full rereadable passage and companion advice. Options conditional:
1. Divert more to mill temporarily (flour now, marsh habitat worsens, Fen strained, needs restoration quest).
2. Protect marsh flow (nests safe, scarcity continues; village relief / supplies follow-up needed).
3. Time-share water (both lose some; delays work; tensions persist until repairs).
4. Build controlled second channel (available with map + Tilda + enough lumber/effort, takes several days, apprentices diverted from repairs and bridge remains unsafe longer). Not a free win; labor/resource costs and chance of storm complications.

### LK_RETURN_01 — The kingdom remembers (2–3 pages, triggered 2+ world days later)

Write variants based on council outcome. Example for time-share:

> When Serena returned, the mill wheel was turning again. Slowly, but steadily.
>
> Elric stood by the door with flour on his sleeves and a tired smile. "Enough to make bread today," he said. "Not enough to keep every promise."
>
> Down at the marsh, Fen had moved the eggs to a higher pool. The guardian acknowledged Serena with a short nod, but did not come closer.
>
> "It's better," Lucas said.
>
> Serena watched the river. "Somewhat better," she corrected.

Other variants:
- Mill-first: village bakery busy, marsh drying; Dribble worries about hatchlings, Fen needs help; possible repair story.
- Marsh-first: eggs protected, bakery short, one hungry family benefits if inventory can help; player must negotiate trade/offers.
- Channel-build: a bridge or school repair delayed because apprentices are busy; a storm tests the water gate; relationships depend on whether Serena kept her labor/payment promises.

### LK_FINALE — One good deed, no perfect ending (3+ pages)

Any chosen route can resolve Chapter One through follow-up action (deliver supplies, repair channel, support vulnerable families, restore passage, honor a difficult promise). A good deed is earned through **effort and responsibility**, not one canonical council answer.

Example closing:

> That evening, Willowbrook hung lanterns across the village square. The bridge was open—not as strong as anyone wished, but inspected, braced, and safe enough for the little wagons.
>
> Serena had learned a great deal about masonry, water, and how many adults could shout at each other before Dribble demanded supper.
>
> "Were we good knights?" Lucas asked.
>
> "We weren't knights at all," Serena said. "Not yet."
>
> Bram gave a small wave from the bank. Fen watched from the edge of the reeds. Elric had sent bread. None of them agreed about everything.
>
> Dribble held up the last loaf. "They agree about this!"
>
> Serena laughed. "We did one good thing. We also made several new problems."
>
> Lucas considered that. "Then I suppose we know where to begin tomorrow."

End with Chapter One complete and **a genuine new lead** (a strange royal seal found in the tower, weather or a messenger from the highlands), journal ledger updated, continue exploring chapter region as desired.


### LK_RIVER_01 — The waterline (2–3 pages)

> Serena followed the riverbank until the grass turned to pebbles. A line of pale stones lay several feet from the water, like a necklace that the river had forgotten to wear.
>
> "Isn't the river supposed to be over there?" Lucas asked.
>
> Dribble dipped one toe into the shallows and pulled it back. "It is certainly cold enough to be a river."
>
> Across the bank, a little wooden gate stood half open. It was marked with an old crest, nearly hidden beneath moss.
>
> "A gate for water," Lucas said. "Someone built this. If it moves the river one way, it must leave less water another way."
>
> Serena touched the crest. The wood was worn smooth by years of careful hands.
>
> She had been thinking of the mill and the bridge as separate troubles. Perhaps the river had other ideas.

Choices record \`clue.water_low\`, and optionally reveal the marsh as a destination. The player now knows something is changing but **does not yet know who is responsible**.

### LK_TILDA_01 — The price of a clever idea (3 pages)

> Tilda the carpenter read the old watchtower map twice, slowly. Then she put it on the table beside a heap of unfinished chairs.
>
> "Can we build it?" Serena demanded.
>
> "Of course," said Tilda. "With enough wood, tools, hands, time and a promise that you'll pay my apprentices."
>
> "That's a lot of enoughs."
>
> "Building things is mostly enoughs. Fighting a dragon is much simpler, if you're the sort of person who likes fighting dragons."
>
> Dribble looked offended. "Some of us are the sort of dragons who prefer making soup."
>
> Tilda pointed to the bridge on the map. "I can lend the apprentices to the river channel, but they won't also be repairing the bridge that week. Everyone will need to agree on which work comes first."
>
> Serena stared at the map. A moment ago it had looked like a brilliant solution. Now it looked like another question.

Talking through the plan reveals \`flag.tilda_consulted\`, unlocking a channel option at council—but the cost and promise must carry forward.

### LK_COUNCIL_AFTERMATH — What a decision feels like (2 pages)

> The meeting was finished. People pushed back their chairs and made their way into the evening.
>
> Serena stood beside the fountain, listening to the last of their footsteps.
>
> "Somebody ought to cheer," she said.
>
> "They might," Lucas replied, "once they see what happens."
>
> Dribble held out a bowl. "For now, I have soup."
>
> Serena took it and sat on the fountain's edge. She had decided. The decision was real. And tomorrow the water would keep flowing, whether or not anyone liked where it went.
>
> "Then we should come back," she said.
>
> Lucas nodded. "That sounds like what knights do."

Do not immediately resolve the chapter. Advance world days through play, travel, or resting and show a follow-up before epilogue.

### LK_FOLLOWUP — Responsibility after the choice (2–4 pages, authored variants)

> The village seemed different when Serena returned. It was not quieter. It was not happier in every corner. But people had begun to live with the choice they had made.
>
> "They haven't forgotten," Lucas said.
>
> "Neither have we," said Serena.
>
> There was still work to do: a delivery to make, a promise to keep, or a neighbor whose trouble had become worse while everyone was busy solving somebody else's.
>
> Dribble tucked a loaf of bread beneath one arm. "I vote we begin with the part that can be eaten."
>
> Serena laughed. "We begin with the part that needs us most."

Codex must author **distinct follow-up variants** for each water outcome, each requiring a nontrivial action and incurring the previously foreshadowed cost. The two illustrative \`resolution.*\` choices in the JSON fixture are placeholders for **different methods of following through**, not guaranteed successful magic buttons. Resolve Chapter One only after a genuine follow-up has been completed.

## Required branching / encounter matrix for Codex

All scene text above is seed manuscript. Write enough additional *complete narrative prose* to support ~3–5 sessions, with each major scene given 2–5 pages and roughly 150–350 words. Avoid filler, random fetch grinding or identical dialogue repeated.

| Stable scene family | Unlock / trigger | Effect, variation or follow-up |
|---|---|---|
| `village.first` | start | hub destinations |
| `pippa.chalk` | first village exploration | Bram clue, path |
| `miller.wheel` | mill visit | grain ledger, water problem |
| `trader.wagons` | village after clue | supply bargain, promise |
| `mira.ledger` | talk to caretaker | shortage choices |
| `bridge.bram_first` | bridge first visit | fracture evidence, Bram's side |
| `bridge.bea` | Bram trusts party | his sister and safety stakes |
| `bridge.repair` | wood + worker support | travel reopens with tradeoff |
| `forest.footprints` | North Woods | old route / washer |
| `forest.fox` | after any forest entry, not too often | rumor and small favor |
| `tower.map` | reach Watchtower | alternate channel evidence |
| `tower.seal` | later, conditional | Chapter Two hook |
| `marsh.fen_first` | reach marsh | threatened habitat |
| `marsh.egg_relocate` | marsh crisis | reversible mitigation / pledge |
| `tilda.work` | discover channel | costs and apprenticeship obligations |
| `dribble.recipe` | food shortages + Dribble help | cook substitute, genuine ingredient cost |
| `lucas.check` | bridge evidence | reveal safety risk before reopening |
| `council.first` | 2+ critical viewpoints known | decision scene, preread |
| `council.return` | after committing outcome | delayed consequence in 2 days |
| `river.storm` | day >= 4 and water project set | seeded setback, repair path |
| `village.news` | revisit after world-day tick | dialogue changes from actions |
| `bram.reconcile` | accusation + later evidence | repair trust |
| `miller.shortage` | marsh-protect route | relief quest |
| `fen.restore` | mill-first route | restore habitat |
| `tilda.apprentices` | channel-build route | keep labor promise |
| `trade.caravan` | bridge reopens | returns trader and new opportunity |
| `chapter.final` | central problem resolved + follow-up | deed and royal seal hint |
| `chapter.epilogue` | after completion | open-end exploration with no dead end |

**At least three outcome families** and **two delayed state changes** must be demonstrable in emulator and tests, not merely scripted prose with identical effects.

## Authoring conventions

Every scene has stable ID, location, preconditions, readable multi-page prose, zero or more choices and their preconditions/effects, consequence entry, and repeat policy. The world map can let a player leave a scene, but **never apply an action until option confirmation**. Deterministic scene selection supports QA recordings. Authored prose must make enough of the stakes legible for a reasonable child to make an informed choice, even if they cannot predict outcomes perfectly.

Narrative style: vocabulary suited to an advanced third-grader, several pages are welcome, lightly funny dialogue and characters with motives. Tone should be adventurous, not preachy or moralizing. A character's criticism is their viewpoint, not the game's verdict.
