#!/usr/bin/env python3
"""Pets Club's Ukrainian words: the word list below into content_uk.h, with each word's step 1..7 worked out.

    python3 games/pets-club/tools/uk_words.py           # writes games/pets-club/content_uk.h
    python3 games/pets-club/tools/uk_words.py --check   # fails if the header is not what this writes (make lint)

A word's step (the design page "Українська в Porthole", ступень слова), from its features, not marked by hand:

    1 + 0.6 (syllables - 1) + 0.25 (letters - 3) + 0.5 special + 0.7 clusters + 1.0 band - 0.6 russian + 1.5 false friend

rounded half up and kept in 1..7. special: і ї є ґ щ ь ʼ, each time; clusters: runs of two or more consonants; band:
0 = among the first words a kid meets ... 3 = rare; russian: looks and means like the Russian word (дім/дом), so a
kid who speaks Russian gets it faster; false friend: looks like a Russian word that means something else (неділя is
Sunday). The coefficients are a start: telemetry's uk_answer events are how they get tuned
(docs/superpowers/specs/2026-11-21-ukrainian-telemetry.md).

Gate: Ukrainian letters only (no ы э ъ ё), the apostrophe as U+02BC, syllables that join back into the word with one
vowel each, an ASCII English gloss of at most 16 characters, and lookalike sets of three listed words with three
different glosses, each word in one set at most. That the font draws every letter is checked in host/test_pets_uk.cpp.
"""

import math
import os
import sys
from pathlib import Path

os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/../../..")  # paths below are relative to apps/porthole
OUT = Path("games/pets-club/content_uk.h")

# syllables (hyphens; ' is the apostrophe) | English | band | flags: r = like the Russian word, f = false friend
WORDS = """
ма-ма|mom|0|r
та-то|dad|0|
ба-ба|grandma|0|r
дід|grandpa|0|r
брат|brother|0|r
се-стра|sister|0|r
син|son|0|r
донь-ка|daughter|0|
друг|friend|0|r
ді-ти|children|0|r
кіт|cat|0|r
кит|whale|1|r
кут|corner|2|
пес|doggy|0|r
со-ба-ка|dog|0|r
ко-за|goat|1|r
ко-са|braid|2|r
ро-са|dew|2|r
ко-ро-ва|cow|0|r
кінь|horse|1|r
тінь|shadow|2|r
лінь|laziness|3|r
вовк|wolf|1|r
лис|fox|2|r
ве-дмідь|bear|1|
за-єць|hare|1|r
ї-жак|hedgehog|1|
бі-лка|squirrel|1|r
гі-лка|branch|2|
бу-лка|bun|2|r
ми-шка|mouse|1|r
ми-ска|bowl|2|r
ши-шка|pine cone|2|r
со-ва|owl|1|r
слон|elephant|1|r
сом|catfish|3|r
сон|sleep|1|r
жук|beetle|1|r
лук|bow|2|r
бук|beech|3|r
бджо-ла|bee|1|
ри-ба|fish|0|r
рі-па|turnip|3|r
ли-па|linden|3|r
рак|crayfish|2|r
мак|poppy|2|r
лак|nail polish|3|r
птах|bird|1|r
ка-чка|duck|1|
ку-рка|hen|1|r
пі-вень|rooster|1|
сви-ня|pig|1|r
ле-ле-ка|stork|2|
жа-ба|frog|1|r
ме-те-лик|butterfly|1|
лев|lion|1|r
тигр|tiger|2|r
ма-впа|monkey|2|
бик|bull|2|r
бік|side|2|r
біг|running|2|r
кро-лик|rabbit|1|r
кі-шка|kitty|0|r
ще-ня|puppy|1|r
ко-ше-ня|kitten|1|
ка-че-ня|duckling|2|
є-нот|raccoon|3|r
я-щі-рка|lizard|3|r
щу-ка|pike|2|r
ґа-ва|crow|3|
хліб|bread|0|r
мо-ло-ко|milk|0|r
сир|cheese|0|r
сік|juice|1|r
суп|soup|0|r
ка-ша|porridge|1|r
на-ша|our|0|r
ва-ша|your|0|r
мед|honey|1|r
я-блу-ко|apple|0|r
гру-ша|pear|1|r
сли-ва|plum|2|r
ма-ли-на|raspberry|1|r
ма-ши-на|car|0|r
кар-ти-на|picture|2|r
рис|rice|1|r
ріг|horn|2|r
рік|year|1|
м'я-со|meat|1|r
м'яч|ball|0|r
м'я-та|mint|2|r
ци-бу-ля|onion|2|
мо-рква|carrot|2|r
ка-пу-ста|cabbage|2|r
са-ло|lard|2|r
се-ло|village|1|r
ми-ло|soap|2|r
сіль|salt|1|r
біль|pain|2|r
ціль|target|3|r
цу-кор|sugar|1|
я-йце|egg|1|r
борщ|borscht|1|r
ва-ре-ник|dumpling|2|r
пи-ріг|pie|2|r
ї-жа|food|1|
во-да|water|0|r
во-на|she|0|r
во-ни|they|0|r
чай|tea|0|r
мо-ро-зи-во|ice cream|2|
дім|house|0|r
дим|smoke|2|r
ві-кно|window|0|r
две-рі|door|0|r
стіл|table|0|r
сті-лець|chair|1|
лі-жко|bed|1|
ло-жка|spoon|1|r
ли-жі|skis|2|r
ча-шка|cup|1|r
ніс|nose|0|r
ніж|knife|1|r
ніч|night|0|r
ки-лим|carpet|3|
ла-мпа|lamp|2|r
кни-жка|book|0|r
ні-жка|little leg|2|r
ля-лька|doll|1|
і-гра-шка|toy|1|r
ву-ли-ця|street|1|r
лі-так|plane|1|
по-тяг|train|1|
а-вто-бус|bus|1|r
ве-ло-си-пед|bike|2|r
со-нце|sun|0|r
мі-сяць|moon|1|r
зі-рка|star|1|
не-бо|sky|0|r
хма-ра|cloud|1|
дощ|rain|1|
сніг|snow|0|r
лід|ice|1|r
ві-тер|wind|1|r
ліс|forest|0|r
лист|leaf|1|r
рі-ка|river|1|r
ру-ка|hand|0|r
мо-ре|sea|0|r
го-ра|mountain|0|r
но-ра|burrow|2|r
ко-ра|bark|3|r
тра-ва|grass|0|r
кві-тка|flower|1|
дуб|oak|2|r
зуб|tooth|0|r
лі-то|summer|0|r
зи-ма|winter|0|r
ве-сна|spring|1|r
о-сінь|autumn|1|r
день|day|0|r
ра-нок|morning|1|
ве-чір|evening|1|r
міст|bridge|1|r
хвіст|tail|1|r
гість|guest|2|r
но-га|leg|0|r
го-ло-ва|head|0|r
о-ко|eye|1|r
ву-хо|ear|1|r
рот|mouth|0|r
жи-віт|tummy|2|r
шко-ла|school|0|r
клас|class|1|r
у-рок|lesson|1|r
ру-чка|pen|1|r
о-лі-вець|pencil|2|
пе-нал|pencil case|2|r
па-пір|paper|2|
ма-па|map|2|
ма-ла|little|1|r
ве-ли-кий|big|1|r
до-брий|kind|1|r
ве-се-лий|happy|1|r
че-рво-ний|red|1|
жо-втий|yellow|1|r
зе-ле-ний|green|1|r
бі-лий|white|1|r
чо-рний|black|1|r
си-ній|blue|1|r
о-дин|one|0|r
два|two|0|r
три|three|0|r
чо-ти-ри|four|0|r
п'ять|five|0|r
шість|six|1|r
сім|seven|1|r
ві-сім|eight|1|r
де-в'ять|nine|1|r
де-сять|ten|1|r
п'я-тни-ця|Friday|1|
ши-ро-кий|wide|2|r
хо-ло-дний|cold|1|r
по-да-ру-нок|gift|1|r
зо-ло-тий|golden|2|r
ве-ре-сень|September|2|
со-ня-шник|sunflower|2|r
ку-ку-ру-дза|corn|2|r
при-го-да|adventure|2|
ка-зка|fairy tale|1|r
ча-рі-вний|magic|2|
ді-вчи-нка|girl|1|r
хло-пчик|boy|1|
дру-жба|friendship|2|r
ща-стя|happiness|2|r
ґу-дзик|button|2|
ґа-нок|porch|3|
те-ле-фон|phone|1|r
ко-мп'ю-тер|computer|2|r
бі-блі-о-те-ка|library|2|r
по-до-ро-жу-ва-ти|to travel|3|
не-ді-ля|Sunday|2|f
го-ди-на|hour|1|f
лю-тий|February|2|f
лу-на|echo|2|f
"""

I_HINT = "і says ee, as in see; и says i, as in sit."
SETS = """
кіт кит кут|i
ліс лис лист|i
дім дим дід|i
ріг рік рис|i
бик бік біг|i
ніс ніж ніч|Look at the last letter.
сир сік син|Look at the last letter.
кінь тінь лінь|Look at the first letter.
міст хвіст гість|Look at the first letters.
мак рак лак|Look at the first letter.
дуб зуб суп|Look at every letter.
жук лук бук|Look at the first letter.
сон сом слон|Look at every letter.
гора нора кора|Look at the first letter.
коза коса роса|Look at every letter.
мама мала мапа|Look at the middle.
село сало мило|Look at every letter.
рука ріка щука|Look at every letter.
ліжко ложка лижі|Look at every letter.
миска мишка шишка|с and ш look alike: look again.
вода вона вони|Look at every letter.
каша наша ваша|Look at the first letter.
білка гілка булка|Look at the first letters.
риба ріпа липа|Look at every letter.
малина машина картина|Look at the middle.
книжка кішка ніжка|Look at the first letters.
мʼяч мʼята мʼясо|Look at the end.
"""

APOS = "ʼ"
VOWELS = set("аеєиіїоуюя")
CONSONANTS = set("бвгґджзйклмнпрстфхцчшщ")
LETTERS = VOWELS | CONSONANTS | {"ь", APOS}
SPECIAL = set("іїєґщь" + APOS)
STEPS = 7   # the ladder: past step 7 there were too few words a step, so one came back every round


def fail(msg: str) -> None:
    sys.exit(f"uk_words: {msg}")


def step(word: str, syl: int, band: int, flags: str) -> int:
    letters = sum(c != APOS for c in word)
    special = sum(c in SPECIAL for c in word)
    clusters, run = 0, 0
    for c in word + "а":
        if c in CONSONANTS:
            run += 1
            continue
        if c == "ь" or c == APOS:   # a soft sign or an apostrophe sits inside a cluster, not between two
            continue
        clusters += run >= 2
        run = 0
    raw = 1 + 0.6 * (syl - 1) + 0.25 * (letters - 3) + 0.5 * special + 0.7 * clusters + band
    raw += -0.6 * ("r" in flags) + 1.5 * ("f" in flags)
    return max(1, min(STEPS, math.floor(raw + 0.5)))   # the top steps hold too few words to be steps of their own


def tiles(syls: list[str]) -> bool:
    """Built from its syllables (exercise 3): two or three, each open but the last, none starting on two consonants."""
    if not 2 <= len(syls) <= 3:
        return False
    return all(s[-1] in VOWELS for s in syls[:-1]) and all(not (s[0] in CONSONANTS and len(s) > 1 and s[1] in CONSONANTS) for s in syls)


def words() -> list[dict]:
    out, seen = [], set()
    for line in WORDS.strip().splitlines():
        syl, en, band, flags = line.split("|")
        syl = syl.replace("'", APOS)
        word = syl.replace("-", "")
        parts = syl.split("-")
        if word in seen:
            fail(f"{word} twice")
        seen.add(word)
        if set(word) - LETTERS:
            fail(f"{word}: letters {''.join(sorted(set(word) - LETTERS))} are not Ukrainian (ы э ъ ё, or a capital)")
        if any(sum(c in VOWELS for c in p) != 1 for p in parts):
            fail(f"{syl}: a syllable without exactly one vowel")
        if not en.isascii() or not 1 <= len(en) <= 16 or not en.replace(" ", "").isalpha():
            fail(f"{word}: gloss {en!r} is not 1..16 ASCII letters")
        if any(w["en"] == en for w in out):   # telemetry names a word by its gloss
            fail(f"{word}: gloss {en!r} is another word's too")
        if band not in "0123" or set(flags) - set("rf"):
            fail(f"{word}: band {band!r} or flags {flags!r}")
        out.append({"uk": word, "syl": syl, "en": en, "step": step(word, len(parts), int(band), flags), "tiles": tiles(parts), "look": 0})
    return out


def sets(ws: list[dict]) -> list[tuple[list[int], str]]:
    at = {w["uk"]: i for i, w in enumerate(ws)}
    out = []
    for n, line in enumerate(SETS.strip().splitlines(), 1):
        names, hint = line.split("|")
        members = names.split()
        if len(members) != 3 or any(m not in at for m in members):
            fail(f"set {names}: three listed words")
        if any(ws[at[m]]["look"] for m in members):
            fail(f"set {names}: a word already in another set")
        if len({ws[at[m]]["en"] for m in members}) != 3:
            fail(f"set {names}: two words with one gloss")
        for m in members:
            ws[at[m]]["look"] = n
        out.append(([at[m] for m in members], I_HINT if hint == "i" else hint))
    return out


def c_str(s: str) -> str:
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def render() -> str:
    ws = words()
    ss = sets(ws)
    rows = ",\n".join(f"  {{{c_str(w['uk'])}, {c_str(w['en'])}, {c_str(w['syl'])}, {w['step']}, {w['look']}, {int(w['tiles'])}}}" for w in ws)
    looks = ",\n".join(f"  {{{{{a}, {b}, {c}}}, {c_str(h)}}}" for (a, b, c), h in ss)
    hist = [sum(w["step"] == s for w in ws) for s in range(1, STEPS + 1)]
    return f"""// AUTO-GENERATED by tools/uk_words.py. Do not edit: change the list there and run it.
// Pets Club's Ukrainian words (ukrainian.h, ukrainian.cpp): {len(ws)} words, {len(ss)} lookalike sets.
// Words per step 1..{STEPS}: {' '.join(map(str, hist))}. Telemetry names a word by its English, so keep glosses stable.
#pragma once
#include <stdint.h>

namespace learn::uk {{
// syl: the syllables joined by '-'; step: 1..7 (tools/uk_words.py); look: its lookalike set + 1 (0: none);
// tiles: exercise 3 can build it from its syllables.
struct Word {{ const char* uk; const char* en; const char* syl; uint8_t step, look, tiles; }};
struct Look {{ uint16_t words[3]; const char* hint; }};   // three WORDS that differ by a letter or two, and what to look at

inline constexpr Word WORDS[] = {{
{rows}
}};
inline constexpr int WORD_COUNT = sizeof WORDS / sizeof *WORDS;
inline constexpr Look LOOKS[] = {{
{looks}
}};
inline constexpr int LOOK_COUNT = sizeof LOOKS / sizeof *LOOKS;
}}  // namespace learn::uk
"""


def main() -> None:
    text = render()
    if sys.argv[1:] == ["--check"]:
        if not OUT.exists() or OUT.read_text(encoding="utf-8") != text:
            fail(f"{OUT} is stale: run python3 {sys.argv[0]}")
        return
    OUT.write_text(text, encoding="utf-8")
    print(f"wrote {OUT}")


if __name__ == "__main__":
    main()
