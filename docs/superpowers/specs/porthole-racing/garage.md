# Rally garage (v1)

Decided 2026-11-21. Stars buy the kid's own car: a little Power, and looks. Implementation:
`apps/porthole/games/racing/garage.h` (catalog and rules), `purse.h` (stars), `tune.h` `KIT_*` (Power effects),
`save.h` v3, and the racing `AGENTS.md` (Garage, Save layout).

## What a kid can buy

| Kind | Items | Price (stars) |
| --- | --- | --- |
| Power parts, 3 tiers each, in order | Engine, Turbo, Tires, Suspension, Nitro tank | 10 / 25 / 50 a tier |
| Paint | Red (stock), Blue, Lime, Purple, Pink, White, Black, Gold, Chrome | 0, 5, 5, 5, 10, 10, 10, 20, 30 |
| Stripes | Racing stripe, Flames, Lightning | 10, 15, 15 |

Each tier is +1 Power (0 to 15). Tier 2 of any part needs the rank Racer, tier 3 Champion. Prices never change and
show before every buy. Nothing is sold back; Undo refunds in full until the kid leaves the garage.

Not in v1: rims, exhaust, number, spoiler, finish effects, trophies, bodies, Bumper and Pit crew.

## Power effects (the kid's car only)

| Part | Changes | Stock | Tier 1 / 2 / 3 |
| --- | --- | --- | --- |
| Engine | top speed | x1 | x1.000425 / 1.00085 / 1.001275 |
| Turbo | 0 to full speed | 2.5 s | 2.487 / 2.475 / 2.462 s |
| Tires | steering, ice | 1.5; ice 0.25 / 0.75 | 1.6 / 1.7 / 1.8; ice 0.20, 0.15, 0.11 / 0.80, 0.85, 0.90 |
| Suspension | off-road, mud | 0.55 / 0.65 | 0.61, 0.67, 0.72 / 0.70, 0.74, 0.78 |
| Nitro tank | boost time | 1500 ms | 1513 / 1526 / 1538 ms |

Boost strength, rocket damage, the post-hit floor and the rivals never change. The bar (`host/test_racing_rules.cpp`
`fairKit`, 40 races a track): with Power 15 the kid driver wins Hard clearly more often than stock but at most 24 in
40, and the competent driver also at most 24 in 40. The brief's first figures (Engine x1.01 to x1.03, Turbo 2.2 to
1.6 s, Nitro 1800 to 2400 ms) won every Normal and Hard race even at tier 1, because no rival may pass the kid's top
speed and 0.35% of pace decides a race. Engine, Turbo and Nitro are therefore the brief's steps times one factor,
`tune.h` `KIT_SCALE` = 4.25%, the largest that meets the bar on every track (4.5% gives Industrial Ruins 28/40);
Tires and Suspension keep the brief's figures, since they barely move a race for the test drivers. Nitro is counted in
20 ms steps, so tiers 2 and 3 both boost 1540 ms.

| Track | Power 15 quicker | Competent, Hard /40 | Kid, Hard /40 stock to Power 15 | Auto, last 40 races |
| --- | --- | --- | --- | --- |
| Desert Canyon | 0.50% | 20 | 8 to 20 | 24 wins, level +4 |
| Industrial Ruins | 0.43% | 23 | 12 to 18 | 25 wins, level +3 |
| Snow Track | 0.30% | 18 | 13 to 19 | 28 wins, level +6 |
| Night Run | 0.42% | 24 | 12 to 23 | 25 wins, level +5 |

## Pages

Tracks: Car, with a gold dot when something is affordable. Garage: stars, the car on a podium, Power N and a 15-step
bar, the Stock switch (all parts off, looks stay), Looks, Power, Back. An item page: Back on top, a Paint / Stripes
chip (looks), < and >, the car tried on (looks) or three tier pips (a part), the status line, and one action (Buy,
Equip, Wearing, Locked, All on your car), with Undo beside it when the kid bought this item in this visit. Buy opens a
question page whose Buy is clear of every button under the finger on the item page. Too few stars: a gold hint "Stars
come from / races & other games" for 2 s, no red, no sound.

## Ranks, boards, Together, stars

- Rookie, Racer at 5 finishes, Champion at 15. Any finish counts. Progress shows as "3 / 5"; Results shows a banner the
  race a rank is reached. Never called "Level".
- Times and Results: a Tuned board (every time, with a Power badge, "P7") and a Stock board (Power 0 races only).
- Together: both cars race stock and red; the protocol is unchanged.
- Stars: a solo finish pays 1 for the first 5 a day, a finish together 2 for the first 5; the skills engine
  (`skills::earnForPlay`) caps all play stars at 10 a day across every game that teaches nothing.
