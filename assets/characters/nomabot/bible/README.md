# Character Bible (Phase C - draft)

Frozen from LCD-tested constants after Phase B exit gate. Do not expand until
[`docs/PHASE_B_NOTES.md`](../../../docs/PHASE_B_NOTES.md) readability QA passes for 3+ desk days.

## Proportions (Phase B - 2026-06-28)

| Parameter | Value |
|-----------|-------|
| Body anchor | (85, 80) |
| Expression offset | dx=0, dy=24 |
| Think head tilt | 11 px |
| Helmet width | ±21 px |
| Typing arms | ±36, oy+42 |
| Body colorkey | `0xF81F` on body + face PNGs |
| Idle breathe | `body_idle_02` standing_breathe (+2px) |
| Clip timing | idle 300ms, blink 150/300ms |
| Pet mode | HUD hidden on LCD (`display.pet_mode`) |

## v0 clips

| Clip | Read |
|------|------|
| idle | Standing |
| blink | Blink |
| coding | Typing |
| think | Thinking |

## Expressions (v0)

neutral, happy, thinking, blink - map in `config.json`.

## Alive pet test

Leave desktop + device running while coding. Pass = HUD-hidden 1m read + 60s video
with at least one "why is he alive?" moment.
