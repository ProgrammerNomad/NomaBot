# NomaBot Character Bible

> Frozen after Prototype Pack v0 passes on-device readability tests (Phase C).
> Draft values below come from v0.5.2 LCD iteration — update after desk testing.

## Hero

- Tiny explorer who loves coding
- Orange explorer helmet (dominant silhouette at 32px)
- Head anchor: **(85, 80)** — home position
- Body sprites: 56×72 RGBA

Reference: [prototype_v0/hero_home.png](../prototype_v0/hero_home.png)

## Colors

From tested v0 sprites (see [STYLE_GUIDE.md](../../docs/STYLE_GUIDE.md) for full palette):

| Role | Hex |
|------|-----|
| Helmet | `#F97316` |
| Body | `#F8FAFC` |
| Visor | `#0F172A` |
| Eyes | `#38BDF8` |
| Outline | `#1E293B` |

## Expressions (v0 → full)

| Emotion | Face sprite | Phase |
|---------|-------------|-------|
| neutral | face_neutral | v0 |
| happy / excited | face_happy | v0 |
| thinking / curious | face_thinking | v0 |
| sleepy | face_sleepy | E |
| surprised | face_surprised | E |
| frustrated | face_angry | E |

## Poses (Prototype v0)

| Pose | Sprite | Sells |
|------|--------|-------|
| Standing | body_stand / body_idle_* | Home / breathing |
| Typing | body_typing_* | Arms down on keyboard |
| Thinking | body_think | Head tilt ↗ + hand on chin |
| Blink | body_blink_01 | Closed visor strip |

## Behavior → clip → visual

| behavior_id | clip | Visual beat |
|-------------|------|-------------|
| breathing | idle | Stand at home, idle bob |
| blink | blink | Visor closes briefly |
| typing | coding | Arms alternate on desk |
| think | think | Head tilt, hold 2–4s |
| look_around | look_left | Body turns left |
| coffee | coffee | Mug raised |
| sleep | sleep | Slumped pose |

Brain unchanged — mapping lives in [behavior.yaml](../behavior.yaml) and `behavior_clips`.

## Animation rules

1. HUD-hidden test must pass before adding behaviors
2. Silhouette before micro-animation
3. Return to home standing pose after each clip cycle

## Apartment (Phase D)

Six props at 170×320: window, desk, plant, laptop, coffee, shelf.

Layout:

```text
Window
────────────
Plant    Shelf
  🤖 home (85,80)
Laptop   Coffee
```

## Accessories

Orange helmet, compass badge, coffee mug, laptop — see [LORE.md](../../docs/LORE.md).
