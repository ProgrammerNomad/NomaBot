# Living NomaBot — Production Spec (M5.1 First Living Character)

> Canvas, anchors, clips, export paths. See [READABILITY.md](./READABILITY.md), [STYLE_GUIDE.md](./STYLE_GUIDE.md), [LORE.md](./LORE.md).

## Canvas

- Display: **170×320** (LILYGO T-Display S3)
- **Body anchor**: `(85, 80)` — all behaviors return here
- **Expression anchor**: relative to body — `{ "dx": 0, "dy": 24 }` in `config.json` (Phase B tuned)
- Hands anchor: `(85, 200)`

## Layer model (permanent)

| Layer | Contents | Notes |
|-------|----------|-------|
| **Body** | Helmet, visor, neck, torso, hands | One visor only — drawn per pose clip |
| **Expression** | Eyes, eyebrows, mouth | Swaps on emotion; no helmet or visor |

Sprite ids remain `face_*` in the pack; source PNGs live in `sprites/face/`. Expression blit uses RGB565 colorkey `0xF81F` (alpha → colorkey at compile time; LovyanGFX keyed draw at runtime).

## Prototype Pack v0

Seven assets before expanding. Generate:

```powershell
uv run python scripts/generate_living_nomabot_art.py
```

Reference copies: `assets/characters/nomabot/prototype_v0/`

Full pack (apartment v1 + 15 clips):

```powershell
uv run python scripts/generate_living_nomabot_art.py --full
```

## Export paths

| Asset | Source | Production |
|-------|--------|------------|
| Apartment | `concept/apartment.png` | `sprites/bg/office.png` → `bg_office` |
| Body poses | `sprites/body/body_*.png` | manifest body ids |
| Expressions | `sprites/face/face_*.png` | manifest `face_*` ids (expression layer) |
| Clips | `animations/*.json` | ClipPlayer |

Generate:

```powershell
uv run python scripts/generate_living_nomabot_art.py
uv run nomabot build-assets --input assets/characters/nomabot --output compiled/nomabot --profile lilygo_tdisplay_s3
uv run python scripts/copy_pack_to_firmware_data.py
```

## Device iteration loop

| Emotion (Brain) | Face sprite |
|-----------------|-------------|
| neutral | face_neutral |
| happy | face_happy |
| excited | face_happy |
| frustrated | face_angry |
| sleepy | face_sleepy |
| curious | face_thinking |

## Prototype v0 clips (four poses)

| Clip | Pose | Sprites |
|------|------|---------|
| idle | Standing | body_idle_01, body_idle_02 |
| blink | Blink | body_blink_01, body_stand |
| coding | Typing | body_typing_01, body_typing_02 |
| think | Thinking | body_think |

Expand to full clip set only after v0 passes HUD-hidden + 60s video QA on hardware.

## v1 clips (~15, post-v0)

| Group | Clip ids |
|-------|----------|
| Idle | idle, blink, look_left, look_right, stretch |
| Coding | coding, think, coffee, celebrate |
| Sleep | sleep, snore, yawn |
| Social | wave, hello, thumbs_up |

## Behavior clip map (prototype)

Standing (`idle`), blink, typing (`coding`), thinking (`think`) only. See trimmed `behavior.yaml` during Phase B LCD iteration.

## Engine (M5.1 ship slice)

- Expression overlay z=11 (`DirtyCharacter` + emotion change)
- Styled speech bubble z=40
- No prop Scene layers in v1 — apartment baked into `bg_office`
