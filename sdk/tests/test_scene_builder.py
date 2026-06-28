"""SceneBuilder tests."""

from nomabot.render import DirtyFlags, RenderState
from nomabot.render.scene import SceneBuilder, scene_to_diagnostics


def test_scene_builder_office_coding() -> None:
    state = RenderState(
        life_mode="work",
        activity="coding",
        behavior_label="Typing...",
        background_sprite_id="bg_office",
        body_sprite_id="body_coding_01",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.FULL)
    assert scene.scene_id == "office"
    assert scene.background.sprite_id == "bg_office"
    assert scene.character.sprite_id == "body_coding_01"
    assert scene.hud.text == "Typing..."
    assert scene.speech_bubble.visible is False
    assert scene.node_count == 3


def test_scene_builder_overlay_visible() -> None:
    state = RenderState(
        overlay_text="Hello",
        behavior_label="Breathing...",
        background_sprite_id="bg_office",
        body_sprite_id="body_idle_01",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.FULL)
    assert scene.speech_bubble.visible is True
    assert scene.speech_bubble.text == "Hello"
    assert scene.node_count == 4


def test_scene_diagnostics() -> None:
    state = RenderState(
        overlay_text="Hello",
        body_sprite_id="typing_03",
        background_sprite_id="bg_office",
        behavior_label="",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.FULL)
    diag = scene_to_diagnostics(scene)
    assert diag.scene == "office"
    assert diag.body == "typing_03"
    assert diag.overlay == "Hello"
    assert diag.render_objects == 3


def test_dirty_character_marks_character_only() -> None:
    state = RenderState(body_sprite_id="body_idle_01", background_sprite_id="bg_office")
    scene = SceneBuilder.build(state, dirty=DirtyFlags.CHARACTER)
    assert scene.character.dirty is True
    assert scene.background.dirty is False
    assert scene.hud.dirty is False


def test_dirty_background_also_marks_character() -> None:
    state = RenderState(body_sprite_id="body_idle_01", background_sprite_id="bg_office")
    scene = SceneBuilder.build(state, dirty=DirtyFlags.BACKGROUND)
    assert scene.background.dirty is True
    assert scene.character.dirty is True
