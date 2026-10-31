# Toy Town feature contract

Players choose a valid default doll or freely style four doll materials. Home
opens the wardrobe, street and scrapbook. Before an outing, the player chooses
up to three bag items. Styling never changes anyone's social worth or abilities.

School balances Mina's presentation and Theo's display. Listening changes what
cue cards achieve; missing paper has a borrowing fallback. Each plan produces a
different result, with an optional repair conversation. Cafe preserves quantities
and comparison drafts; commit requires food and water within ten pretend coins.
Park varies its notebook location and matching clues together only when a new
outing starts. Wrong searches preserve progress and support a revised idea.

Completion offers Keep memory, Add details or Skip. Details allow a caption and
sticker choice. Keep records once in an eight-entry rolling scrapbook; Skip
completes without inserting or replacing a memory. A neutral stopping screen
makes leaving and continuing equally easy. Repeated touches cannot duplicate a
completion. There are no clock-based changes, game sound or attendance rewards.

Back opens Resume / Town / Exit. Town keeps the unfinished outing and allows
wardrobe or scrapbook play; resume it before starting a new outing. Title
Continue opens the exact saved page. Every meaningful action checkpoints urgently,
including pending decisions, comparisons, clues and memory edits. The shell owns
actual play limits; interruptions and time away never discard Toy Town progress.

Stable enums and an 88-byte V1 size+CRC record isolate each shell profile. This
is the first released schema, so no historical Toy Town migration exists. Future
fields append before CRC with a version bump and genuine V1 migration fixtures.
Corrupt or unsupported blobs are rejected without partially loading them.
