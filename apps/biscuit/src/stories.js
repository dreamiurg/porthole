// Original branching adventures. Short pages fit the device; the stories reward close reading.
export const stories = [
  {
    id: 'moon', title: 'The Moon Biscuit', symbol: '☾', subtitle: 'A midnight adventure', unlockDay: 1,
    pages: [
      'At midnight, a perfect wedge vanished from every moon biscuit in Pepper’s bakery window. The glass was locked. By morning, golden crumbs lay outside, on the wrong side of the glass.',
      'Pepper had opened the shop only yesterday. “Mice,” she declared. Biscuit tilted his head at a crescent. “Such tidy bites! My crumbs go everywhere. Do these mice carry rulers?”',
      'Ivy spread flour beneath the cabinet before closing time. At midnight the clock struck thirteen. Something clicked inside the wall. Another row of moons acquired identical missing pieces.',
      'There were no tracks in the flour. The crumbs outside smelled of lemon; the biscuits inside smelled of ginger. Biscuit nudged Ivy’s hand. “Two smells! Come sniff. I’ll save you a spot.”',
      'Pepper weighed the tray. It weighed exactly what it had before. “So the missing bits haven’t gone far,” Ivy said. Biscuit looked underneath. The cabinet had an unusually thick wooden floor.',
      'A thin brass wire ran from the cabinet into the wall. Upstairs, they found a shuttered skylight, a warm chimney, and a photograph of the shop wearing bunting: THE EDIBLE OBSERVATORY.',
      'In the photograph, a woman pointed at a crescent while children peered through a telescope. Pepper recognized her grandmother. “She was a baker,” she said. “Mostly. Apparently not entirely.”',
      'Behind the clock was a second dial with twenty-nine little marks. Its handle was sticky. Pepper had wound it while cleaning. “I thought it was the bell. The bell was also sticky.”',
      'The chimney breathed out warm, lemon-scented air. Below them, a weight rattled inside the cabinet. Two trails led away from the clock: one toward the roof, one beneath the display.',
      '“We could follow the wire up and see what it turns,” Ivy said. “Or find out where the pieces are hiding.” Biscuit approved of both plans, especially the one containing pieces.',
    ],
    prompt: 'Which part of the old observatory shall we investigate?',
    choices: [
      {
        label: 'Follow the wire upstairs',
        ending: [
          'The wire lifted a shutter over a telescope. A metal model beside it showed how much of the moon was lit. Each night, the same mechanism lowered a different cutter into the cabinet.',
          'Its old clock was running too fast: tonight’s crescents belonged to next week. Ivy found a marked lever that slowed the model. Pepper watched the real moon and adjusted the dial.',
          'From the roof they spotted lemon crumbs falling from the next chimney. The night baker next door was cooling buns. Two mysteries had been walking around in the same crumb costume.',
          'Pepper reopened the Edible Observatory. Visitors compared biscuits with the moon; a hidden drawer supplied the cuttings. Biscuit watched beside Ivy, with star-shaped crumbs on his nose.',
        ],
      },
      {
        label: 'Open the cabinet’s base',
        ending: [
          'Ivy found a latch beneath the thick floor. A drawer slid out, full of ginger wedges and a floury notebook. Biscuit inhaled. “Excellent. The missing moon has been keeping a diary.”',
          'The notebook explained the clock-driven cutters, then recorded thirty disastrous recipes. One biscuit bent a spoon. Another floated. On the last page: USE THE OFFCUTS. THEY MAKE STARS.',
          'Pepper fitted a star cutter into the spare slot. At the next click, the cabinet produced a crescent and three tiny stars. Nothing was missing; the tray had been hiding half the show.',
          'They traced the outdoor crumbs to lemon buns cooling next door. Pepper added her own recipe to the notebook. Biscuit tried to help turn the page, leaving its first floury paw print.',
        ],
      },
    ],
  },
  {
    id: 'library', title: 'The Secret Library', symbol: '✿', subtitle: 'A mystery between the shelves', unlockDay: 1,
    pages: [
      'The atlas showed a river through the middle of Ivy’s street. The newer atlas showed no river at all. Neither mentioned the sound of running water behind the dictionary shelf.',
      'Biscuit put his nose against the wood. “Damp stone. Old paper. Someone’s cheese sandwich.” Ivy pulled out a dictionary. A green door appeared behind it, with a brass card slot.',
      'A mouse librarian opened the door. “You’re early,” she said. “For what?” asked Ivy. The mouse consulted a card. “Your book return. It was due eighty-three years before you borrowed it.”',
      'The card bore Ivy’s address, but another name. Beyond the door, shelves disappeared into the dark. Their labels were peculiar: LOST ROADS, UNBUILT BRIDGES, PLACES PEOPLE ALMOST REMEMBER.',
      '“We lend books to houses,” the mouse explained. “People move. Addresses usually stay.” She showed them the missing volume: a mapmaker’s notebook borrowed by a former resident of Ivy’s home.',
      'Back upstairs, Ivy found a folded map tucked inside the old atlas. Half the river was blue. Half was dotted. At the edge, someone had written: THE WATER KNOWS THE WAY HOME.',
      'Biscuit noticed that the map smelled of the same damp stone as the secret library. “Either the river is very good at reading,” he said, “or we have found its entrance.”',
      'The mouse brought two other records: a plan of the old river tunnels, and a register of letters sent to Ivy’s address. Each mentioned a person called Kit, but neither gave a surname.',
      'The tunnel plan ended under the market. The letters ended with a parcel delivered to the library, never opened because nobody could agree which shelf an unfinished map belonged on.',
      'Ivy spread the records across the desk. One route followed the missing river. The other followed its mapmaker. Biscuit rescued the cheese sandwich from beneath the oldest document.',
    ],
    prompt: 'Shall we follow the water, or the person who mapped it?',
    choices: [
      {
        label: 'Trace the hidden river',
        ending: [
          'The librarian led them down a dry walkway beside the tunnel. Under the market, the river still flowed. Kit’s dotted line marked a channel covered when the town built its new streets.',
          'On the wall, Kit had carved measurements beside a tiny kingfisher. Farther along, a real kingfisher watched from a grating. Above it, daylight shone through a forgotten courtyard.',
          'They returned with a page from a sealed survey box. It described a garden beside the river, never built. In the unopened parcel, the mouse found the rest of Kit’s notebook.',
          'Ivy drew the courtyard onto the town map. Soon neighbors began opening its locked gate. Biscuit chose a bench near the water. “Room for both of us,” he said, resting his chin on Ivy’s knee.',
        ],
      },
      {
        label: 'Read the mapmaker’s letters',
        ending: [
          'The letters were from Kit to her brother, who had moved inland. Each described a sound: the ferry bell, rain beneath a bridge, pebbles rolling where the river grew shallow.',
          'Inside the unopened parcel lay the notebook and a second map. This one had no street names. Instead, little symbols marked places where her brother could recognize home by listening.',
          '“Two maps of one town,” Ivy said. “They answer different questions.” The mouse made a new shelf: WAYS OF FINDING HOME. Neither atlas had room for the memory of a ferry bell.',
          'Ivy began a sound map of the neighborhood. Biscuit contributed the bakery door, the squeaky park gate, and a detailed account of his dinner bowl. The river received its own page.',
        ],
      },
    ],
  },
  {
    id: 'dragon', title: 'A Dragon’s Bedtime', symbol: '✧', subtitle: 'A tale for sleepy explorers', unlockDay: 1,
    pages: [
      'A dragon in purple pajamas arrived carrying six scorched pillows and a book tied shut with string. “I have tried warm milk,” she said. “The saucepan is now part of the ceiling.”',
      'Her trouble had begun with a bedtime story. Whenever she reached the final sentence, another page appeared. She had been reading for three nights. The hero still hadn’t reached the island.',
      'Ivy opened the book carefully. A sailor stood on a ship’s deck. Below the illustration: At last, the island came into view, but—. A new page slid out. On it, a storm was gathering.',
      'Biscuit sat on the loose page. “Would a cuddle help?” The book wriggled underneath him. The dragon apologized to it, which produced a page about a shipwrecked apology.',
      'Inside the cover they found a label: STORIES FOR TWO VOICES. The dragon frowned. “I have only one voice. A loud one, admittedly.” Ivy noticed a second ink color in the margins.',
      'Those notes belonged to the sailor: I could use a harbor. A smaller storm would also do. The printed narrator answered every request with another difficulty. Nobody had asked the island.',
      'Ivy held the page near a lamp. Beneath the waves, pale words appeared: I HAVE BEEN HERE ALL ALONG. There was no picture beside them, just the outline of an enormous sleeping turtle.',
      '“That island moves,” Ivy said. “The sailor is chasing a turtle.” The dragon stared. Biscuit considered this. “I have chased worse things. One of them was my own tail, so I won’t judge.”',
      'The book offered two blank speech bubbles. One belonged to the sailor. The other rose from the turtle. Whatever they wrote would give the story its missing second voice.',
      'The dragon uncapped a pen. “Do we ask where the island is going, or tell the sailor how to stop chasing it?” Outside, dawn was already putting a pale edge on the rooftops.',
    ],
    prompt: 'Whose voice shall we add to the story?',
    choices: [
      {
        label: 'Let the island answer',
        ending: [
          'Ivy wrote in the turtle’s bubble: I AM GOING TO THE WARM CURRENT. YOU MAY COME, BUT PLEASE STOP DROPPING ANCHORS ON MY BACK. The dragon read it in a magnificent underwater rumble.',
          'The sailor hauled up his anchor. His ship drifted beside the turtle, and the endless storm became a passing shower. The narrator tried to add a whirlpool. The turtle sneezed it away.',
          'A last picture appeared: a ship and a moving island beneath the same moon. The final sentence stayed final. The dragon shut the book and looked around for a pillow that still existed.',
          'Biscuit nudged his cushion toward her. By breakfast, the dragon slept with one claw marking the page. Once in a while she murmured directions. The warm current seemed to need a left turn.',
        ],
      },
      {
        label: 'Let the sailor change course',
        ending: [
          'The dragon wrote: I WANT TO EXPLORE, NOT ARRIVE. Ivy read the words aloud. The sailor looked up from his map, then turned the ship toward a cluster of lights far behind him.',
          'They were other ships. Each carried a traveler chasing the same moving island. The sailor invited them to tie their boats together for supper. A floating harbor formed without a shore.',
          'The last illustration showed lanterns, borrowed chairs, and maps spread across three decks. The narrator left room for another voyage, but stopped inventing reasons to delay supper.',
          'The dragon finally slept. Biscuit borrowed the book the following night and curled up beside Ivy. After one chapter he yawned. “Save some adventure for tomorrow.” The cover stayed quiet.',
        ],
      },
    ],
  },
  {
    id: 'seed', title: 'The Sleepy Seed', symbol: '❀', subtitle: 'Some things take their time', unlockDay: 2,
    pages: [
      'The seed packet came from a botanist’s greenhouse. On it she had written SLEEPY SUNFLOWERS, then DO NOT MISTAKE STILLNESS FOR AN ANSWER. “Sleepy flowers?” Biscuit said. “We’ll get along.”',
      'Ivy planted three seeds in clear pots with paper sleeves. A week later, nothing had appeared. She checked the water, the light, and the packet. Biscuit checked for anything resembling lunch.',
      'In the greenhouse, ordinary seedlings leaned toward the windows. The three empty pots sat under an old shade. Its cord ran through a row of pulleys to a small wooden weather vane.',
      '“Perhaps the seeds are too old,” Ivy said. Biscuit sniffed the pots. “Maybe they’re having a very long nap.” He sniffed again, and his tail stirred. “Something in there smells alive.”',
      'Ivy slid off a paper sleeve. A white root pressed against the clear wall, then curved sideways. The seed had sprouted underground. Above it, the soil remained stubbornly undisturbed.',
      'A notebook lay beneath the bench. Most pages recorded ordinary things: dates, rain, soil temperature. One page showed the greenhouse before a shed had been built outside its eastern wall.',
      'The shed now blocked the morning sun. Worse, its gutter dripped into the shade’s pulley. The swollen wood had jammed the shade shut. Someone had carefully watered a patch of permanent dusk.',
      'They freed the cord. A band of sunlight crossed the pots. Under the bench, another label appeared: THREE WINDOWS, THREE POSSIBLE GARDENS. The botanist had drawn a different view beside each.',
      'The south window faced a bare courtyard. The west looked onto a high wall. The third view showed the inside of a pot, full of curling roots. There was room to try something new.',
      '“We could move the plants outside and see what follows them,” Ivy said. “Or keep a root window and watch what happens underneath.” Biscuit preferred any garden that admitted dogs.',
    ],
    prompt: 'What sort of garden shall we make with the newly sprouted seeds?',
    choices: [
      {
        label: 'Plant a courtyard lookout',
        ending: [
          'They planted two seedlings by the sunny courtyard and kept the third indoors until it grew stronger. Weeks passed. The stems climbed. Biscuit used one as an unreliable measure of his height.',
          'When the flowers opened, bees appeared first. Then came finches, arguing over seeds before any were ready. Ivy added arrival dates to the botanist’s notebook, beneath the weather records.',
          'One afternoon, a neighbor brought a photograph. The courtyard had once been full of sunflowers. The finches were returning to a place their own families had visited before the shed existed.',
          'Ivy left some seed heads standing through autumn. A finch landed while Biscuit watched from his cushion. “Our flowers made friends!” The notebook gained a page called UNEXPECTED VISITORS.',
        ],
      },
      {
        label: 'Build an underground window',
        ending: [
          'They kept one plant in a deep clear container, covered except when Ivy made a sketch. The others moved into roomy pots. Biscuit napped at her feet, waking when her pencil tickled his ear.',
          'The sketches revealed a surprise: the roots changed direction toward the wetter side. Above the soil, the stem turned toward light. One plant was following two different maps at once.',
          'Ivy compared her drawings with the botanist’s final page. The unfinished design was a planter with a removable dark panel, a window into a place gardeners usually had to guess about.',
          'At the next garden gathering, visitors looked below the leaves. Biscuit dozed beside the display, paws twitching in a dream. Ivy made him a sign too: DOG RESTING. SEEDS BUSY.',
        ],
      },
    ],
  },
  {
    id: 'cloud', title: 'The Cloud Collector', symbol: '☁', subtitle: 'An adventure in looking up', unlockDay: 3,
    pages: [
      'Professor Vale kept clouds in jars. Small ones, fortunately. His shelves held a sulky drizzle, a magnificent thunderhead the size of a cabbage, and a jar labeled NOTHING MUCH, PROBABLY.',
      'The last jar had vanished. Vale was upset. “Someone took my most unusual specimen!” Biscuit tipped his head at the label. “What was so special about it? Even little things can surprise you.”',
      'The latch was intact. No shelf was empty. In fact, there was one extra jar, full of perfectly ordinary fog. Vale insisted that it had not been there when he went to bed.',
      'Ivy noticed droplets on the outside of the fog jar. Every other jar was dry. A ribbon tied to its lid pointed toward the open window, although the curtains hung perfectly still.',
      'The professor brought out his notebook. NOTHING MUCH had come from a clear patch above the harbor, between two banks of cloud. Its temperature had changed whenever a ship passed below it.',
      '“You collected a gap,” Ivy said. Vale straightened. “A very particular gap.” Biscuit sneezed toward the fog. The ribbon swung toward him. He retreated. “It appears to have opinions.”',
      'At the window, they could see a narrow stream of fog moving uphill against the breeze. It curled around chimneys, crossed the square, and disappeared behind the shuttered old conservatory.',
      'A delivery label on the strange jar read RETURN TO SOURCE. Beneath it, in Vale’s own handwriting: DO NOT OPEN BESIDE ANOTHER SPECIMEN. “I remember writing that,” he said. “Not why.”',
      'The absent gap might be pulling fog from wherever it had escaped. Or the new jar might be pushing its weather into town. Ivy marked the fog’s direction on the window with her finger.',
      'They could follow the moving trail uphill, or compare the jars in the professor’s sheltered testing cabinet. Biscuit tucked his nose under Ivy’s hand. “My nose would like a little break.”',
    ],
    prompt: 'Shall we follow the wandering weather, or work out what the jars are doing?',
    choices: [
      {
        label: 'Follow the uphill fog',
        ending: [
          'Inside the conservatory, the missing jar stood open beside a dry fern. The caretaker had borrowed it, believing its label promised nothing much. Now every leaf wore a necklace of water.',
          'The gap was drawing in the town’s fog, then releasing it as a gentle mist. It wasn’t empty; it was a space that wanted filling. The fog jar on Vale’s shelf was its packed lid.',
          'Vale arrived carrying the lid and three pages of objections. He stopped beside a fern unfurling its first new frond. Together, they adjusted the opening until the wandering fog thinned away.',
          'The conservatory reopened as a cloud garden. Vale kept careful weather records. Biscuit greeted visitors with a wag and a towel. He loved the ferns, but wet paws still felt like wet paws.',
        ],
      },
      {
        label: 'Compare the jars safely',
        ending: [
          'Inside the testing cabinet, Ivy turned the fog jar upside down. Its label slipped, revealing a brass thread around the base. It wasn’t another jar at all. It was the missing jar’s lid.',
          'Vale had built a lid that expanded to hold escaping weather, then forgotten to note it on the shelf. The caretaker had borrowed the open jar for a fern in the old conservatory.',
          'They reunited the two pieces. The uphill fog stopped at once. Vale then made a smaller version: a lid that caught morning mist and released water slowly around the caretaker’s plants.',
          'The new label read WEATHER TRANSFER LID. Biscuit inspected it. “Better,” he said, “although I would add WHICH END OPENS.” The professor considered this, then fetched another label.',
        ],
      },
    ],
  },
  {
    id: 'lighthouse', title: 'A Light for Lost Paws', symbol: '✦', subtitle: 'A little light goes a long way', unlockDay: 5,
    pages: [
      'The lighthouse flashed three times. Somewhere in the fog, another light answered twice. Captain Mara closed her logbook. “That lamp should have been taken down forty years ago,” she said.',
      'She had another problem: Pip, the harbor dog, had vanished during his evening rounds. His barking seemed to come from the old boathouse, then from the opposite side of the harbor.',
      'Biscuit listened at the lighthouse door. The second bark was fainter, but identical. His ears perked up. “Two Pips? Or is the harbor playing copycat? Listen, it’s the very same woof!”',
      'Mara spread a chart on the table. A stone wall faced the boathouse across the water. Ivy traced the route a sound might take. The echo explained two dogs, but not the extra light.',
      'An old photograph showed a low signal lamp beside the boathouse. Its window faced the harbor through a narrow arch. In the new chart, that arch had been bricked up after a storm.',
      'Yet something still flashed there: two short bursts, then a pause. Mara checked the lighthouse controls. Nothing was broken. The reply arrived only when the beam swept across the boathouse.',
      'Pip had left his rounds notebook at the dock office. His last entry was a drawing: a paw, a little box, and the same arch from the photograph. He had circled the box twice.',
      '“He writes better pictures than I do,” Biscuit said. Ivy noticed that the box had a handle. Mara recognized it as the harbor’s missing inspection lantern, last seen beside a repair cart.',
      'There was a dry service path to the boathouse, safely away from the water. Mara took its key. Ivy could help her locate Pip by comparing the flashes, or by timing the answering barks.',
      'The fog made everything look like a half-erased drawing. Biscuit stayed close to Ivy and Mara. “Let’s find Pip,” he said. “If he’s missed supper, he can share mine.” His tail gave a wag.',
    ],
    prompt: 'Which signal shall we use to find Pip with Captain Mara?',
    choices: [
      {
        label: 'Map the returning light',
        ending: [
          'Ivy watched three sweeps of the beam while Mara checked the chart. The double flash came from two tiny windows above the blocked arch. They followed the service path to the old lamp room.',
          'Pip was inside, perfectly comfortable but locked behind a swollen door. Beside him stood the missing lantern. He had found it while investigating a glint and barked when the door stuck.',
          'Two glass prisms in the abandoned lamp split the lighthouse beam, sending the mysterious reply. Mara freed Pip, then held the prisms against a wall. Two miniature rainbows opened like fans.',
          'The old lamp became a display at the harbor museum. Pip contributed his drawing to the label. Biscuit suggested an addition: THIS DOOR STICKS. Nobody objected to that bit of history.',
        ],
      },
      {
        label: 'Listen between the echoes',
        ending: [
          'Biscuit barked once. Pip answered, followed by the fainter echo. As they walked with Mara, the first reply grew clearer. Ivy ignored the copy and listened for the scrape between Pip’s barks.',
          'It came from the old lamp room. Pip was nudging a lantern along the floor toward the stuck door. Mara opened it with her service key and a determined shove. Pip emerged carrying the handle.',
          'Above him, two glass prisms caught the lighthouse beam. That explained the flashes. The scraping supplied the missing clue: unlike the bark, its quiet sound had not crossed the harbor twice.',
          'Mara added the echo to her harbor guide so visitors would know why sounds changed places. Pip resumed his rounds with Biscuit. At the dock café, two wagging tails arrived before Ivy did.',
        ],
      },
    ],
  },
  {
    id: 'comet', title: 'The Comet’s Letter', symbol: '★', subtitle: 'Friends, however far apart', unlockDay: 7,
    pages: [
      'The silver envelope arrived inside a library book. It said WHOEVER FINDS THIS NEXT, and its stamp showed a comet that looked as though it had missed an appointment.',
      'Inside was a letter: I am passing your world again. Last time, a girl in a red coat waved from the hill. I promised to bring her news. Please tell me where she has gone.',
      'There was no date, only a drawing of the stars. Ivy recognized the hill, but a windmill stood where the observatory stood now. Biscuit sniffed the paper. “Old. Also faintly electrical.”',
      'At the observatory, Keeper Lin compared the drawing with her records. The comet had last passed the town seventy-six years earlier. The girl in the red coat would be considerably older now.',
      '“Does it know how long it’s been?” Ivy asked. Lin showed them a note in an old log: VISITOR THINKS ONE ORBIT IS A SHORT ERRAND. Biscuit leaned into Ivy. That was a long time between cuddles.',
      'The letter described ice mountains, a moon with an underground sea, and a rock that looked exactly like a turnip. At the bottom: I kept the pebble she gave me. It helps me remember home.',
      'A photograph in the log showed three children beside the windmill. One wore a red coat. None was named. On the back, someone had drawn a star inside a square, the mark of an old bookbinder.',
      'Lin found a timetable for tonight’s passing. The comet would be visible from the hill for a few minutes. The bookbinder’s shop, now a repair shop, stood at the other end of town.',
      'They could follow the girl’s trail before dusk, or prepare a reply and search the records afterward. Lin offered her signal mirror for either plan.',
      '“Let’s tell it we’re glad it came back,” Biscuit said. Ivy folded the letter along its worn creases. Above the clouds, a traveler was returning to a place that had kept changing.',
    ],
    prompt: 'Shall we find the old friend first, or make sure the comet receives a reply?',
    choices: [
      {
        label: 'Follow the bookbinder’s mark',
        ending: [
          'The repairer recognized the photograph. Her grandmother was the girl in red, and she still lived above the shop. Mrs. Rowan opened the letter, then laughed at the turnip-shaped rock.',
          '“I asked him to find one,” she said. From a drawer she took a smooth stone with a hollow in it. It had once fitted around another pebble, the one now traveling beyond the planets.',
          'They reached the hill together. Lin signaled with the mirror while Mrs. Rowan held up her stone. A silver speck brightened overhead. The reply arrived as a new line across the old letter.',
          'YOU HAVE CHANGED, it said. SO HAVE I. Mrs. Rowan began describing her seventy-six years. Biscuit settled beside her. This, he suspected, would be a story requiring more than one biscuit.',
        ],
      },
      {
        label: 'Send news from the hill',
        ending: [
          'Ivy wrote that the windmill was now an observatory, the town had grown, and they were looking for the girl. She drew Biscuit too. “Bigger ears, please! I want it to know me when it visits.”',
          'Lin signaled at dusk. New words appeared beneath Ivy’s message: I THOUGHT I WAS ONLY A LITTLE LATE. She answered that the letter had arrived, and there was still news worth sending.',
          'The next morning they found Mrs. Rowan through the bookbinder’s mark. She brought old photographs to the observatory. They began a town journal for a reader with a very long route.',
          'A small silver margin remained in the comet’s letter. Every few nights, another reply appeared there. Biscuit sent a drawing of a turnip and a muddy paw print. “So it gets a hello from me.”',
        ],
      },
    ],
  },
];
