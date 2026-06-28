"""Emulator sprite mode scene painting tests."""

from nomabot.render import DirtyFlags, RenderState
from nomabot.render.scene import SceneBuilder


def test_sprite_scene_node_count_with_overlay() -> None:
    state = RenderState(
        behavior_label="Typing...",
        overlay_text="Hello",
        background_sprite_id="bg_office",
        body_sprite_id="body_idle_01",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.FULL)
    assert scene.node_count == 5
    assert scene.background.z < scene.character.z < scene.hud.z < scene.speech_bubble.z


def test_sprite_scene_dirty_character_only() -> None:
    state = RenderState(
        body_sprite_id="body_coding_01",
        background_sprite_id="bg_office",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.CHARACTER)
    assert scene.character.dirty
    assert not scene.background.dirty
