"""Eyes render mode scene tests."""

from nomabot.render import DirtyFlags, RenderState
from nomabot.render.scene import SceneBuilder


def test_eyes_mode_hides_body_shows_expression_sprite() -> None:
    state = RenderState(
        body_sprite_id="eyes_neutral",
        background_sprite_id="bg_dark",
        clock_text="22:41",
        weather_text="28C Clear",
    )
    scene = SceneBuilder.build(state, eyes_only=True, dirty=DirtyFlags.FULL)
    assert scene.character.visible is False
    assert scene.expression.visible is True
    assert scene.expression.sprite_id == "eyes_neutral"
    assert scene.ambient_bar.visible is True
    assert scene.ambient_bar.id == "22:41"
    assert scene.node_count == 4


def test_eyes_blink_sprite_from_clip() -> None:
    state = RenderState(
        body_sprite_id="eyes_blink",
        background_sprite_id="bg_dark",
    )
    scene = SceneBuilder.build(state, eyes_only=True, dirty=DirtyFlags.FULL)
    assert scene.expression.sprite_id == "eyes_blink"
