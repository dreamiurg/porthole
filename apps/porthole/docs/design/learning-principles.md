# Learning principles for Porthole's games

> 2026-11-16: the puzzles and Little reads described here as Biscuit's moved to Pets Club (`games/pets-club/learn.cpp`,
> its Play menu); Biscuit keeps its stories and discoveries. The plan below is unchanged.

Porthole is built for two kids: one at the start of US first grade (about 6) and one at the start of third grade
(about 8). Several games teach on purpose (Biscuit's puzzles and stories, Times Cards). This page is the guidance for
choosing and shaping a learning mechanic: what the research supports, how strongly, and what that means on a round
touchscreen with no sound. Read it before proposing or building any mechanic that is meant to teach.

The research summary was compiled on 2026-11-15 from the sources listed at the end. Confidence is ours: **strong** is
a replicated finding or meta-analysis, **moderate** is a few studies or an extrapolation across ages, and **weak** is
theory, one small study or advocacy. Where a number comes from a secondary source, the table says so.

## The principles

1. **Practise the material itself, not general "brain" skills.** Working-memory and attention training improves the
   trained task and does not transfer to arithmetic or reading (far transfer about d = 0.05 in Melby-Lervåg, Redick
   & Hulme 2016, 145 comparisons). No memory or attention mini-games for their own sake. *Strong.*
2. **Pitch difficulty so the kid is right most of the time, but not always: aim at about 75 to 85% correct.** The
   "85% rule" (Wilson et al. 2019) is derived from models of binary learning only, not from children. The child
   evidence is Math Garden, an adaptive maths practice system that targets 0.75 (Klinkenberg et al. 2011), and Jansen
   et al. 2013 (grades 3 to 6), where a higher success rate meant more problems attempted and similar gains. Adapt
   per kid and per skill; never lock a kid to an age-based level. *Moderate.*
3. **Make the kid recall the answer, then show it.** Retrieval practice beats re-reading for fourth graders
   (d = 0.48 to 0.64, Karpicke, Blunt & Smith 2016), but only while success stays reasonably high; at about 8% success
   it did nothing. Hints (a first letter, fewer choices) are the way to keep a hard item retrievable. *Strong overall,
   moderate at age 6.*
4. **Bring old material back over days (spaced repetition).** Distributed practice and practice testing are the two
   techniques rated high-utility across ages (Dunlosky et al. 2013). A missed item comes back the next day or two, a
   known one at growing intervals. Being away must punish nothing: an overdue item is simply due. *Strong in general,
   moderate for young children's maths.*
5. **Mix problem types once each is introduced (interleaving).** After a short block on a new type, mix types:
   addition with subtraction, several tables, multiplication with its division facts. Accuracy inside the session
   drops, and that is expected; retention a day later is much better (77% vs 38%, fourth graders, Taylor & Rohrer 2010,
   figures via a secondary source). *Moderate: one elementary study, the rest seventh grade and adults.*
6. **Judge learning on the next day, not inside the round.** The right signal for adaptation and for us is how an
   item does on its first try of a later day, not the round's score. *Strong.*
7. **Number sense comes from placing numbers on a line.** Linear number board games close gaps in number-line
   estimation in four 15-minute sessions (Siegler & Ramani 2008, preschoolers), and only when the child counts on from
   the current square, not from 1 (Laski & Siegler 2013, kindergarten). Fraction magnitude on a line predicts maths
   achievement (Siegler et al. 2011) and drives fraction interventions (Fuchs et al. 2016, fourth grade). For the
   6-year-old: placing and counting on along 0 to 20 and 0 to 100. For the 8-year-old: fractions with denominators 2,
   3, 4, 6 and 8 as points on 0 to 1 (Common Core 3.NF.2). *Moderate.*
8. **Build fact fluency on strategies, and keep time pressure invisible.** Grade 1 expects fluency within 10 by
   strategies such as making ten (1.OA.6); grade 3 expects all single-digit products and their divisions by the end of
   the year (3.OA.7). Response time may feed the adaptation silently, but no visible timer and no penalty for being
   slow: time pressure worsens strategy choice, most for anxious kids (Caviola et al. 2017). The claim that timed tests
   *cause* anxiety is advocacy (Boaler 2015), not evidence. *Moderate.*
9. **After a mistake, show the right answer and a short "how".** "Wrong" alone teaches almost nothing; the correct
   answer plus a brief explanation does (Shute 2008). Praise the work, not the kid, and keep it short. Kids who already
   know a topic need less feedback (Fyfe et al. 2012, grades 2 and 3). *Strong for the first part, weak for the last.*
10. **Rewards: progress the kid can see and spend, not pay per answer.** Expected tangible rewards for doing a task
    lower free choice of it (d = -0.36; children -0.43 for rewards just for taking part, Deci, Koestner & Ryan 1999),
    while gamification helps learning modestly (g = 0.49 cognitive, Sailer & Homner 2020) through story and
    cooperation. So stars are honest information about progress ("you learned 3 new facts"), paid in round-sized
    lumps, spent on the pet's world; never a leaderboard or a comparison between siblings. An honest "I got it wrong"
    must never cost stars. *Moderate.*
11. **Reading for a first grader is decoding first.** Systematic phonics helps most in kindergarten and first grade,
    and in first grade lifts comprehension too (National Reading Panel 2000); reading is decoding times language
    comprehension (Gough & Tunmer 1986). Text for the 6-year-old should use letter patterns already met (decodable
    text; immediate accuracy gains, long-term effect unproven: Cheatham & Allor 2012), short, with a comprehension
    question. With no sound we cannot teach phonics properly; "fill in the missing letter" is spelling, and there is no
    direct evidence for it. *Strong for phonics in general, weak for our format.*
12. **Short and frequent beats long and rare.** A round should fit in a couple of minutes and invite a return
    tomorrow (spacing; Siegler's effective sessions were 15 minutes). This also fits the device's screen-time limits.
    *Moderate.*

## Choosing the next mechanic

A new learning mechanic should pass these checks. Say in its PR which principles it rests on.

- **What does it teach, and to which grade standard?** Name the skill and the Common Core code (for example 1.OA.6,
  3.OA.7, 3.NF.2, 2.MD.8). Content below the kid's grade is review, not the main course.
- **Does it adapt?** It keeps a level per kid and per skill and moves it toward 75 to 85% correct (principle 2).
- **Does the kid retrieve, and does a miss come back?** Recall first, the answer and a short "how" after a miss, and
  the missed item scheduled for a later day (principles 3, 4, 9).
- **Does it mix?** Once two types are known, rounds mix them (principle 5).
- **Is the game part the reason to come back, not the payment?** Stars as visible progress, spent in the pet's world;
  no per-answer pay, no timers, no comparisons (principles 8, 10).
- **No memory or attention drills for their own sake** (principle 1).

## What this means for the games now

- **Biscuit's puzzles** are fixed-level and random: no adaptation, no spaced review of misses, no "how" after a
  miss, and the times tables stop at 6. The plan in `adaptive-practice.md` fixes this.
- **Times Cards** already follows principles 2 to 4 and 10 (Leitner boxes, a pace that adapts toward about 85%, a
  missed card shown again in the round and the next day, two stars per finished round whatever the grades). Open
  points: division facts (3.OA.7), and self-grading (Easy, Hard, Wrong) relies on a 6-year-old judging their own
  recall honestly, which this research does not cover.
- **Stories** are long for a first grader (kid playtest, 2026-11-15). **Little reads** (Biscuit's Library, first row)
  add short passages with one question, five levels from decodable first-grade text to third-grade inference, a
  3-up/1-down level per kid and the line that holds the answer after a miss (principles 2, 3, 9, 11). Open: a missed
  passage is not scheduled for a later day (principle 4); it only comes back by chance at its level.

## Sources

Read in full or on the publisher's page unless marked *(abstract only)* or *(secondary)*.

- Wilson, Shenhav, Straccia & Cohen (2019). The Eighty Five Percent Rule for optimal learning. *Nature
  Communications* 10:4646. https://escholarship.org/content/qt0v22s4g4/qt0v22s4g4.pdf
- Klinkenberg, Straatemeier & van der Maas (2011). Computer adaptive practice of Maths ability using a new item
  response model for on the fly ability and difficulty estimation. *Computers & Education* 57(2):1813-1824.
  https://dare.uva.nl/id/de65bb65-d198-4dbc-ac7b-bc25ca23e627
- Jansen et al. (2013). The influence of experiencing success in math on math anxiety, perceived math competence, and
  math performance. *Learning and Individual Differences* 24:190-197 *(abstract only)*.
  https://dare.uva.nl/id/789fc422-4935-49ca-bcba-803239522951
- Melby-Lervåg & Hulme (2013). Is working memory training effective? A meta-analytic review. *Developmental
  Psychology* 49(2):270-291. https://library.scottbarrykaufman.com/uploads/2013/02/Melby-Lervag-Hulme-20131.pdf
- Melby-Lervåg, Redick & Hulme (2016). Working memory training does not improve performance on measures of
  intelligence or other measures of "far transfer". *Perspectives on Psychological Science* 11(4):512-534.
  https://discovery-pp.ucl.ac.uk/10120081/1/1745691616635612.pdf
- Dunlosky, Rawson, Marsh, Nathan & Willingham (2013). Improving students' learning with effective learning
  techniques. *Psychological Science in the Public Interest* 14(1):4-58. https://scholars.duke.edu/publication/954654
- Karpicke, Blunt & Smith (2016). Retrieval-based learning: positive effects of retrieval practice in elementary
  school children. *Frontiers in Psychology* 7:350. https://www.frontiersin.org/articles/10.3389/fpsyg.2016.00350/full
- Rohrer & Pashler (2010). Recent research on human learning challenges conventional instructional strategies.
  *Educational Researcher* 39(5) *(secondary for the child spacing studies)*.
  https://home.cs.colorado.edu/~mozer/Teaching/syllabi/TopicsInCognitiveScienceFall2011/RohrerPashler2010ER.pdf
- Taylor & Rohrer (2010). The effects of interleaved practice. *Applied Cognitive Psychology* 24(6):837-848
  *(figures secondary, via Agarwal & Agostinelli, AFT 2020)*. https://digitalcommons.usf.edu/psy_facpub/1760/
- Rohrer & Taylor (2007). The shuffling of mathematics problems improves learning. *Instructional Science*
  35:481-498. https://digitalcommons.usf.edu/psy_facpub/1767/
- Rohrer, Dedrick & Stershic (2015). Interleaved practice improves mathematics learning. *Journal of Educational
  Psychology* 107:900-908 *(secondary)*. https://www.aft.org/ae/spring2020/agarwal_agostinelli
- Siegler & Ramani (2008). Playing linear numerical board games promotes low-income children's numerical
  development. *Developmental Science* 11(5):655-661 *(What Works Clearinghouse review)*.
  https://ies.ed.gov/ncee/wwc/study/73675
- Siegler & Ramani (2009). *Journal of Educational Psychology* 101(3):545-560 *(What Works Clearinghouse review)*.
  https://ies.ed.gov/ncee/wwc/study/73725
- Laski & Siegler (2013). Learning from number board games: you learn what you encode. *Developmental Psychology*
  49(11). https://www.cmu.edu/dietrich/psychology/cs/research-teaching/docs/2013LaskiSieglerNumberBoardGames.pdf
- Siegler, Thompson & Schneider (2011). An integrated theory of whole number and fractions development. *Cognitive
  Psychology* 62(4):273-296. https://www.uni-trier.de/fileadmin/fb1/prof/PSY/PAE/Forschung/SieglerEtAl2011.pdf
- Fuchs et al. (2016). Effects of intervention to improve at-risk fourth graders' understanding, calculations, and
  word problems with fractions. *Elementary School Journal* 116(4) *(abstract only)*.
  https://edknowledge.stanford.edu/node/275
- Common Core State Standards, Mathematics: 1.OA.C.6, 2.OA.B.2, 2.MD.C.8, 3.OA.C.7, 3.NF.A.2.
  https://www.thecorestandards.org/Math/Content/3/OA/
- Caviola, Carey, Mammarella & Szucs (2017). Stress, time pressure, strategy selection and math anxiety in
  mathematics: a review of the literature. *Frontiers in Psychology* 8:1488.
  https://research.unipd.it/bitstream/11577/3255605/1/Caviola_etal2017Frontiers.pdf
- Baroody (2006). Why children have difficulties mastering the basic number combinations and how to help them.
  *Teaching Children Mathematics*, August 2006:22-31 (position paper).
- Boaler, Williams & Confer (2015). Fluency without fear. youcubed (advocacy, not peer reviewed).
  https://youcubed.org/?p=963
- Deci, Koestner & Ryan (1999). A meta-analytic review of experiments examining the effects of extrinsic rewards on
  intrinsic motivation. *Psychological Bulletin* 125(6):627-668.
  https://depts.washington.edu/techdocs/papers/deciExtrinsicRewardsAndIntrinsicMotivation99.pdf
- Hanus & Fox (2015). Assessing the effects of gamification in the classroom. *Computers & Education* 80:152-161
  *(secondary)*.
- Sailer & Homner (2020). The gamification of learning: a meta-analysis. *Educational Psychology Review* 32:77-112.
  https://opus.bibliothek.uni-augsburg.de/opus4/frontdoor/index/index/docId/109056
- Shute (2008). Focus on formative feedback. *Review of Educational Research* 78(1):153-189.
  https://myweb.fsu.edu/vshute/pdf/shute%202008_b.pdf
- Fyfe, Rittle-Johnson & DeCaro (2012). The effects of feedback during exploratory mathematics problem solving:
  prior knowledge matters. *Journal of Educational Psychology* 104(4) *(via the dissertation's abstract)*.
  https://ir.vanderbilt.edu/items/133cb4e3-42ab-4296-a618-23c30ed5040e
- National Reading Panel (2000). Teaching children to read. NICHD.
  https://www.nichd.nih.gov/publications/pubs/nrp/findings
- Gough & Tunmer (1986). Decoding, reading, and reading disability. *Remedial and Special Education* 7:6-10
  *(abstract only)*.
- Cheatham & Allor (2012). The influence of decodability in early reading text on reading achievement: a review.
  *Reading and Writing* 25:2223-2246 *(abstract only)*.

Not verified, so not relied on: Hattie & Timperley (2007) on feedback, and the child studies of spacing in maths
(Barzagar Nazari & Ebersbach 2019; Sobel et al. 2011), which could not be opened.
