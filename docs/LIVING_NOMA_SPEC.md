# Living NomaBot — Production Spec (M5.1 First Living Character)

> Canvas, anchors, clips, export paths. See [READABILITY.md](./READABILITY.md), [STYLE_GUIDE.md](./STYLE_GUIDE.md), [LORE.md](./LORE.md).

## Canvas

- Display: **170×320** (LILYGO T-Display S3)
- **Home position** (head anchor): `(85, 80)` — all behaviors return here
- Hands anchor: `(85, 200)`

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
| Faces | `sprites/face/face_*.png` | manifest `face_*` ids |
| Clips | `animations/*.json` | ClipPlayer |

Generate:

```powershell
uv run python scripts/generate_living_nomabot_art.py
uv run nomabot build-assets --input assets/characters/nomabot --output compiled/nomabot --profile lilygo_tdisplay_s3
uv run python scripts/copy_pack_to_firmware_data.py
```

## Expression map (`config.json`)

| Emotion (Brain) | Face sprite |
|-----------------|-------------|
| neutral | face_neutral |
| happy | face_happy |
| excited | face_happy |
| frustrated | face_angry |
| sleepy | face_sleepy |
| curious | face_thinking |

## v1 clips (~15)

| Group | Clip ids |
|-------|----------|
| Idle | idle, blink, look_left, look_right, stretch |
| Coding | coding, think, coffee, celebrate |
| Sleep | sleep, snore, yawn |
| Social | wave, hello, thumbs_up |

## Behavior clip map

Idle behaviors use short clips (`blink`, `look_left`, `wave`, etc.). Coding uses `coding`, `think`, `coffee`, `celebrate`. Sleep uses `sleep`, `snore`, `yawn`.

## Engine (M5.1 ship slice)

- Expression overlay z=11 (`DirtyCharacter` + emotion change)
- Styled speech bubble z=40
- No prop Scene layers in v1 — apartment baked into `bg_office`
