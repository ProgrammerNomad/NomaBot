"""Desktop eyes branch render mode preservation tests."""

from types import SimpleNamespace

from nomabot_desktop.core.lifecycle import _sync_emulator_state
from nomabot_desktop.core.state_manager import BotState
from nomabot_desktop.transport import EmulatorState


def test_sync_emulator_state_preserves_eyes_render_mode():
    emu = EmulatorState(width=320, height=170)
    emu.render_mode = "eyes"
    emu.character_id = "eyes"
    ctx = SimpleNamespace(emu_state=emu)

    state = BotState(name="coding", activity="coding", emotion="happy", life_mode="work", animation="focus")
    _sync_emulator_state(ctx, state)  # type: ignore[arg-type]

    assert emu.render_mode == "eyes"
    assert emu.activity == "coding"
    assert emu.animation == "focus"
