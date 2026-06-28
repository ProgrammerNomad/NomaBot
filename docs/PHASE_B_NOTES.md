# Phase B — Device Iteration Notes

> LCD-first tuning log for Prototype Pack v0. Source for Phase C Character Bible.

**Device:** LILYGO T-Display S3 (170×320), COM3  
**Pack:** pruned v0 — 7 body, 4 face, 4 clips  
**Version target:** 0.5.3 after 3+ desk days pass exit gate

## Final constants (Phase B pass — 2026-06-28)

| Parameter | Value | File |
|-----------|-------|------|
| `anchors.expression.dy` | **24** | `assets/characters/nomabot/config.json` |
| `anchors.expression.dx` | 0 | `config.json` |
| Expression eye row `cy` | **11** | `scripts/generate_living_nomabot_art.py` |
| Think head tilt | **11** px | `_draw_robot_body` |
| Helmet width | ±21 px | `_draw_helmet` |
| Typing arm reach | ±36, oy+42 | `_draw_robot_body` |
| Standing arms | oy+42 hang | `_draw_robot_body` |
| Blink visor close | hx±16 full band | `_draw_robot_body` |

**Alignment math:** body blit uses anchor Y as sprite **top**; visor center ≈ bodyY+34. Eyes at expression_top+cy → dy=24 places eye row at screen Y≈115 (visor band 110–118).

---

## Iteration log

| Date | Change | Constant | Pass/Fail | Notes (1m / HUD-hidden / video) |
|------|--------|----------|-----------|----------------------------------|
| 2026-06-28 | Baseline pruned v0 on device | dy=28, cy=10, tilt=8 | Partial | User: alignment/proportions off; poses mostly OK |
| 2026-06-28 | Expression anchor | dy **28→24** | Pending LCD | Geometry: align eyes to visor center |
| 2026-06-28 | Eye row in sprite | cy **10→11** | Pending LCD | Fine-tune within visor band |
| 2026-06-28 | Think silhouette | tilt **8→11**, hand higher | Pending LCD | HUD-hidden think read |
| 2026-06-28 | Typing vs standing | arms ±36 oy+42 vs hang oy+42 | Pending LCD | 1m silhouette contrast |
| 2026-06-28 | Blink read | visor hx±16 close | Pending LCD | 60s video: blink register |
| 2026-06-28 | Helmet at 1m | ellipse ±21 | Pending LCD | Head presence at distance |

---

## Structural QA (run once per flash)

- [ ] One visor, one set of eyes
- [ ] No black box around expression
- [ ] `diagnostics`: `clip` and `body_sprite_id` change idle ↔ coding

## Readability QA (repeat until pass)

- [ ] **1-meter test:** standing vs typing vs thinking vs blink distinct
- [ ] **HUD-hidden test:** cover behavior label — pose communicates
- [ ] **Home position:** returns to standing at (85, 80)
- [ ] **60s video:** blink, think tilt, typing ≠ idle; no glitches

### Dev panel (desktop COM3)

| Activity | Expected clips | Expected silhouette |
|----------|------------------|---------------------|
| idle | idle, blink | Standing arms / visor close |
| coding | coding, think | Typing arms / head tilt |

---

## 60-second video test procedure

1. Leave device in autonomous mode (desktop connected, life_mode work).
2. Record phone video for **60 seconds** at desk distance (~1 m).
3. Review recording (not live view):
   - Did blink register?
   - Did thinking head tilt read?
   - Did typing feel different from idle?
   - One visor, one eyes — no expression black box?
   - Any "why is he alive?" vs "what is that glitch?" moments
4. Log pass/fail in iteration table above.

---

## Phase C exit gate (user sign-off)

Do **not** start Character Bible until **3+ consecutive desk days** with all readability QA checked.

When signed off:

- [ ] Bump `VERSION` and `firmware/platformio.ini` to **0.5.3** (applied in repo; confirm on device)
- [ ] Freeze proportions from "Final constants" into `assets/characters/nomabot/bible/`

---

## Flash loop

```powershell
uv run python scripts/generate_living_nomabot_art.py
uv run nomabot build-assets --input assets/characters/nomabot --output compiled/nomabot --profile lilygo_tdisplay_s3
uv run python scripts/copy_pack_to_firmware_data.py
cd firmware
pio run -e lilygo_tdisplay_s3 -t uploadfs --upload-port COM3
```
