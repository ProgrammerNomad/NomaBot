"""Character renderer dirty band mapping tests."""

from nomabot.render import DirtyFlags, RenderState
from nomabot.render.scene import SceneBuilder, apply_dirty_flags


def test_dirty_message_only_bubble() -> None:
    state = RenderState(
        behavior_label="Typing...",
        overlay_text="Hi",
        body_sprite_id="body_idle_01",
        background_sprite_id="bg_office",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.NONE)
    apply_dirty_flags(scene, DirtyFlags.MESSAGE)
    assert scene.speech_bubble.dirty is True
    assert scene.character.dirty is False
    assert scene.hud.dirty is False


def test_dirty_behavior_only_hud() -> None:
    state = RenderState(
        behavior_label="Typing...",
        body_sprite_id="body_idle_01",
        background_sprite_id="bg_office",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.NONE)
    apply_dirty_flags(scene, DirtyFlags.BEHAVIOR)
    assert scene.hud.dirty is True
    assert scene.character.dirty is False


def test_dirty_full_all_visible_nodes() -> None:
    state = RenderState(
        behavior_label="Typing...",
        overlay_text="Hi",
        body_sprite_id="body_idle_01",
        background_sprite_id="bg_office",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.FULL)
    assert scene.background.dirty
    assert scene.character.dirty
    assert scene.expression.dirty
    assert scene.hud.dirty
    assert scene.speech_bubble.dirty
