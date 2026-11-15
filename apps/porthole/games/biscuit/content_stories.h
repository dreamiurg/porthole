// Biscuit's fifteen branching stories (the first seven came from the standalone firmware). Plain data. {name} is the
// child (the profile name), {pet} the dog's name:
// run every page, prompt and ending through personalize() before drawing it. Titles, subtitles and labels are shown
// as written. ids and array order are stable (saves keep one bit per story by index): append, never reorder.
#pragma once
#include <stdint.h>

namespace biscuit {
constexpr int STORY_PAGES = 10, ENDING_PAGES = 4;
struct StoryChoice { const char* label; const char* ending[ENDING_PAGES]; };
struct Story {
  const char* id;
  const char* title;
  const char* subtitle;
  uint8_t unlockDay;              // days together before it opens
  const char* pages[STORY_PAGES];
  const char* prompt;             // the question before the two choices
  StoryChoice choices[2];
};

inline constexpr Story STORIES[] = {
  {"moon", "The Moon Biscuit", "A midnight adventure", 1, {
     "At midnight, a perfect wedge vanished from every moon biscuit in Pepper's bakery window. The glass was locked. By morning, golden crumbs lay outside, on the wrong side of the glass.",
     "Pepper had opened the shop only yesterday. \"Mice,\" she declared. {pet} tilted his head at a crescent. \"Such tidy bites! My crumbs go everywhere. Do these mice carry rulers?\"",
     "{name} spread flour beneath the cabinet before closing time. At midnight the clock struck thirteen. Something clicked inside the wall. Another row of moons acquired identical missing pieces.",
     "There were no tracks in the flour. The crumbs outside smelled of lemon; the biscuits inside smelled of ginger. {pet} nudged {name}'s hand. \"Two smells! Come sniff. I'll save you a spot.\"",
     "Pepper weighed the tray. It weighed exactly what it had before. \"So the missing bits haven't gone far,\" {name} said. {pet} looked underneath. The cabinet had an unusually thick wooden floor.",
     "A thin brass wire ran from the cabinet into the wall. Upstairs, they found a shuttered skylight, a warm chimney, and a photograph of the shop wearing bunting: THE EDIBLE OBSERVATORY.",
     "In the photograph, a woman pointed at a crescent while children peered through a telescope. Pepper recognized her grandmother. \"She was a baker,\" she said. \"Mostly. Apparently not entirely.\"",
     "Behind the clock was a second dial with twenty-nine little marks. Its handle was sticky. Pepper had wound it while cleaning. \"I thought it was the bell. The bell was also sticky.\"",
     "The chimney breathed out warm, lemon-scented air. Below them, a weight rattled inside the cabinet. Two trails led away from the clock: one toward the roof, one beneath the display.",
     "\"We could follow the wire up and see what it turns,\" {name} said. \"Or find out where the pieces are hiding.\" {pet} approved of both plans, especially the one containing pieces.",
   }, "Which part of the old observatory shall we investigate?", {
     {"Follow the wire upstairs", {
       "The wire lifted a shutter over a telescope. A metal model beside it showed how much of the moon was lit. Each night, the same mechanism lowered a different cutter into the cabinet.",
       "Its old clock was running too fast: tonight's crescents belonged to next week. {name} found a marked lever that slowed the model. Pepper watched the real moon and adjusted the dial.",
       "From the roof they spotted lemon crumbs falling from the next chimney. The night baker next door was cooling buns. Two mysteries had been walking around in the same crumb costume.",
       "Pepper reopened the Edible Observatory. Visitors compared biscuits with the moon; a hidden drawer supplied the cuttings. {pet} watched beside {name}, with star-shaped crumbs on his nose.",
     }},
     {"Open the cabinet's base", {
       "{name} found a latch beneath the thick floor. A drawer slid out, full of ginger wedges and a floury notebook. {pet} inhaled. \"Excellent. The missing moon has been keeping a diary.\"",
       "The notebook explained the clock-driven cutters, then recorded thirty disastrous recipes. One biscuit bent a spoon. Another floated. On the last page: USE THE OFFCUTS. THEY MAKE STARS.",
       "Pepper fitted a star cutter into the spare slot. At the next click, the cabinet produced a crescent and three tiny stars. Nothing was missing; the tray had been hiding half the show.",
       "They traced the outdoor crumbs to lemon buns cooling next door. Pepper added her own recipe to the notebook. {pet} tried to help turn the page, leaving its first floury paw print.",
     }},
   }},
  {"library", "The Secret Library", "A mystery between the shelves", 1, {
     "The atlas showed a river through the middle of {name}'s street. The newer atlas showed no river at all. Neither mentioned the sound of running water behind the dictionary shelf.",
     "{pet} put his nose against the wood. \"Damp stone. Old paper. Someone's cheese sandwich.\" {name} pulled out a dictionary. A green door appeared behind it, with a brass card slot.",
     "A mouse librarian opened the door. \"You're early,\" she said. \"For what?\" asked {name}. The mouse consulted a card. \"Your book return. It was due eighty-three years before you borrowed it.\"",
     "The card bore {name}'s address, but another name. Beyond the door, shelves disappeared into the dark. Their labels were peculiar: LOST ROADS, UNBUILT BRIDGES, PLACES PEOPLE ALMOST REMEMBER.",
     "\"We lend books to houses,\" the mouse explained. \"People move. Addresses usually stay.\" She showed them the missing volume: a mapmaker's notebook borrowed by a former resident of {name}'s home.",
     "Back upstairs, {name} found a folded map tucked inside the old atlas. Half the river was blue. Half was dotted. At the edge, someone had written: THE WATER KNOWS THE WAY HOME.",
     "{pet} noticed that the map smelled of the same damp stone as the secret library. \"Either the river is very good at reading,\" he said, \"or we have found its entrance.\"",
     "The mouse brought two other records: a plan of the old river tunnels, and a register of letters sent to {name}'s address. Each mentioned a person called Kit, but neither gave a surname.",
     "The tunnel plan ended under the market. The letters ended with a parcel delivered to the library, never opened because nobody could agree which shelf an unfinished map belonged on.",
     "{name} spread the records across the desk. One route followed the missing river. The other followed its mapmaker. {pet} rescued the cheese sandwich from beneath the oldest document.",
   }, "Shall we follow the water, or the person who mapped it?", {
     {"Trace the hidden river", {
       "The librarian led them down a dry walkway beside the tunnel. Under the market, the river still flowed. Kit's dotted line marked a channel covered when the town built its new streets.",
       "On the wall, Kit had carved measurements beside a tiny kingfisher. Farther along, a real kingfisher watched from a grating. Above it, daylight shone through a forgotten courtyard.",
       "They returned with a page from a sealed survey box. It described a garden beside the river, never built. In the unopened parcel, the mouse found the rest of Kit's notebook.",
       "{name} drew the courtyard onto the town map. Soon neighbors began opening its locked gate. {pet} chose a bench near the water. \"Room for both of us,\" he said, resting his chin on {name}'s knee.",
     }},
     {"Read the mapmaker's letters", {
       "The letters were from Kit to her brother, who had moved inland. Each described a sound: the ferry bell, rain beneath a bridge, pebbles rolling where the river grew shallow.",
       "Inside the unopened parcel lay the notebook and a second map. This one had no street names. Instead, little symbols marked places where her brother could recognize home by listening.",
       "\"Two maps of one town,\" {name} said. \"They answer different questions.\" The mouse made a new shelf: WAYS OF FINDING HOME. Neither atlas had room for the memory of a ferry bell.",
       "{name} began a sound map of the neighborhood. {pet} contributed the bakery door, the squeaky park gate, and a detailed account of his dinner bowl. The river received its own page.",
     }},
   }},
  {"dragon", "A Dragon's Bedtime", "A tale for sleepy explorers", 1, {
     "A dragon in purple pajamas arrived carrying six scorched pillows and a book tied shut with string. \"I have tried warm milk,\" she said. \"The saucepan is now part of the ceiling.\"",
     "Her trouble had begun with a bedtime story. Whenever she reached the final sentence, another page appeared. She had been reading for three nights. The hero still hadn't reached the island.",
     "{name} opened the book carefully. A sailor stood on a ship's deck. Below the illustration: At last, the island came into view, but-. A new page slid out. On it, a storm was gathering.",
     "{pet} sat on the loose page. \"Would a cuddle help?\" The book wriggled underneath him. The dragon apologized to it, which produced a page about a shipwrecked apology.",
     "Inside the cover they found a label: STORIES FOR TWO VOICES. The dragon frowned. \"I have only one voice. A loud one, admittedly.\" {name} noticed a second ink color in the margins.",
     "Those notes belonged to the sailor: I could use a harbor. A smaller storm would also do. The printed narrator answered every request with another difficulty. Nobody had asked the island.",
     "{name} held the page near a lamp. Beneath the waves, pale words appeared: I HAVE BEEN HERE ALL ALONG. There was no picture beside them, just the outline of an enormous sleeping turtle.",
     "\"That island moves,\" {name} said. \"The sailor is chasing a turtle.\" The dragon stared. {pet} considered this. \"I have chased worse things. One of them was my own tail, so I won't judge.\"",
     "The book offered two blank speech bubbles. One belonged to the sailor. The other rose from the turtle. Whatever they wrote would give the story its missing second voice.",
     "The dragon uncapped a pen. \"Do we ask where the island is going, or tell the sailor how to stop chasing it?\" Outside, dawn was already putting a pale edge on the rooftops.",
   }, "Whose voice shall we add to the story?", {
     {"Let the island answer", {
       "{name} wrote in the turtle's bubble: I AM GOING TO THE WARM CURRENT. YOU MAY COME, BUT PLEASE STOP DROPPING ANCHORS ON MY BACK. The dragon read it in a magnificent underwater rumble.",
       "The sailor hauled up his anchor. His ship drifted beside the turtle, and the endless storm became a passing shower. The narrator tried to add a whirlpool. The turtle sneezed it away.",
       "A last picture appeared: a ship and a moving island beneath the same moon. The final sentence stayed final. The dragon shut the book and looked around for a pillow that still existed.",
       "{pet} nudged his cushion toward her. By breakfast, the dragon slept with one claw marking the page. Once in a while she murmured directions. The warm current seemed to need a left turn.",
     }},
     {"Let the sailor change course", {
       "The dragon wrote: I WANT TO EXPLORE, NOT ARRIVE. {name} read the words aloud. The sailor looked up from his map, then turned the ship toward a cluster of lights far behind him.",
       "They were other ships. Each carried a traveler chasing the same moving island. The sailor invited them to tie their boats together for supper. A floating harbor formed without a shore.",
       "The last illustration showed lanterns, borrowed chairs, and maps spread across three decks. The narrator left room for another voyage, but stopped inventing reasons to delay supper.",
       "The dragon finally slept. {pet} borrowed the book the following night and curled up beside {name}. After one chapter he yawned. \"Save some adventure for tomorrow.\" The cover stayed quiet.",
     }},
   }},
  {"seed", "The Sleepy Seed", "Some things take their time", 2, {
     "The seed packet came from a botanist's greenhouse. On it she had written SLEEPY SUNFLOWERS, then DO NOT MISTAKE STILLNESS FOR AN ANSWER. \"Sleepy flowers?\" {pet} said. \"We'll get along.\"",
     "{name} planted three seeds in clear pots with paper sleeves. A week later, nothing had appeared. She checked the water, the light, and the packet. {pet} checked for anything resembling lunch.",
     "In the greenhouse, ordinary seedlings leaned toward the windows. The three empty pots sat under an old shade. Its cord ran through a row of pulleys to a small wooden weather vane.",
     "\"Perhaps the seeds are too old,\" {name} said. {pet} sniffed the pots. \"Maybe they're having a very long nap.\" He sniffed again, and his tail stirred. \"Something in there smells alive.\"",
     "{name} slid off a paper sleeve. A white root pressed against the clear wall, then curved sideways. The seed had sprouted underground. Above it, the soil remained stubbornly undisturbed.",
     "A notebook lay beneath the bench. Most pages recorded ordinary things: dates, rain, soil temperature. One page showed the greenhouse before a shed had been built outside its eastern wall.",
     "The shed now blocked the morning sun. Worse, its gutter dripped into the shade's pulley. The swollen wood had jammed the shade shut. Someone had carefully watered a patch of permanent dusk.",
     "They freed the cord. A band of sunlight crossed the pots. Under the bench, another label appeared: THREE WINDOWS, THREE POSSIBLE GARDENS. The botanist had drawn a different view beside each.",
     "The south window faced a bare courtyard. The west looked onto a high wall. The third view showed the inside of a pot, full of curling roots. There was room to try something new.",
     "\"We could move the plants outside and see what follows them,\" {name} said. \"Or keep a root window and watch what happens underneath.\" {pet} preferred any garden that admitted dogs.",
   }, "What sort of garden shall we make with the newly sprouted seeds?", {
     {"Plant a courtyard lookout", {
       "They planted two seedlings by the sunny courtyard and kept the third indoors until it grew stronger. Weeks passed. The stems climbed. {pet} used one as an unreliable measure of his height.",
       "When the flowers opened, bees appeared first. Then came finches, arguing over seeds before any were ready. {name} added arrival dates to the botanist's notebook, beneath the weather records.",
       "One afternoon, a neighbor brought a photograph. The courtyard had once been full of sunflowers. The finches were returning to a place their own families had visited before the shed existed.",
       "{name} left some seed heads standing through autumn. A finch landed while {pet} watched from his cushion. \"Our flowers made friends!\" The notebook gained a page called UNEXPECTED VISITORS.",
     }},
     {"Build an underground window", {
       "They kept one plant in a deep clear container, covered except when {name} made a sketch. The others moved into roomy pots. {pet} napped at her feet, waking when her pencil tickled his ear.",
       "The sketches revealed a surprise: the roots changed direction toward the wetter side. Above the soil, the stem turned toward light. One plant was following two different maps at once.",
       "{name} compared her drawings with the botanist's final page. The unfinished design was a planter with a removable dark panel, a window into a place gardeners usually had to guess about.",
       "At the next garden gathering, visitors looked below the leaves. {pet} dozed beside the display, paws twitching in a dream. {name} made him a sign too: DOG RESTING. SEEDS BUSY.",
     }},
   }},
  {"cloud", "The Cloud Collector", "An adventure in looking up", 3, {
     "Professor Vale kept clouds in jars. Small ones, fortunately. His shelves held a sulky drizzle, a magnificent thunderhead the size of a cabbage, and a jar labeled NOTHING MUCH, PROBABLY.",
     "The last jar had vanished. Vale was upset. \"Someone took my most unusual specimen!\" {pet} tipped his head at the label. \"What was so special about it? Even little things can surprise you.\"",
     "The latch was intact. No shelf was empty. In fact, there was one extra jar, full of perfectly ordinary fog. Vale insisted that it had not been there when he went to bed.",
     "{name} noticed droplets on the outside of the fog jar. Every other jar was dry. A ribbon tied to its lid pointed toward the open window, although the curtains hung perfectly still.",
     "The professor brought out his notebook. NOTHING MUCH had come from a clear patch above the harbor, between two banks of cloud. Its temperature had changed whenever a ship passed below it.",
     "\"You collected a gap,\" {name} said. Vale straightened. \"A very particular gap.\" {pet} sneezed toward the fog. The ribbon swung toward him. He retreated. \"It appears to have opinions.\"",
     "At the window, they could see a narrow stream of fog moving uphill against the breeze. It curled around chimneys, crossed the square, and disappeared behind the shuttered old conservatory.",
     "A delivery label on the strange jar read RETURN TO SOURCE. Beneath it, in Vale's own handwriting: DO NOT OPEN BESIDE ANOTHER SPECIMEN. \"I remember writing that,\" he said. \"Not why.\"",
     "The absent gap might be pulling fog from wherever it had escaped. Or the new jar might be pushing its weather into town. {name} marked the fog's direction on the window with her finger.",
     "They could follow the moving trail uphill, or compare the jars in the professor's sheltered testing cabinet. {pet} tucked his nose under {name}'s hand. \"My nose would like a little break.\"",
   }, "Shall we follow the wandering weather, or work out what the jars are doing?", {
     {"Follow the uphill fog", {
       "Inside the conservatory, the missing jar stood open beside a dry fern. The caretaker had borrowed it, believing its label promised nothing much. Now every leaf wore a necklace of water.",
       "The gap was drawing in the town's fog, then releasing it as a gentle mist. It wasn't empty; it was a space that wanted filling. The fog jar on Vale's shelf was its packed lid.",
       "Vale arrived carrying the lid and three pages of objections. He stopped beside a fern unfurling its first new frond. Together, they adjusted the opening until the wandering fog thinned away.",
       "The conservatory reopened as a cloud garden. Vale kept careful weather records. {pet} greeted visitors with a wag and a towel. He loved the ferns, but wet paws still felt like wet paws.",
     }},
     {"Compare the jars safely", {
       "Inside the testing cabinet, {name} turned the fog jar upside down. Its label slipped, revealing a brass thread around the base. It wasn't another jar at all. It was the missing jar's lid.",
       "Vale had built a lid that expanded to hold escaping weather, then forgotten to note it on the shelf. The caretaker had borrowed the open jar for a fern in the old conservatory.",
       "They reunited the two pieces. The uphill fog stopped at once. Vale then made a smaller version: a lid that caught morning mist and released water slowly around the caretaker's plants.",
       "The new label read WEATHER TRANSFER LID. {pet} inspected it. \"Better,\" he said, \"although I would add WHICH END OPENS.\" The professor considered this, then fetched another label.",
     }},
   }},
  {"lighthouse", "A Light for Lost Paws", "A little light goes a long way", 5, {
     "The lighthouse flashed three times. Somewhere in the fog, another light answered twice. Captain Mara closed her logbook. \"That lamp should have been taken down forty years ago,\" she said.",
     "She had another problem: Pip, the harbor dog, had vanished during his evening rounds. His barking seemed to come from the old boathouse, then from the opposite side of the harbor.",
     "{pet} listened at the lighthouse door. The second bark was fainter, but identical. His ears perked up. \"Two Pips? Or is the harbor playing copycat? Listen, it's the very same woof!\"",
     "Mara spread a chart on the table. A stone wall faced the boathouse across the water. {name} traced the route a sound might take. The echo explained two dogs, but not the extra light.",
     "An old photograph showed a low signal lamp beside the boathouse. Its window faced the harbor through a narrow arch. In the new chart, that arch had been bricked up after a storm.",
     "Yet something still flashed there: two short bursts, then a pause. Mara checked the lighthouse controls. Nothing was broken. The reply arrived only when the beam swept across the boathouse.",
     "Pip had left his rounds notebook at the dock office. His last entry was a drawing: a paw, a little box, and the same arch from the photograph. He had circled the box twice.",
     "\"He writes better pictures than I do,\" {pet} said. {name} noticed that the box had a handle. Mara recognized it as the harbor's missing inspection lantern, last seen beside a repair cart.",
     "There was a dry service path to the boathouse, safely away from the water. Mara took its key. {name} could help her locate Pip by comparing the flashes, or by timing the answering barks.",
     "The fog made everything look like a half-erased drawing. {pet} stayed close to {name} and Mara. \"Let's find Pip,\" he said. \"If he's missed supper, he can share mine.\" His tail gave a wag.",
   }, "Which signal shall we use to find Pip with Captain Mara?", {
     {"Map the returning light", {
       "{name} watched three sweeps of the beam while Mara checked the chart. The double flash came from two tiny windows above the blocked arch. They followed the service path to the old lamp room.",
       "Pip was inside, perfectly comfortable but locked behind a swollen door. Beside him stood the missing lantern. He had found it while investigating a glint and barked when the door stuck.",
       "Two glass prisms in the abandoned lamp split the lighthouse beam, sending the mysterious reply. Mara freed Pip, then held the prisms against a wall. Two miniature rainbows opened like fans.",
       "The old lamp became a display at the harbor museum. Pip contributed his drawing to the label. {pet} suggested an addition: THIS DOOR STICKS. Nobody objected to that bit of history.",
     }},
     {"Listen between the echoes", {
       "{pet} barked once. Pip answered, followed by the fainter echo. As they walked with Mara, the first reply grew clearer. {name} ignored the copy and listened for the scrape between Pip's barks.",
       "It came from the old lamp room. Pip was nudging a lantern along the floor toward the stuck door. Mara opened it with her service key and a determined shove. Pip emerged carrying the handle.",
       "Above him, two glass prisms caught the lighthouse beam. That explained the flashes. The scraping supplied the missing clue: unlike the bark, its quiet sound had not crossed the harbor twice.",
       "Mara added the echo to her harbor guide so visitors would know why sounds changed places. Pip resumed his rounds with {pet}. At the dock caf\u00e9, two wagging tails arrived before {name} did.",
     }},
   }},
  {"comet", "The Comet's Letter", "Friends, however far apart", 7, {
     "The silver envelope arrived inside a library book. It said WHOEVER FINDS THIS NEXT, and its stamp showed a comet that looked as though it had missed an appointment.",
     "Inside was a letter: I am passing your world again. Last time, a girl in a red coat waved from the hill. I promised to bring her news. Please tell me where she has gone.",
     "There was no date, only a drawing of the stars. {name} recognized the hill, but a windmill stood where the observatory stood now. {pet} sniffed the paper. \"Old. Also faintly electrical.\"",
     "At the observatory, Keeper Lin compared the drawing with her records. The comet had last passed the town seventy-six years earlier. The girl in the red coat would be considerably older now.",
     "\"Does it know how long it's been?\" {name} asked. Lin showed them a note in an old log: VISITOR THINKS ONE ORBIT IS A SHORT ERRAND. {pet} leaned into {name}. That was a long time between cuddles.",
     "The letter described ice mountains, a moon with an underground sea, and a rock that looked exactly like a turnip. At the bottom: I kept the pebble she gave me. It helps me remember home.",
     "A photograph in the log showed three children beside the windmill. One wore a red coat. None was named. On the back, someone had drawn a star inside a square, the mark of an old bookbinder.",
     "Lin found a timetable for tonight's passing. The comet would be visible from the hill for a few minutes. The bookbinder's shop, now a repair shop, stood at the other end of town.",
     "They could follow the girl's trail before dusk, or prepare a reply and search the records afterward. Lin offered her signal mirror for either plan.",
     "\"Let's tell it we're glad it came back,\" {pet} said. {name} folded the letter along its worn creases. Above the clouds, a traveler was returning to a place that had kept changing.",
   }, "Shall we find the old friend first, or make sure the comet receives a reply?", {
     {"Follow the bookbinder's mark", {
       "The repairer recognized the photograph. Her grandmother was the girl in red, and she still lived above the shop. Mrs. Rowan opened the letter, then laughed at the turnip-shaped rock.",
       "\"I asked him to find one,\" she said. From a drawer she took a smooth stone with a hollow in it. It had once fitted around another pebble, the one now traveling beyond the planets.",
       "They reached the hill together. Lin signaled with the mirror while Mrs. Rowan held up her stone. A silver speck brightened overhead. The reply arrived as a new line across the old letter.",
       "YOU HAVE CHANGED, it said. SO HAVE I. Mrs. Rowan began describing her seventy-six years. {pet} settled beside her. This, he suspected, would be a story requiring more than one biscuit.",
     }},
     {"Send news from the hill", {
       "{name} wrote that the windmill was now an observatory, the town had grown, and they were looking for the girl. She drew {pet} too. \"Bigger ears, please! I want it to know me when it visits.\"",
       "Lin signaled at dusk. New words appeared beneath {name}'s message: I THOUGHT I WAS ONLY A LITTLE LATE. She answered that the letter had arrived, and there was still news worth sending.",
       "The next morning they found Mrs. Rowan through the bookbinder's mark. She brought old photographs to the observatory. They began a town journal for a reader with a very long route.",
       "A small silver margin remained in the comet's letter. Every few nights, another reply appeared there. {pet} sent a drawing of a turnip and a muddy paw print. \"So it gets a hello from me.\"",
     }},
   }},
  {"park", "The Dog Park Gate", "Seven paws and one key", 8, {
     "The dog park gate was shut, and a paper sign hung on it: BACK AT TWO. {pet} pressed his nose to the fence. \"I smell tennis balls. I smell puddles. I do not smell anyone in charge.\"",
     "A small crowd waited on the path. {name} counted the dogs: a spotty one, a fluffy one, a tiny one in a sweater. Each had a ball. Each ball was a different color.",
     "The tiny dog's owner sighed. \"Every day the ball bin is full. Today it is empty.\" Through the fence, {name} saw the bin lying on its side beside a very tidy line of wet pawprints.",
     "{pet} sniffed the prints. \"Big paws. Then small paws. Then big, small, big, small.\" {name} noticed it too. The prints made a pattern, like a song you could walk to.",
     "The pattern led to the picnic table. Under it sat a sock full of ball-shaped lumps. {pet} wagged. \"Somebody was saving them. I know that feeling. I once saved a sandwich.\"",
     "A gardener in a green hat waved from the far path. \"Oh dear, did I scare you? I only borrowed the balls to keep them out of the flower bed!\" She looked quite embarrassed.",
     "Beside her was a wheelbarrow of fresh dirt. \"I planted something by the pond,\" she said. \"A surprise for the dogs. But I locked the gate so nobody trampled it.\"",
     "{name} looked at the pond. A little fence of sticks stood in a ring. Inside it, something small and green poked up. {pet} leaned forward. It smelled like grass, and a bit like mint.",
     "The gardener held two things: the gate key, and a bag of seeds. \"I can open the gate now, or you can help me finish the planting first.\" She smiled hopefully.",
     "The dogs were whining politely. {name} knelt by {pet}. \"Which way is better?\" {pet} thought about it with his whole body, which mostly meant his tail.",
   }, "What shall we do first?", {
     {"Open the gate first", {
       "The gardener turned the key. The dogs poured in like a happy river. The spotty one found the ball sock at once and scattered every ball across the grass.",
       "{name} showed the gardener the stick ring. She asked everyone to walk around it. The dogs followed the paw pattern, big, small, big, small, and stayed clear of the little plants.",
       "By evening, the sprouts stood tall and untrampled. The gardener hung a new sign: PLEASE STEP AROUND THE MINT. The dogs seemed to read it, or at least to smell it.",
       "{pet} lay in the mint patch for a moment, just to be sure. \"Cool,\" he said. Then he got up and chased the red ball, because nobody can guard mint all day.",
     }},
     {"Help plant the seeds", {
       "{name} pressed seeds into the soil, three to a hole, while {pet} supervised from the shade. The gardener counted holes out loud. There were twelve, in rows of three.",
       "\"Four rows of three,\" {name} said. The gardener nodded. \"Mint likes company.\" {pet} dug one hole of his own, very quickly, and was asked to please undig it.",
       "When the last seed was covered, the gardener unlocked the gate and carried the ball bin back. The dogs rushed in to a park with a brand new corner of dirt that smelled like promise.",
       "The sign came down. A new one went up: WATER ME, A LITTLE BIT EACH DAY. {pet} took that as a personal job. He carried a stick to the pond to help, and it was not wet enough.",
     }},
   }},
  {"snow", "The Snowed-In Morning", "A quiet white world", 10, {
     "{name} woke up and the window was all white. No street, no mailbox, no fence. {pet} stood on the bed with his front paws on the sill. \"Somebody turned the world into a blanket.\"",
     "Outside, the snow was up to {pet}'s tummy. He took one step and sneezed. Then another. \"Cold nose,\" he reported. \"Good cold, though. Like a surprise.\"",
     "Across the yard, {name} spotted a line of tiny marks in the snow. Each was two little dots, then a long scratch. They crossed the lawn and stopped at the old woodpile.",
     "{pet} sniffed the first mark. \"Small. Quick. Smells like seeds.\" {name} followed the marks, counting. Eleven steps, then a jump gap, then eleven more. The animal liked to hop.",
     "At the woodpile, the marks went two ways. One trail curved toward the bird feeder, which hung empty. The other trail slipped under the porch, where something rustled softly.",
     "{name} found a sunflower seed shell on the step. Then another. Then a tiny pile, nibbled neat. {pet} lay down with his chin on the snow. \"Somebody had breakfast here.\"",
     "From under the porch came a little chirp, and then a puff of tiny breath in the cold air. {pet} held very still, which is hard for a dog, and wiggled only his tail.",
     "A small gray bird hopped out. It had a puffed-up chest and one feather sticking up. It looked at the empty feeder, then at {name}, then at the empty feeder again.",
     "{name} remembered the seed bag in the shed. The path to the shed was deep snow. The path to the feeder was shorter, but the feeder was frozen shut and needed warming.",
     "The bird waited. {pet} leaned on {name}'s boot. \"Whatever we do, let's do it before my paws turn to icicles.\" The little bird hopped closer, as if it understood.",
   }, "How shall we help the little bird?", {
     {"Fetch seeds from the shed", {
       "{name} and {pet} plowed through the drift, leaving a long trail. {pet} bounced like a rabbit. At the shed, a bag of sunflower seeds sat on the very top shelf.",
       "They scattered a handful on the porch step. The little bird hopped over, then three more birds came, then five. {name} counted eight in all, each with a tiny puffed chest.",
       "The birds ate and chirped and left small stitches of footprints across the step. {pet} sat very still, though his tail swept the snow into a neat fan.",
       "That afternoon, {name} thawed the feeder with warm water and filled it. By evening it swayed with birds. {pet} dozed on the rug, paws twitching, dreaming of fluffy things.",
     }},
     {"Warm up the frozen feeder", {
       "{name} carried a cup of warm water out and poured it carefully on the lid. The ice cracked and slid off. Inside the feeder, a few old seeds lay dry and ready.",
       "The little gray bird hopped onto the perch at once. It pecked, chirped, and shook snow off its wings. {pet} watched from the step, ears up. \"He says thank you.\"",
       "Then {name} noticed something: the feeder had a tiny door on the back, the kind that opened for refilling. Someone had left it unlatched, which was how the cold had gotten in.",
       "{name} latched it and refilled the feeder from the shed. {pet} got a biscuit for helping. He ate it by the window, watching the bird eat its seeds. Two breakfasts, side by side.",
     }},
   }},
  {"train", "The Little Train", "Stops along the way", 12, {
     "The little train hummed into the station at noon. {pet} had his ticket in his teeth, a bit soggy, and {name} held the picnic basket. \"Window seat,\" {pet} said. \"Always the window seat.\"",
     "The conductor had a bushy mustache and a punch that made a star-shaped hole. She tapped {pet}'s ticket. \"Next stop, Maple Meadow. Four stops after that, the sea.\"",
     "The train rolled past fields and fences and a very long line of laundry. {name} counted the stops on the wall map: Maple Meadow, Bridge Hill, Apple Gate, Pond End, then the sea.",
     "At Maple Meadow, a boy got on with a big paper bag. He sat near {pet} and the bag crinkled. {pet} sniffed. \"Cinnamon,\" he whispered. \"I will be very polite about it.\"",
     "At Bridge Hill, the train rattled over a long iron bridge. Under it, a river flashed like a silver ribbon. {name} noticed that the bridge posts came in sets of three, over and over.",
     "At Apple Gate, the conductor walked down the aisle looking worried. \"One passenger left without his scarf,\" she said. \"A long red one. It's a favorite of someone.\"",
     "{pet} sniffed the floor and the seats. Under the window bench he found it: a red scarf, soft and a little cinnamon-y. He dragged it into the aisle, proud and slightly tangled.",
     "The boy with the paper bag looked up. \"That's my grandpa's,\" he said. \"He's at the front car. He went to get a snack and I forgot to tell him where the scarf went.\"",
     "The train slowed for Pond End. Outside the window, ducks floated in a row. The conductor checked her watch. \"We have two minutes. The front car is quick to walk to, but the doors may stick.\"",
     "{name} looked at the scarf, then at the boy, then at {pet}. The boy was too shy to walk alone. The conductor had a whole train to watch. Someone had to carry the scarf.",
   }, "Who shall carry the scarf to the front car?", {
     {"Send the dog with the boy", {
       "{pet} trotted down the aisle with the scarf trailing behind like a flag, and the boy followed close behind. The doors stuck a little, but {pet} nosed one open for them.",
       "In the front car, a grandpa with kind eyes looked up. \"My scarf! And a dog I have never met!\" He wrapped it around his neck and patted {pet}'s head twice.",
       "He opened his own paper bag and gave {pet} a small oat cookie. \"For the trip,\" he said. It crumbled in just the right way, and {pet} ate every crumb off the floor.",
       "When the train rolled on to the sea, {pet} sat beside the grandpa and the boy. {name} stayed by the window and counted three ducks, three more ducks, and a brave seagull.",
     }},
     {"Walk the scarf up ourselves", {
       "{name} carried the scarf, and the boy came too, holding the cinnamon bag in both hands. {pet} stayed on the bench and kept the picnic safe. Someone had to.",
       "The doors stuck, but together they pushed. In the front car, the grandpa was looking everywhere. \"My scarf!\" he cried. The boy held it up, grinning for the first time.",
       "He gave {name} a cinnamon roll from the paper bag, still warm. {name} saved half for {pet} and walked back through the cars. The floor tilted gently. The train sang its steady song.",
       "{pet} met them at the seat with a wiggle. The roll was gone in two bites, and he liked every bite of it. The sea showed up soon, bright and big and blue.",
     }},
   }},
  {"museum", "The Night at the Museum", "A hush and a flashlight", 14, {
     "The museum closed at five, but tonight there was a sleepover just for kids and one dog. {pet} wore a name tag. It said GUEST, which he considered a promotion.",
     "The guide, Mr. Okoro, gave everyone a small flashlight. \"Shine it softly,\" he said. \"The old things are sleepy.\" {pet} padded behind {name}, nails clicking on the marble floor.",
     "They passed a stone lion, a giant clock, and a glass case full of tiny silver spoons. {pet} stopped at a painting of a woman holding a loaf of bread. \"Smells old,\" he decided. \"Good old.\"",
     "In the next room, a long dinosaur skeleton stretched across the ceiling. {name} shined the light along its tail. Each bone had a small brass number. The last one read twenty-two.",
     "Mr. Okoro frowned at his clipboard. \"That is odd,\" he said. \"There should be twenty-three. One of the tail bones is missing, and I counted them before dinner.\"",
     "{pet} put his nose to the floor. Under the skeleton, he found a faint trail of flour dust. It led away, past the lion and out of the room. \"Somebody snacked,\" he reported.",
     "{name} followed the dust to the gift shop. The door was ajar. Inside, a single flashlight lay on a shelf of plush dinosaurs, shining on a bone-shaped biscuit tin.",
     "A girl in a nightgown was fast asleep on the floor beside the tin, clutching something white to her chest. It was a bone, a smooth long one, with a brass number twenty-three.",
     "Mr. Okoro stopped in the doorway. \"Oh, goodness,\" he whispered. \"That one is a replica. We keep it in a bin for the toddlers to touch. It must have rolled out to the hallway.\"",
     "{pet} nosed the tin. It held ordinary biscuits, which was a relief. {name} looked at the sleeping girl, then at the skeleton, then at the long dark hallway between them.",
   }, "What shall we do with the bone and the girl?", {
     {"Let her keep sleeping", {
       "{name} turned off the flashlight and {pet} lay down beside the girl, warm as a rug. Mr. Okoro tiptoed away to find a blanket. The museum creaked softly, like a ship at night.",
       "In the morning, the girl woke with the bone still in her arms. She was not embarrassed. \"It was cold,\" she explained. \"And the dinosaur has so many bones. He would not miss one.\"",
       "Mr. Okoro laughed and showed her the bin of replicas. \"This is the bone for hugging,\" he said. He wrote her name on the bin. She helped him hang the number back on the real tail.",
       "{pet} got a biscuit from the gift shop tin for guarding her sleep. He ate it under the dinosaur, who looked down on him with sleepy stone eyes. It felt like friendship.",
     }},
     {"Carry the bone back first", {
       "{name} lifted the replica carefully and walked it back down the hallway, with {pet} lighting the way by wagging. In the dinosaur room, a small empty spot waited at the tail.",
       "Mr. Okoro showed {name} how the replica clipped in with two tiny hooks. Click. The count was twenty-three again. The brass number gleamed. {pet} sat as if he had done it himself.",
       "When they returned to the gift shop, the girl was awake and rubbing her eyes. \"Where did my bone go?\" she asked. {name} gave her a smaller plush one, and Mr. Okoro gave her the tour.",
       "She turned out to love the dinosaur room. She counted every number aloud, all the way up to twenty-three, and then {pet} counted too, in his own way, with one soft woof for each.",
     }},
   }},
  {"fort", "The Rainy Day Fort", "Under the blanket roof", 16, {
     "Rain tapped on the window all afternoon. {name} sat on the couch with a pile of pillows, three blankets, and a long clothespin. {pet} watched from the rug. \"We could build something,\" he said.",
     "They draped the blanket over two chairs. It sagged in the middle like a sleepy hammock. {pet} crawled under and sneezed from the dust. \"Cozy,\" he announced. \"But the roof is drooping.\"",
     "{name} fixed it with a broom handle. The fort stood up, wobbly and proud. Inside was a pillow floor and a flashlight. {pet} turned around three times and flopped down.",
     "Under the pillow floor, {name} felt something hard. It was a small flat box with a tiny brass lock and no key. On the lid, someone had scratched a note: THREE KNOCKS, THEN LOOK UP.",
     "{pet} sniffed the box. \"Smells like crayons and an old mitten.\" {name} knocked three times on the lid, then looked up at the blanket roof. Nothing happened. It was just a blanket.",
     "{name} looked again. Stitched into the corner of the blanket was a tiny pocket, no bigger than a pocket for a button. Inside the pocket was a thin paper, folded very small.",
     "The paper said: THE KEY IS WHERE THE DOG SLEEPS. {pet} lifted his head. \"I sleep in many places,\" he said. \"The rug, the couch, the sunny spot, and the sock drawer, when I can.\"",
     "{name} counted on fingers. Rug, couch, sunny spot, sock drawer. Four places, and only one key. The rain kept tapping on the window like someone knocking to be let in.",
     "{pet} remembered something. His bed, the round one with the stars on it, had a small seam on the side. He had always wondered about the seam. It smelled faintly of metal.",
     "{name} looked at the rug. Then at the dog bed across the room. One was easy to check, and the other was a surprise waiting to be found. Both had good smells.",
   }, "Where shall we look for the key?", {
     {"Check the dog bed seam", {
       "{name} unpicked the seam gently. Out slid a small brass key on a faded blue ribbon. {pet} looked astonished. \"All this time I was sleeping on a treasure,\" he said. \"Rude.\"",
       "The key turned with a soft click. Inside the box lay a handful of old crayons, a button, and a drawing of a fort exactly like theirs, signed by a child from long ago.",
       "On the back it said: FOR WHOEVER BUILDS IT NEXT. {name} added a drawing of {pet} in the fort and tucked everything back inside. The rain eased up, a little at a time.",
       "{pet} curled in the fort that evening, the box between his paws. The roof sagged again, but only a little. It was the kind of sag that keeps a fort feeling like home.",
     }},
     {"Search the sunny spot", {
       "{name} pulled up the corner of the rug in the sunny spot. Beneath it, a loose board wiggled. Under the board lay the key, wrapped in a bit of old handkerchief. {pet} sneezed happily.",
       "The key opened the box. Inside lay a stack of folded notes, each one from a different kid who had built the fort before. They were all about rainy days and snacks.",
       "{name} read three aloud. One said: ADD A FLASHLIGHT. One said: ADD A DOG. {pet} wagged. He had been in the fort for years, apparently, without realizing it was his job.",
       "{name} wrote a new note and added it to the pile. It said: THE FORT IS BEST WITH A DOG. {pet} put one paw on the paper, just to make it official, and the rain sang on.",
     }},
   }},
  {"tide", "The Tide Pool Door", "Small worlds by the sea", 18, {
     "The tide went out far that morning. {name} carried a bucket along the rocks, and {pet} trotted behind, nose high. \"The sea smells like salt and old seaweed,\" he said. \"I love it.\"",
     "In a rocky dip sat a little pool of clear water. {name} knelt. A small crab scuttled sideways. A star-shaped creature sat very still. {pet} put one paw close, then thought better of it.",
     "A hand-painted sign stood by the pool. It was worn and salty, and the letters were faded. It read: PLEASE LOOK, DO NOT T-UCH. One letter was missing, and the 'O' had been added with a pen.",
     "{name} laughed. \"It should say TOUCH, but it says T-UCH.\" {pet} cocked his head. \"A word with a hole in it,\" he said. \"Like a biscuit that has been nibbled on one side.\"",
     "Beside the sign, {name} noticed a row of limpets stuck to the rock. They made a neat line, three, then a gap, then three, then a gap. It was a pattern, like a quiet song.",
     "Past the third gap, one limpet was missing. In its place was a small round hole in the stone, no bigger than a button. Water trickled in and out, slow and shiny.",
     "{pet} put his nose near the hole. \"It smells like a secret,\" he said. \"Cool and a little bit sweet.\" {name} looked closer. The hole was lined with tiny shells, arranged just so.",
     "A gull cried overhead. The tide had begun to turn, and a thin line of foam crept up the rocks. {name} had maybe ten minutes before the pool would be covered again.",
     "The hole whispered when the water ran through it. In the pool beside it, a pale shell shaped like a tiny door lay half buried in the sand. It was just the right size for a hermit crab.",
     "{name} could look closely at the shell door, or write the missing letter on the sign and leave the shell alone. {pet} shifted his paws. \"Either way, let's be quick. I do not like wet toes.\"",
   }, "What shall we do before the tide comes back?", {
     {"Look closely at the shell door", {
       "{name} crouched and studied the little shell. A hermit crab stepped out, wearing it like a hat. It waved a claw, then tucked back in, as though it had been waiting for visitors.",
       "The crab walked to the hole and slipped through. Under the rock, a tiny cave opened up, lined with polished shells. It was a little house, tidy and snug, with a doorstep.",
       "{name} did not touch a single thing. The tide came in softly and covered the cave, one wave at a time. {pet} watched the water close like a quiet curtain.",
       "On the way home, {name} drew the pattern of limpets in the sand: three, gap, three, gap. {pet} carried the bucket, empty and proud. Nothing had been taken, everything had been seen.",
     }},
     {"Fix the sign with the missing letter", {
       "{name} found a small pink stone and carefully wrote an O in the gap, right where it belonged. Now the sign said, PLEASE LOOK, DO NOT TOUCH. It looked like someone had fixed it.",
       "{pet} sat by the sign and guarded it, tail swishing in the sand. A family came along, read the sign, and knelt to look at the crab without a single poke.",
       "The tide came in and covered the stone letter, but the sign still stood, a little more complete than before. {name} smiled. A good sign can speak even when no one is nearby.",
       "On the way back to the beach, {pet} found a smooth blue pebble. {name} let him keep it. He carried it home in his mouth all the way and put it on the windowsill like a trophy.",
     }},
   }},
  {"kite", "The Lost Kite", "Following a string", 20, {
     "A red kite sailed over the hill with a long ribbon tail. It had no string, no child, and no plan. {pet} watched it from the hilltop. \"That kite is out for a walk,\" he said.",
     "{name} ran after it, and {pet} ran faster, ears flapping like flags. The kite drifted over the pond and dipped near the old oak. Then the wind caught it and it was gone again.",
     "On the grass lay a spool of string, the end snapped off clean. {name} picked it up. A small tag was tied to it. The tag said: MY FIRST KITE. PLEASE BRING HOME. And a drawing of a house.",
     "The drawing showed a house with a blue door, a round window, and a tall chimney. {pet} sniffed the tag. \"Smells like a crayon and a chilly morning.\" {name} studied the drawing.",
     "They walked down the hill to the lane of houses. {name} counted doors: red, green, yellow, blue. The blue door had a round window. It was the fourth house on the lane.",
     "At the blue door, no one answered. A watering can sat by the step, and a pair of tiny rain boots stood beside it. {pet} lay down on the mat. \"Somebody lives here,\" he said.",
     "A neighbor in a straw hat leaned over the fence. \"They are away until supper,\" she said. \"The little one cried all morning about her kite. I have been looking, but my knees are old.\"",
     "{name} looked up. High in the oak tree by the pond, a flash of red showed between the leaves. The kite had snagged on a branch. Its tail danced in the breeze.",
     "The oak was tall, and the lowest branch was out of reach. {pet} barked once, politely, at the tree. A squirrel stared back from a knothole, unimpressed.",
     "{name} could climb the neighbor's ladder to unhook the kite, or tie the spool to a stick and try to lasso it down. {pet} wagged. Both ideas sounded like they needed a dog.",
   }, "How shall we get the kite down?", {
     {"Borrow the ladder", {
       "The neighbor lent them her ladder and held it steady with both hands. {name} climbed slowly, one rung at a time, and {pet} waited at the bottom, ears up, tail still.",
       "At the top, {name} reached the kite and gently freed its tail from the twigs. It was whole and unscratched, only a little leafy. The squirrel watched and said nothing.",
       "They carried it down together and left it on the blue doorstep with the spool and the tag. The neighbor tucked a note under the string: FOUND BY A FRIEND. ALL SAFE.",
       "As they walked home, a small voice squealed behind them. A little girl was kneeling by the door, hugging her kite. {pet} wagged so hard his whole back half wagged too.",
     }},
     {"Tie the spool to a stick", {
       "{name} found a long stick and tied the spool to it, with a loop at the end. {pet} stood guard. After three tries, the loop caught the kite's tail and gave a gentle tug.",
       "The kite slipped off the branch and floated down slowly, tail swirling, and landed right on {pet}'s head. He sat very still, wearing it like a bright red hat.",
       "{name} laughed so hard the kite wobbled. They carried the kite to the blue door and tied the string back on, with a little knot for luck. Then they left it on the step.",
       "The next day, they saw the kite flying high over the hill, with a small girl at the other end of the string. {pet} gave one happy bark, and the kite dipped like a wave.",
     }},
   }},
  {"kitchen", "The Kitchen Experiment", "Stir, wait, and see", 22, {
     "{name} wanted to see what would happen if you mixed things from the pantry. {pet} sat on a stool, which was against the rules, but also the best seat. \"I will taste test,\" he offered.",
     "On the counter lay a clear bowl, a spoon, a cup of water, and three small jars: flour, salt, and baking soda. {name} lined them up in a row. A notebook lay open nearby, pencil ready.",
     "First, {name} stirred flour into water. It made a pale, gloopy paste. {pet} sniffed it and leaned back. \"Smells like a wet pancake that gave up,\" he said, ears flat.",
     "Next, a pinch of salt. The paste looked the same, only a bit grainier. {name} wrote in the notebook: SALT: NOTHING VISIBLE. {pet} licked his nose just in case.",
     "Then {name} spooned in the baking soda. Tiny bubbles rose, fizzing softly, like a bottle of sparkling water that had been left to dream. {pet} jumped off the stool and backed away.",
     "\"It's alive!\" he said, but he did not really think so. The bubbles slowed and stopped. {name} wrote: BAKING SODA: BUBBLES, THEN QUIET. The kitchen smelled faintly of nothing much.",
     "On the shelf above, {name} noticed a small bottle of vinegar. A sticky note on the cap said: MIXES WITH BAKING SODA. TRY IT IN A SMALL CUP FIRST. The handwriting was careful and neat.",
     "{pet} sniffed the bottle. It stung his nose. \"Sharp,\" he said. \"Like a sour grape with a grudge.\" {name} laughed, then looked at the bubbling bowl and the little cup.",
     "There were two ways to try it. A little vinegar in the small cup would be tidy and safe. A splash in the big bowl would make a bigger show, but it might spill over.",
     "{pet} watched from a safe distance, one paw on the floor and one on the stool. \"Whatever you pick,\" he said, \"please do it over the sink. I would like to keep my toes dry.\"",
   }, "How shall we try the vinegar?", {
     {"A little in the small cup", {
       "{name} measured one spoonful of baking soda into the cup, then added a few drops of vinegar. The mixture fizzed up in a tidy white crown and stayed inside the rim.",
       "{name} wrote down: SMALL CUP, SMALL FIZZ. Then {name} tried two spoons of vinegar, then three. Each time, the fizz grew a little taller, as neatly as a staircase.",
       "By the sixth step, the cup was full of foam. {pet} peeked over the edge with wide eyes. \"A cloud in a cup,\" he whispered. The notebook page filled with careful marks.",
       "{name} rinsed everything in the sink. The kitchen smelled clean and lemony. {pet} got his taste test after all: a plain cracker, which he declared the best part of the experiment.",
     }},
     {"A big splash in the bowl", {
       "{name} stood over the sink and tipped the vinegar into the bowl all at once. The foam rose fast, climbed the sides, and spilled gently over into the basin like a slow white wave.",
       "{pet} yelped and hopped back, then crept forward again. \"It is only soap with ideas,\" he said, and wagged. The bubbles popped one by one with a tiny crackle.",
       "{name} wrote: BIG SPLASH, BIG FOAM, DO IT OVER THE SINK. The page had a drawing of a volcano with a smiling dog beside it. The bowl was empty and sparkling by the end.",
       "They wiped the counter together. {pet} licked a drip of flour paste from his paw and made a face. {name} gave him a biscuit to clear his tongue. Science was good, but biscuits were better.",
     }},
   }},
};
constexpr int STORY_COUNT = sizeof STORIES / sizeof STORIES[0];
}  // namespace biscuit
