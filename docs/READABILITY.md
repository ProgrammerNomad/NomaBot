# Readability Rules — First Living Character

> Design law for NomaBot at **170×320**. The physical LCD is the spec.

## Primary rule

**If a behavior cannot be recognized with the HUD hidden, the animation is not good enough.**

Eventually behavior labels turn off. Typing, thinking, sleepy, and celebrating must read from silhouette alone.

## Tests

### 1-meter test

Stand one meter from your desk. Can you instantly tell:

- Is NomaBot typing?
- Is NomaBot thinking?
- Is NomaBot idle?

If not, increase **silhouette contrast** before polishing eyes or fingers.

### HUD-hidden test

Cover the top behavior label. Does the pose still communicate?

### Silhouette first

Animate in this order:

1. Body silhouette
2. Head angle
3. Arm position
4. Expression overlay
5. Micro details (eyes, fingers) — last

Example — thinking reads from head tilt alone:

```text
Idle          Thinking
  O             O↗
 /|\           /|\
 / \           / \
```

## Home position

Body anchor: **(85, 80)**. Expression sits at **body + (0, 24)** (Phase B). All behaviors return to the same standing silhouette at the body anchor.

## Video test (60 seconds)

After sprite or anchor changes, record **60 seconds** of autonomous behavior and review the recording (catches issues live viewing misses):

- Did blink register?
- Did thinking head tilt read?
- Did typing feel different from idle?
- One visor, one set of eyes — no black box around expression
- Any moment of "why is he alive?" (good) vs "what is that glitch?" (bad)

## Device iteration loop

After every sprite change:

```powershell
uv run python scripts/generate_living_nomabot_art.py
uv run nomabot build-assets --input assets/characters/nomabot --output compiled/nomabot --profile lilygo_tdisplay_s3
uv run python scripts/copy_pack_to_firmware_data.py
cd firmware
pio run -e lilygo_tdisplay_s3 -t uploadfs
```

Look at the **physical device**, not the monitor. Adjust and repeat.

## Prototype Pack v0 gate

Before expanding to full bible / apartment / 15+ behaviors, v0 must pass on hardware:

- Standing, typing, thinking, blink distinct at 1 m with HUD hidden
- `diagnostics` shows `clip` and `body_sprite_id` changing with behavior

See [18_BUILD.md](./18_BUILD.md), [LIVING_NOMA_SPEC.md](./LIVING_NOMA_SPEC.md), and iteration log [PHASE_B_NOTES.md](./PHASE_B_NOTES.md).
