# NomaBot Style Guide

> Master visual rules. On-device readability laws: [READABILITY.md](./READABILITY.md).

## Core identity

**Tiny explorer who happens to love coding.**

## Silhouette test

NomaBot must read at thumbnail size:

```text
⬛⬛🟧⬛⬛   ← orange helmet dome
```

If the helmet is not obvious at 32px, redesign before production sprites.

## Camera (frozen)

- **Allowed:** straight front, 3/4 front only
- **Never:** side profile in-game, top-down, isometric, random perspective mix

## Color palette (24 colors — do not add new colors in production)

| Name | Hex | Use |
|------|-----|-----|
| Helmet Orange | `#F97316` | Helmet, mug, backpack accents |
| Helmet Dark | `#C2410C` | Helmet shadow |
| Body White | `#F8FAFC` | Body shell |
| Body Gray | `#CBD5E1` | Body shadow |
| Visor Black | `#0F172A` | Visor band |
| Eye Blue | `#38BDF8` | Eyes |
| Eye Highlight | `#E0F2FE` | Eye shine |
| Outline | `#1E293B` | 2px outlines |
| Desk Wood | `#92400E` | Desk, shelf |
| Desk Light | `#B45309` | Wood highlight |
| Wall | `#1E293B` | Apartment wall |
| Wall Alt | `#334155` | Wall panels |
| Window Sky | `#7DD3FC` | Window |
| Window Frame | `#64748B` | Frame |
| Plant Green | `#22C55E` | Plant |
| Plant Pot | `#A16207` | Pot |
| Lamp Warm | `#FDE68A` | Lamp glow |
| Laptop Gray | `#475569` | Laptop |
| Screen Glow | `#34D399` | Laptop screen |
| Coffee Liquid | `#78350F` | Coffee |
| Compass Gold | `#FBBF24` | Badge |
| Bubble Fill | `#F1F5F9` | Speech bubble |
| Bubble Border | `#64748B` | Bubble outline |
| Shadow | `#0F172A` | Bottom-right shadows |

## Drawing rules

| Rule | Value |
|------|--------|
| Outline | 2px, color Outline |
| Light source | Top-left |
| Shadow | Bottom-right |
| Helmet | Always orange explorer style |
| Visor | Always black band |
| Eyes | Always blue (unless closed expression) |

## Resolution workflow

1. Large concept (1024) — design exploration only
2. **Redraw at 170×320** for apartment and in-game assets
3. Poses and frames from simplified 170×320 style — **never** downscale AI noise to production

## Signature accessories

| Item | Notes |
|------|--------|
| Orange explorer helmet | On head or helmet hook |
| Orange coffee mug | Desk / hand |
| Small laptop | Desk / lap |
| Orange backpack | Hook or floor spot |
| Compass badge | Helmet front |

## Expressions (12)

Neutral, Smile, Happy, Laugh, Thinking, Confused, Sleepy, Yawning, Angry, Embarrassed, Surprised, Proud.

Face-only overlays — same body, swap `sprites/face/`.

## Related docs

- [LORE.md](./LORE.md)
- [LIVING_NOMA_SPEC.md](./LIVING_NOMA_SPEC.md)
- [SCENE_SPEC.md](./SCENE_SPEC.md)
