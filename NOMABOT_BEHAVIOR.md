# NomaBot Behavior Specification

> Milestone 4 (v0.4.0) — Companion Brain. Desktop sends world context only; firmware Brain picks behaviors, goals, habits, and dreams.

## Core formula

```text
LifeMode + Activity + Emotion + Season
         + Goal (with progress)
         + ShortMemory + Energy + Boredom + Curiosity
         + Personality (traits + likes)
    →  Behavior  →  TextRenderer
```

## Life modes

| Mode | When (desktop) | Feel |
|------|----------------|------|
| `work` | Weekday daytime, IDE active | Focus, goals with progress |
| `home` | Evening at desk, not coding | Tea, look outside, wind down |
| `travel` | User away / calendar stub | Helmet, map, bike, countdown |
| `night` | Late hours | Yawn, dreams, sleep sequences |
| `vacation` | Manual / weekend stub | Slow, playful, curious |

Desktop `LifeModeService` sets mode from time of day. Firmware never picks life mode.

## Activities

| Activity | When (desktop) | Default behaviors |
|----------|----------------|-------------------|
| `idle` | No focused app | look_around, blink, breathing, read |
| `coding` | IDE / editor foreground | typing, think, coffee, stretch, smile |
| `sleep` | Night mode or explicit | yawn, blink_slow, sleep, dream |

## Emotions

Emotions modify behavior weight tables and decay toward `neutral` (YAML `emotion_meta.duration_sec`).

| Emotion | Set by desktop when | Effect |
|---------|---------------------|--------|
| `neutral` | Default | Base activity table |
| `happy` | Build success, good news | More smile, wave, celebrate |
| `frustrated` | Build failed, errors | recover sequence: think → sigh → coffee |
| `sleepy` | Late night | yawn, blink_slow, sleep |
| `excited` | Deploy, milestone | celebrate |

## Brain internals (firmware + Python mirror)

| Subsystem | Location | Notes |
|-----------|----------|-------|
| Energy | Firmware Brain | Decays during coding; coffee/stretch/sleep restore |
| Boredom | Firmware Brain | Repeating behavior increases; boosts break behaviors |
| Curiosity | Firmware Brain | Autonomous "I wonder..." moments |
| Short memory | Firmware Brain | last coffee, last sleep; daily reset |
| Goals | Firmware Brain | focus 0→100→celebrate; recover on frustrated |
| Habits | YAML + desktop scheduler | morning, tea_break, evening, dream sequences |
| Long memory | Desktop SQLite | days_together, user_name; friendship messages |

## Protocol commands (world context only)

| Command | Params | Desktop sends |
|---------|--------|---------------|
| `set_activity` | `activity` | ActivityService |
| `set_emotion` | `emotion` | BuildEventService, dev panel |
| `set_life_mode` | `mode` | LifeModeService |
| `set_season` | `season` | SeasonService |
| `trigger_habit` | `habit` | DailyRoutineService, scheduler |
| `show_message` | `text`, `duration_ms` | FriendshipService, scheduler |
| `play_animation` | `animation` | Override only (bypasses Brain until next activity) |

Never send behavior IDs from desktop.

## Data pipeline

1. `behavior.yaml` + `personality.yaml` → `behavior.json` (SDK compiler)
2. Pack compile copies `behavior.json` to LittleFS
3. Firmware `Brain.loadFromPackPath()` loads personality from JSON; behavior tables from embedded defaults (YAML drives compiler tests; tables synced via `behavior_defaults`)

## Diagnostics payload

```json
{
  "life_mode": "work",
  "activity": "coding",
  "emotion": "happy",
  "energy": 72,
  "boredom": 15,
  "goal": "focus",
  "goal_progress": 60,
  "behavior": "typing",
  "render_mode": "text",
  "short_memory": { "last_coffee_min_ago": 45 }
}
```

## Text renderer layout

```text
WORK · CODING
happy · focus · 60%
Energy: 72
Typing...
Build failed          ← message overlay, auto-clear 5s
```

## Architecture freeze (post-M4)

No new Brain subsystems after v0.4.0. Tune YAML, swap renderer (v0.5 sprites), add desktop integrations (v0.8 plugins).
