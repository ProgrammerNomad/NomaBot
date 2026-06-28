"""SceneBuilder tests."""

from nomabot.render import DirtyFlags, RenderState
from nomabot.render.scene import SceneBuilder, expression_for_emotion, scene_to_diagnostics


def test_scene_builder_office_coding() -> None:
    state = RenderState(
        life_mode="work",
        activity="coding",
        behavior_label="Typing...",
        background_sprite_id="bg_office",
        body_sprite_id="body_typing_01",
        emotion="neutral",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.FULL)
    assert scene.scene_id == "office"
    assert scene.background.sprite_id == "bg_office"
    assert scene.character.sprite_id == "body_typing_01"
    assert scene.expression.sprite_id == "face_neutral"
    assert scene.hud.text == "Typing..."
    assert scene.speech_bubble.visible is False
    assert scene.node_count == 4


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
    assert scene.node_count == 5


def test_expression_for_emotion_happy() -> None:
    assert expression_for_emotion("happy") == "face_happy"
    assert expression_for_emotion("frustrated") == "face_angry"


def test_scene_diagnostics() -> None:
    state = RenderState(
        overlay_text="Hello",
        body_sprite_id="body_typing_01",
        background_sprite_id="bg_office",
        behavior_label="",
        emotion="happy",
    )
    scene = SceneBuilder.build(state, dirty=DirtyFlags.FULL)
    diag = scene_to_diagnostics(scene)
    assert diag.scene == "office"
    assert diag.body == "body_typing_01"
    assert diag.eyes == "face_happy"
    assert diag.overlay == "Hello"
    assert diag.render_objects == 4


def test_expression_anchor_relative_to_body() -> None:
    state = RenderState(
        body_sprite_id="body_idle_01",
        background_sprite_id="bg_office",
    )
    scene = SceneBuilder.build(state, anchor_x=85, anchor_y=80, expression_dx=0, expression_dy=24)
    assert scene.character.x == 85
    assert scene.character.y == 80
    assert scene.expression.x == 85
    assert scene.expression.y == 104


def test_dirty_character_marks_character_and_expression() -> None:
    state = RenderState(body_sprite_id="body_idle_01", background_sprite_id="bg_office")
    scene = SceneBuilder.build(state, dirty=DirtyFlags.CHARACTER)
    assert scene.character.dirty is True
    assert scene.expression.dirty is True
    assert scene.background.dirty is False


def test_dirty_background_also_marks_character() -> None:
    state = RenderState(body_sprite_id="body_idle_01", background_sprite_id="bg_office")
    scene = SceneBuilder.build(state, dirty=DirtyFlags.BACKGROUND)
    assert scene.background.dirty is True
    assert scene.character.dirty is True
    assert scene.expression.dirty is True
