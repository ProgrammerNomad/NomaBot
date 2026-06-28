# Architecture Freeze (v0.5.0)

> After this release, **ship features — do not redesign subsystems** unless fixing a fundamental flaw (requires ADR in `docs/adr/`).

See also [SCENE_SPEC.md](./SCENE_SPEC.md) for the Tiny World Renderer contract (M5+).

## Frozen subsystems

| Subsystem | Location | Notes |
|-----------|----------|-------|
| **Brain** | `firmware/src/brain/` | Behavior, goals, habits, energy, emotion decay |
| **Behavior tables** | pack `behavior.json` / YAML compiler | Tuning only |
| **Goals & memory** | Brain short memory | Timestamps use `*Ms` suffix |
| **Context** | `ContextArbitrator`, `CommandSource` | Desktop world context |
| **Protocol v1** | `sdk/src/nomabot/protocol/` | NDJSON envelope `"v": 1` |
| **CommandRouter** | `desktop/.../command_router.py` | Context / Overlay / Renderer |
| **OverlayQueue** | `OverlayManager`, `OverlayService` | IDs, priority, expiry — not Brain state |
| **DirtyTracker** | `firmware/src/render/dirty_tracker.*` | Pending + Forced masks |
| **RenderScheduler** | `firmware/src/render/render_scheduler.*` | Band scheduling; sprite path → SceneBuilder → CharacterRenderer |
| **SceneBuilder + Scene** | `firmware/src/render/scene_builder.*`, `scene.h` | RenderState → scene graph (frozen API M5+) |
| **CharacterRenderer** | `firmware/src/render/character_renderer.*` | Scene-aware draw alongside TextSceneRenderer |
| **Main loop** | `firmware/src/main.cpp` | USB → tick → present → sleep |

## Allowed without ADR

- YAML / behavior tuning
- **Character Renderer** swap (M5+) — text → sprites → SVG/OLED/ePaper; pipeline: `RenderScheduler → SceneBuilder → CharacterRenderer → SpriteCache`
- New protocol commands routed through **CommandRouter**
- Character packs and asset compiler output
- Plugin **context** injection (not behavior picking)

## Main loop contract (frozen)

```text
usbPoll()
brainTick()
rendererPresent()
sleepYield()
```

Do not reorder without an ADR.

## Context vs overlay

**Context** (Brain inputs): activity, emotion, life_mode, season, habits.

**Overlay** (ephemeral UI): speech bubbles, build toasts, welcome messages — via `show_message` with `id`, never via `StateManager.message_active`.

## Timestamp convention

Brain short memory stores **monotonic millis** with `Ms` suffix (e.g. `_lastCoffeeMs`). Comparisons: `nowMs - lastFooMs > threshold`.
