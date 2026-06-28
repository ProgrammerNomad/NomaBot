# Scene Specification (M5+)

> Contract for the Tiny World Renderer — scene graph, draw order, dirty rules, and RenderState mapping.
> Linked from [ARCHITECTURE_FREEZE.md](./ARCHITECTURE_FREEZE.md) and [10_ROADMAP.md](./10_ROADMAP.md).

## Vision

The display shows a **tiny room**, not a single sprite. The world changes around the robot (office, travel, night, rain) more than the body sprite alone. The renderer draws **scenes**, not raw sprite IDs.

## Frozen pipeline (M5+)

```text
Brain → RenderState → DirtyTracker → RenderScheduler → SceneBuilder → CharacterRenderer → SpriteCache → LGFX
```

Animation stays upstream: `Behavior → Animation → Frame → RenderState.bodySpriteId → SceneBuilder → Scene`.

## Scene objects (M5)

| Node | z | M5 content | Future |
|------|---|------------|--------|
| `BackgroundNode` | 0 | Full-screen bg sprite (`bg_office`) | office, travel, night, rain layers |
| `CharacterNode` | 10 | Single body sprite at pack anchor | Body, eyes, accessories, shadow |
| `ExpressionNode` | 11 | Face overlay from `RenderState.emotion` (M5.1) | 12 expressions from pack |
| `HudNode` | 30 | Behavior label strip (top) | Energy, mode indicators |
| `SpeechBubbleNode` | 40 | Styled overlay bubble (M5.1) | Multi-line, tail |

Reserved (M6+): `PropNode` (z=20, baked into apartment bg in M5.1), `WeatherNode` (z=5).

Art direction: [STYLE_GUIDE.md](./STYLE_GUIDE.md), [LORE.md](./LORE.md), [LIVING_NOMA_SPEC.md](./LIVING_NOMA_SPEC.md).

## Node fields

```cpp
struct SceneNode {
  const char *id;        // logical id: "office", "body_typing_03", "hud", "dev_hello"
  const char *spriteId;  // asset ref; null for text-only nodes
  const char *text;      // HUD / bubble copy; null for sprite-only nodes
  int x, y, z;
  bool visible;
  bool dirty;
};
```

## Draw order

Lowest z first: Background (0) → Character (10) → Props (20, future) → HUD (30) → SpeechBubble (40).

## Dirty rules (M5 — coarse flags only)

| Dirty flag | Scene nodes redrawn |
|------------|---------------------|
| `DirtyFull` | All visible nodes (first paint clears screen via background) |
| `DirtyBackground` | Background; re-capture character footprint cache; redraw character on top |
| `DirtyCharacter` | Restore background patch from cache; redraw character |
| `DirtyBehavior` | HUD |
| `DirtyMessage` | SpeechBubble |

M5 uses **`DirtyCharacter`** for the whole character entity. Do not split per-part dirty yet.

### Reserved DirtyFlags (M6+, not implemented)

Document only — same enum bits, no new logic in M5:

- `DirtyBody`, `DirtyEyes`, `DirtyAccessory` — fold under `DirtyCharacter` until M6
- `DirtyBubble`, `DirtyHud` — fold under `DirtyMessage` / `DirtyBehavior` until needed

## RenderState → Scene mapping

| RenderState field | Scene output |
|-------------------|--------------|
| `backgroundSpriteId` | `background.spriteId`; scene id derived (e.g. `bg_office` → `"office"`) |
| `lifeMode` + `activity` | Scene id hint (M5: still single office bg) |
| `bodySpriteId` | `character.spriteId`, `character.id` = sprite id |
| `behaviorLabel` | `hud.text`, visible when non-empty |
| `overlayText` | `speechBubble.text`, visible when non-empty |
| Pack anchor | `character.x`, `character.y` from `PackLoader` |

SceneBuilder does **not** include Brain types — only `RenderState` + pack defaults.

## BackgroundCache

On character move/frame change, **do not** `fillRect` erase the old body.

1. After background blit, capture RGB565 patch at character footprint into `BackgroundCache`.
2. On `DirtyCharacter`: restore patch → blit new body sprite.
3. Keeps desk/window/props visible under the character in future milestones.

## Scene Pack layout (future)

Target asset tree (no forced migration in M5):

```text
nomabot/
  backgrounds/          # office/, travel/, night/
  characters/
    body/
    eyes/               # M6+
    accessories/        # M6+
  animations/
  manifest.json
```

M5 continues to use `sprites/bg/`, `sprites/body/`, and `config.json` backgrounds map.

## Diagnostics snapshot

Last built scene exposed for debug (not Brain state):

```json
{
  "scene": "office",
  "body": "typing_03",
  "eyes": "",
  "overlay": "Hello",
  "render_objects": 4
}
```

## Out of scope (M5)

- Per-part dirty (`DirtyEyes`, `DirtyHat`, …)
- Weather, props, accessories, eye blink layers
- Scene Pack directory migration
- Brain / CommandRouter / main loop changes
- Multi-bubble overlay stack
