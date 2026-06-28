# Prototype Pack v0

Seven assets for **First Living Character** (M5.1). Flash-test-adjust on the physical LCD before expanding.

| File | Role |
|------|------|
| body_stand.png | Home / idle |
| body_typing_01.png | Typing silhouette |
| body_think.png | Head tilt thinking |
| body_blink_01.png | Blink |
| face_neutral.png | Default expression |
| face_happy.png | Happy |
| face_thinking.png | Thinking overlay |
| hero_home.png | Reference composite at home anchor |

Generate fresh copies:

```powershell
uv run python scripts/generate_living_nomabot_art.py
```

Full pack (apartment v1 + expanded clips):

```powershell
uv run python scripts/generate_living_nomabot_art.py --full
```
