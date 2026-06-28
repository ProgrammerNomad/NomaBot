# NomaBot — Character Design Package

Phase 1 art deliverables. **Approve this package before Phase 2 (apartment production).**

## Sheets

| File | Content |
|------|---------|
| `sheet_01_hero.png` | Front, left, right, back, head close-up, height guide, palette |
| `sheet_02_expressions.png` | 12 face expressions |
| `sheet_03_personality.md` | Likes, dislikes, habits |

Regenerate reference sheets:

```powershell
cd C:\xampp\htdocs\NomaBot
uv run python scripts/generate_living_nomabot_art.py --concept-only
```

Production sprites (170×320 apartment, poses, faces):

```powershell
uv run python scripts/generate_living_nomabot_art.py
```

## Approval checklist

- [ ] Silhouette reads at 32px thumbnail
- [ ] Camera is 3/4 front or front only
- [ ] Palette uses only STYLE_GUIDE colors
- [ ] 12 expressions approved
- [ ] Personality sheet matches LORE.md
