export default [
  {
    id: 'math-01', topic: 'math', title: 'How can two digits count anything?',
    pages: [
      'Binary uses only 0 and 1. Its place values double as you move left: 1, 2, 4, 8, 16. A 1 includes that value; a 0 leaves it out.',
      'So 1101 means 8 + 4 + 0 + 1: thirteen. The number has not changed, only its outfit. Computers store information using patterns of bits.',
    ],
    wonder: 'Invent two symbols for 0 and 1. What would a page of your binary writing look like?',
    source: { name: 'CS Unplugged · University of Canterbury', url: 'https://www.csunplugged.org/static/slides/en/binary-representation/binary-representation-speaker-notes.pdf' },
  },
  {
    id: 'math-02', topic: 'math', title: 'Why does one extra bit double the possibilities?',
    pages: [
      'Two bits have four patterns: 00, 01, 10, 11. Add a third bit and every old pattern gets two versions, one starting with 0 and one with 1.',
      'That makes eight patterns. Four bits make sixteen. Each extra yes-or-no choice doubles the possible messages, even before you decide what they mean.',
    ],
    wonder: 'Design a tiny flag using three squares, each dark or light. What messages could its eight patterns carry?',
    source: { name: 'CS Unplugged · University of Canterbury', url: 'https://classic.csunplugged.org/documents/books/english/unplugged-book-v1.pdf' },
  },
  {
    id: 'math-03', topic: 'math', title: 'Can an odd number reveal a hidden mistake?',
    pages: [
      'Arrange two-sided cards in a grid. Add a border so every row and column has an even number of dark cards. This rule is called even parity.',
      'Flip just one card: its row and column now have odd totals. Their crossing locates the change. Several flips can fool this simple error detector.',
    ],
    wonder: 'Draw a small parity grid. What patterns of changes could leave every row and column even?',
    source: { name: 'CS Unplugged · University of Canterbury', url: 'https://www.csunplugged.org/en/topics/error-detection-and-correction/parity-magic-junior/' },
  },
  {
    id: 'math-04', topic: 'math', title: 'How can a triangle count itself?',
    pages: [
      'Stack dots in rows of 1, 2, 3 and 4. You get a triangle of ten dots. Turn a second copy around and the pair fits into a 4-by-5 rectangle.',
      'The rectangle has twenty dots, so each triangle has half: ten. The same trick works for any number of rows. Two awkward shapes become one easy count.',
    ],
    wonder: 'Draw two five-row dot triangles in different colours. How could they share one rectangle?',
    source: { name: 'NRICH · University of Cambridge', url: 'https://nrich.maths.org/problems/handshakes?tab=teacher' },
  },
  {
    id: 'math-05', topic: 'math', title: 'Why do odd numbers build squares?',
    pages: [
      'Start with one dot. Add an L-shaped border of three dots to make a 2-by-2 square. Another border of five makes a 3-by-3 square.',
      'The borders keep growing by two: 3, 5, 7, 9. That is why 1 + 3 + 5 + 7 makes sixteen, a 4-by-4 square. A picture can be a mathematical argument.',
    ],
    wonder: 'Sketch a square growing one border at a time. Where does each new border gain its two extra dots?',
    source: { name: 'NRICH · University of Cambridge', url: 'https://nrich.maths.org/problems/picturing-square-numbers' },
  },
  {
    id: 'math-06', topic: 'math', title: 'Why does counting handshakes need a half?',
    pages: [
      'Six people each shake five other hands. Six times five gives thirty, but every handshake appears twice: once for each person involved.',
      'Halving thirty gives fifteen actual handshakes. Counting from two viewpoints is useful, provided you remember when both views describe the same thing.',
    ],
    wonder: 'Draw six people as dots and handshakes as lines. What else could the same picture represent?',
    source: { name: 'NRICH · University of Cambridge', url: 'https://nrich.maths.org/problems/handshakes?tab=teacher' },
  },
  {
    id: 'math-07', topic: 'math', title: 'Can a loop have only one side?',
    pages: [
      'Give a paper strip half a twist, then join its ends. You have made a Möbius strip. The twist connects what seemed to be two different sides.',
      'Trace a line along the middle without lifting your pencil. It travels over both original faces before returning. The finished loop has one continuous side.',
    ],
    wonder: 'Make a paper loop with tape and a half twist. How does tracing its edge compare with tracing its middle?',
    source: { name: 'NRICH · University of Cambridge', url: 'https://nrich.maths.org/making-maths-make-magic-circle' },
  },
  {
    id: 'math-08', topic: 'math', title: 'Why is the middle number so useful?',
    pages: [
      'Take three neighbours on the number line: 7, 8, 9. Move one from the largest to the smallest, and you get 8, 8, 8. The total stays twenty-four.',
      'So three consecutive whole numbers always add to three times the middle one. Five neighbours balance around their middle too. Symmetry does the work.',
    ],
    wonder: 'Draw five consecutive numbers as towers of squares. How could you rearrange them into equal towers?',
    source: { name: 'NRICH · University of Cambridge', url: 'https://nrich.maths.org/problems/three-neighbours' },
  },
  {
    id: 'animals-01', topic: 'animals', title: 'What is hidden inside a dog’s nose?',
    pages: [
      'A dog’s nose contains a maze of narrow passages. The folds fit a large scent-detecting surface into a small space. A sniff sends odour molecules inside.',
      'Molecules meet receptors that send signals to the brain. Researchers model these air currents to understand sniffing and help design artificial noses.',
    ],
    wonder: 'If Biscuit wrote a map using smells instead of colours, which places might deserve the biggest labels?',
    source: { name: 'Penn State University', url: 'https://www.psu.edu/news/research/story/scent-penn-state-engineers-search-better-artificial-nose' },
  },
  {
    id: 'animals-02', topic: 'animals', title: 'Can a dance give directions?',
    pages: [
      'A honeybee that finds flowers can perform a waggle dance in the hive. Its movements carry information about the direction and distance of the food.',
      'Other bees can use that information on their own journeys. Researchers film and decode dances to discover where colonies are gathering food.',
    ],
    wonder: 'Invent movements that could describe a route to a bookshelf. What information would your dance need?',
    source: { name: 'University of Sussex · LASI', url: 'https://www.sussex.ac.uk/lasi/sussexplan/dances' },
  },
  {
    id: 'animals-03', topic: 'animals', title: 'How does an octopus change its skin’s colour?',
    pages: [
      'An octopus has tiny pigment sacs in its skin, called chromatophores. Muscles stretch the sacs wider or let them shrink, changing how much colour shows.',
      'Nerves control the muscles, making patterns change rapidly. An octopus can also raise bumps on its skin. Its disguise can have texture as well as colour.',
    ],
    wonder: 'Design two octopus disguises on paper: one for pebbles and one for seaweed. What must change besides colour?',
    source: { name: 'Smithsonian Ocean', url: 'https://ocean.si.edu/ocean-life/invertebrates/how-octopuses-and-squids-change-color' },
  },
  {
    id: 'animals-04', topic: 'animals', title: 'How does a penguin wear air?',
    pages: [
      'Penguin feathers trap a layer of air next to the body. That trapped air helps insulate the bird from cold water. Its coat contains an invisible ingredient.',
      'Penguins spread oil from a gland near the tail over their feathers while preening. This helps the feather coat repel water and keep doing its job.',
    ],
    wonder: 'Design a fictional underwater coat for Biscuit. How would you keep its insulating layer from escaping?',
    source: { name: 'Smithsonian Ocean', url: 'https://ocean.si.edu/ocean-life/seabirds/penguins' },
  },
  {
    id: 'animals-05', topic: 'animals', title: 'How can an elephant call hide from human ears?',
    pages: [
      'Elephants communicate with deep rumbles. Parts of a rumble can be below the range humans can hear. Low pitch does not necessarily mean low volume.',
      'Low-frequency sounds travel well through forests, helping separated elephants stay in contact. A quiet landscape to human ears may contain elephant messages.',
    ],
    wonder: 'Imagine a sound map that includes sounds people cannot hear. Which familiar places might seem less quiet?',
    source: { name: 'Cornell University · Elephant Listening Project', url: 'https://www.birds.cornell.edu/ccb/elephant-listening-project/sound/' },
  },
  {
    id: 'animals-06', topic: 'animals', title: 'How does a platypus hunt with its eyes shut?',
    pages: [
      'A diving platypus closes its eyes, ears and nostrils while searching for food. Its bill contains sensors for pressure and electrical signals.',
      'The bill helps it find small prey on the river bottom. Scientists are still studying exactly how these signals work together. A strange bill, a serious tool.',
    ],
    wonder: 'Invent an explorer’s tool for muddy water. What could it measure when a camera would be little help?',
    source: { name: 'Australian Museum', url: 'https://australian.museum/learn/animals/mammals/platypus/' },
  },
  {
    id: 'animals-07', topic: 'animals', title: 'How can an echo reveal an insect?',
    pages: [
      'Many bats send out high-pitched calls and listen for the echoes. Sound bounces from nearby objects, including the insects a bat is hunting.',
      'Returning echoes help the bat locate its surroundings as it flies. This is echolocation: gathering information with sound. Bats can also see.',
    ],
    wonder: 'Imagine drawing a room from its echoes. Which details would be easier to discover than others?',
    source: { name: 'U.S. National Park Service', url: 'https://www.nps.gov/sagu/learn/nature/bats.htm' },
  },
  {
    id: 'animals-08', topic: 'animals', title: 'Is a bat’s wing a very unusual hand?',
    pages: [
      'A bat’s wing has greatly lengthened finger bones with thin skin stretched between them. Many movable joints let the wing bend and change shape.',
      'That flexible structure helps bats turn and manoeuvre in flight. A bat and a person share a familiar basic hand plan, put to remarkably different uses.',
    ],
    wonder: 'Sketch a bat wing beside your hand. Which parts seem familiar, and which proportions would need changing?',
    source: { name: 'U.S. National Park Service', url: 'https://home.nps.gov/subjects/bats/how-bats-fly.htm' },
  },
  {
    id: 'language-01', topic: 'language', title: 'Can a language have grammar you can see?',
    pages: [
      'American Sign Language, or ASL, is a complete natural language. Hands, face and body carry meaning. Its grammar differs from English grammar.',
      'It has its own rules for forming words and arranging them. A signed conversation can tell stories, argue ideas or make jokes.',
    ],
    wonder: 'What might a visual language let a storyteller show directly in the space around them?',
    source: { name: 'NIH · NIDCD', url: 'https://www.nidcd.nih.gov/health/american-sign-language' },
  },
  {
    id: 'language-02', topic: 'language', title: 'Is there one sign language for the whole world?',
    pages: [
      'There are many sign languages. American Sign Language and British Sign Language are distinct, even though English is spoken in both countries.',
      'Knowing one does not automatically mean understanding the other. Signed languages also have regional differences in signs and rhythm.',
    ],
    wonder: 'What could make a travelling signed storyteller’s performance easy to follow across language differences?',
    source: { name: 'NIH · NIDCD', url: 'https://www.nidcd.nih.gov/health/american-sign-language' },
  },
  {
    id: 'language-03', topic: 'language', title: 'Can eyebrows be part of a sentence?',
    pages: [
      'In ASL, facial expressions do grammatical work. Eyebrows, eyes and body position can help signal a question, alongside the hands.',
      'The face is carrying language, not merely showing a mood. Watching only a signer’s hands would leave out useful information.',
    ],
    wonder: 'How could a comic show the difference between a character wondering something and stating it?',
    source: { name: 'NIH · NIDCD', url: 'https://www.nidcd.nih.gov/health/american-sign-language' },
  },
  {
    id: 'language-04', topic: 'language', title: 'How can six dots hold a book?',
    pages: [
      'Braille uses patterns of raised dots that readers feel with their fingertips. A basic cell has six positions: two columns, each three dots high.',
      'Different patterns can represent letters, numbers and punctuation. Cells are small enough to feel with a fingertip, turning a page into a tactile text.',
    ],
    wonder: 'Invent a raised-dot label for a fictional object. What would make nearby symbols easy to tell apart?',
    source: { name: 'Library of Congress · NLS', url: 'https://www.loc.gov/nls/services-and-resources/informational-publications/about-braille/' },
  },
  {
    id: 'language-05', topic: 'language', title: 'How can a whole word fit in one small pattern?',
    pages: [
      'Braille does not always spell every word letter by letter. Contracted braille uses certain patterns for groups of letters or whole words.',
      'These contractions save space, helping keep raised-dot books less bulky. Readers learn the shared rules so a short pattern carries a longer meaning.',
    ],
    wonder: 'Invent three shortcuts for a message to Biscuit. How would you explain the rules so somebody else could read it?',
    source: { name: 'Library of Congress · NLS', url: 'https://www.loc.gov/nls/services-and-resources/informational-publications/about-braille/' },
  },
  {
    id: 'language-06', topic: 'language', title: 'How did three scripts unlock an ancient message?',
    pages: [
      'The Rosetta Stone repeats a decree in three scripts: Egyptian hieroglyphs, Egyptian Demotic and Greek. Three scripts, but two languages.',
      'Scholars could read the Greek. Comparing its message with the Egyptian writing helped them work out hieroglyphs. Repetition became a clue.',
    ],
    wonder: 'Invent a sign in two scripts, one known and one made up. What clues could help someone decipher your new script?',
    source: { name: 'British Museum', url: 'https://www.britishmuseum.org/blog/everything-you-ever-wanted-know-about-rosetta-stone' },
  },
  {
    id: 'language-07', topic: 'language', title: 'Can a whistle carry a conversation?',
    pages: [
      'On La Gomera in the Canary Islands, Silbo Gomero turns spoken Spanish into whistles. Changes in pitch and interruptions represent language sounds.',
      'Listeners learn to recover words from those whistle patterns. It is a way to carry messages, rather than a separate wordless tune.',
    ],
    wonder: 'Imagine a message travelling across a valley. What might a whistled version preserve, and what might be harder to distinguish?',
    source: { name: 'UNESCO · Intangible Cultural Heritage', url: 'https://ich.unesco.org/en/RL/whistled-language-of-the-island-of-la-gomera-canary-islands-the-silbo-gomero-00172' },
  },
  {
    id: 'language-08', topic: 'language', title: 'Why might one conversation use two languages?',
    pages: [
      'Multilingual speakers sometimes switch languages during a conversation, or even within a sentence. Linguists call this code-switching.',
      'A speaker may choose a phrase that fits especially well in another language. Switching can keep the conversation flowing when a word in one language is elusive.',
    ],
    wonder: 'Imagine two characters who share several languages. What might make one character change languages during their story?',
    source: { name: 'Cambridge University Press & Assessment', url: 'https://www.cambridgeenglish.org/news/view/how-english-teachers-can-crack-the-code/' },
  },
  {
    id: 'body-01', topic: 'body', title: 'How does a sound become something you hear?',
    pages: [
      'Sound waves make your eardrum vibrate. Three tiny bones pass those vibrations into the fluid-filled cochlea inside your inner ear.',
      'The moving fluid bends tiny projections on sensory cells. That helps create electrical signals which travel to the brain. A vibration becomes information.',
    ],
    wonder: 'Imagine designing a machine that notices a whisper. Which part would catch movement, and which would interpret it?',
    source: { name: 'NIH · NIDCD', url: 'https://www.nidcd.nih.gov/health/how-do-we-hear' },
  },
  {
    id: 'body-02', topic: 'body', title: 'Why does your inner ear contain tiny loops?',
    pages: [
      'Three fluid-filled loops in each inner ear help detect head rotation. They point in different directions, so they can report different kinds of turns.',
      'Moving fluid bends sensory structures. The brain combines these signals with information from eyes and body to help keep track of balance.',
    ],
    wonder: 'Sketch an imaginary explorer robot. Where would you put sensors so it could tell which way it was turning?',
    source: { name: 'NIH · NIDCD', url: 'https://www.nidcd.nih.gov/health/balance-disorders' },
  },
  {
    id: 'body-03', topic: 'body', title: 'Does an eye send pictures to the brain?',
    pages: [
      'The cornea and lens focus light onto the retina at the back of the eye. There, special cells called photoreceptors respond to light.',
      'They help turn light into electrical signals. The optic nerve carries signals to the brain, which turns that information into the images you see.',
    ],
    wonder: 'Draw the journey from a book’s page to a reader’s brain. Where does the information change form?',
    source: { name: 'NIH · National Eye Institute', url: 'https://www.nei.nih.gov/learn-about-eye-health/healthy-vision/how-eyes-work' },
  },
  {
    id: 'body-04', topic: 'body', title: 'How can a nose recognise so many smells?',
    pages: [
      'Odour molecules reach sensory cells high inside the nose. Different molecules can activate different combinations of receptors, which send signals to the brain.',
      'There need not be a separate receptor type for every smell. Combinations create many patterns, rather like a few musical notes making many tunes.',
    ],
    wonder: 'Invent a smell alphabet using a few imaginary signals. How could combinations describe a forest or an old book?',
    source: { name: 'NIH · NIDCD', url: 'https://www.nidcd.nih.gov/health/smell-disorders' },
  },
  {
    id: 'body-05', topic: 'body', title: 'Why does your nose help with flavour?',
    pages: [
      'Food aromas can reach the nose from behind: through a passage linking the throat and nose. Smell and taste work together to produce much of flavour.',
      'When a blocked nose keeps those aromas from reaching smell cells, familiar foods may seem bland. The tongue is only part of the flavour detective team.',
    ],
    wonder: 'Describe a favourite flavour without naming the food. Which words describe smell, and which describe texture?',
    source: { name: 'NIH · NIDCD', url: 'https://www.nidcd.nih.gov/health/smell-disorders' },
  },
  {
    id: 'body-06', topic: 'body', title: 'How does making more space pull air inside?',
    pages: [
      'A sheet of muscle called the diaphragm sits below your lungs. When it contracts, it moves downward and makes more room in your chest.',
      'Your lungs expand and air flows in. As the diaphragm relaxes, the lungs recoil and air flows out. Breathing changes space as well as moving air.',
    ],
    wonder: 'Draw two diagrams of the chest, one with the diaphragm higher and one lower. How would you show the changing space?',
    source: { name: 'NIH · NHLBI', url: 'https://www.nhlbi.nih.gov/health/lungs/breathing-benefits' },
  },
  {
    id: 'body-07', topic: 'body', title: 'Where does breathed-in oxygen meet the blood?',
    pages: [
      'Inside the lungs are tiny air sacs called alveoli, surrounded by small blood vessels. Oxygen crosses their thin walls into the blood.',
      'Red blood cells carry oxygen onward. Carbon dioxide travels the other way, from blood into the air sacs, ready to leave when you breathe out.',
    ],
    wonder: 'Imagine designing an exchange station for two kinds of cargo. How would you give both plenty of room to cross?',
    source: { name: 'NIH · NHLBI', url: 'https://www.nhlbi.nih.gov/health/lungs/breathing-benefits' },
  },
  {
    id: 'body-08', topic: 'body', title: 'Is a skeleton ever really finished?',
    pages: [
      'Bone is living tissue, constantly being remodelled. Some cells break down old bone while others build new bone. Your skeleton has a maintenance crew.',
      'The builders are called osteoblasts; the cells removing bone are osteoclasts. Both jobs matter as bones renew themselves and grow.',
    ],
    wonder: 'Invent a building that can replace its own worn parts while people still use it. How would its crews coordinate?',
    source: { name: 'NIH · NIAMS', url: 'https://www.niams.nih.gov/health-topics/educational-resources/health-lesson-learning-about-bones' },
  },
];
