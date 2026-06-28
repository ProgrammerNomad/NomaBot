# NomaBot Behavior Specification

> Milestone 3.5 — frozen architecture. Desktop sends world context; firmware picks moment-to-moment behaviors.

## Core formula

```text
LifeMode + Activity + Emotion  →  Behavior  →  Renderer
         (+ personality, + conditions, + memory)
```

## Life modes

| Mode | Activities | Feel |
|------|------------|------|
| `work` | idle, coding, meetings, debugging | Focused, coffee, celebrate/sigh |
| `relax` | idle, music, reading, look_outside | Slow, wave, smile |
| `travel` | pack, helmet, map, ride | Energetic, countdown |
| `night` | wind_down, sleep, dream | Dim, yawn, slow blink |

Week 1 ships `work` only. Desktop sets life mode from time/context (Week 3).

## Activities (Week 1)

| Activity | When (desktop) | Default behaviors |
|----------|----------------|-------------------|
| `idle` | No focused app | look_around, blink, breathing, read |
| `coding` | IDE / editor foreground | typing, think, coffee, stretch, smile |
| `sleep` | Night mode or explicit | yawn, blink_slow, sleep, dream |

## Emotions

Emotions modify behavior weight tables. They decay toward `neutral` after ~5 minutes.

| Emotion | Set by desktop when | Effect |
|---------|---------------------|--------|
| `neutral` | Default | Base activity table |
| `happy` | Build success, good news | More smile, wave, celebrate |
| `frustrated` | Build failed, errors | think, sigh, coffee, typing_slow |
| `sleepy` | Late night | yawn, blink_slow, sleep |
| `excited` | Deploy, milestone | celebrate, jump, wave |

### Emotion × activity examples

**Coding + happy:** typing, typing, smile, coffee

**Coding + frustrated:** think, sigh, coffee, typing_slow

**Idle + happy:** wave, look_around, smile

**Idle + sleepy:** yawn, blink_slow, sleep

## Behavior catalog

| ID | Label | Clip (Week 4) |
|----|-------|---------------|
| breathing | Breathing... | idle |
| blink | Blink | idle |
| look_around | Look around | idle |
| typing | Typing... | coding |
| typing_slow | Typing slowly... | coding |
| think | Thinking... | coding |
| coffee | Coffee | coffee |
| stretch | Stretch | idle |
| smile | Smile | idle |
| sigh | Sigh... | idle |
| wave | Wave | idle |
| yawn | Yawn... | sleep |
| blink_slow | Blink slowly... | sleep |
| sleep | Sleep | sleep |
| dream | Dream... | sleep |
| celebrate | Celebrate! | celebrate |

## Transitions

1. **Activity change** — pick new behavior immediately from new activity table (respecting emotion).
2. **Behavior timer** — when duration elapses, weighted-random pick next behavior (exclude current if possible).
3. **Emotion change** — re-pick behavior from emotion table on next tick.
4. **Explicit override** — `play_animation` bypasses engine until next `set_activity`.

## Personality traits

Traits scale behavior weights (0–100). Same firmware, different YAML per character.

| Trait | Effect |
|-------|--------|
| `energy` | Boosts active behaviors when high |
| `coffee_love` | Boosts `coffee` weight |
| `curiosity` | Boosts `look_around`, `think` |
| `sleepiness` | Boosts `yawn`, `sleep` |
| `optimism` | Boosts `smile`, `celebrate` |
| `patience` | Reduces `sigh`, `frustrated` decay |
| `playfulness` | Boosts `wave`, playful idle behaviors |

## Conditions (Week 2)

Evaluated against runtime state before a behavior is eligible:

| Behavior | Condition |
|----------|-----------|
| `coffee` | `energy < 40` OR always if `coffee_love >= 50` |
| `sleep` | `idle_minutes > 10` |

## Memory (Week 3)

Tiny runtime store — no firmware changes to add fields:

- `last_coffee` — timestamp
- `last_build` — `success` | `failed`
- `last_sleep` — timestamp

## Desktop command contract

Desktop sends **world context only**:

```json
{ "cmd": "set_life_mode", "params": { "mode": "work" } }
{ "cmd": "set_activity",  "params": { "activity": "coding" } }
{ "cmd": "set_emotion",   "params": { "emotion": "frustrated" } }
{ "cmd": "show_message",  "params": { "text": "Build failed" } }
```

Desktop never sends `typing`, `coffee`, or other micro-behaviors.

## Diagnostics

```json
{
  "life_mode": "work",
  "activity": "coding",
  "emotion": "frustrated",
  "behavior": "typing",
  "render_mode": "text",
  "time_in_behavior_sec": 8,
  "next_behavior": "coffee"
}
```
