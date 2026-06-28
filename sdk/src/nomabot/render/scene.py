"""Scene graph mirror for emulator, tests, and diagnostics."""

from __future__ import annotations

from dataclasses import dataclass, field

from nomabot.render import DirtyFlags, RenderState, has_dirty

SCENE_Z_BACKGROUND = 0
SCENE_Z_CHARACTER = 10
SCENE_Z_PROP = 20
SCENE_Z_HUD = 30
SCENE_Z_SPEECH_BUBBLE = 40


@dataclass
class SceneNode:
    id: str | None = None
    sprite_id: str | None = None
    text: str | None = None
    x: int = 0
    y: int = 0
    z: int = 0
    visible: bool = False
    dirty: bool = False


@dataclass
class Scene:
    scene_id: str = "office"
    background: SceneNode = field(default_factory=SceneNode)
    character: SceneNode = field(default_factory=SceneNode)
    hud: SceneNode = field(default_factory=SceneNode)
    speech_bubble: SceneNode = field(default_factory=SceneNode)
    node_count: int = 0


@dataclass
class SceneDiagnostics:
    scene: str = "office"
    body: str = ""
    eyes: str = ""
    overlay: str = ""
    render_objects: int = 0


def scene_id_from_background(bg_sprite_id: str | None) -> str:
    if not bg_sprite_id:
        return "office"
    if bg_sprite_id.startswith("bg_"):
        return bg_sprite_id[3:]
    return bg_sprite_id


def scene_visible_node_count(scene: Scene) -> int:
    count = 0
    for node in (scene.background, scene.character, scene.hud, scene.speech_bubble):
        if node.visible:
            count += 1
    return count


def scene_to_diagnostics(scene: Scene) -> SceneDiagnostics:
    return SceneDiagnostics(
        scene=scene.scene_id or "office",
        body=scene.character.id or "" if scene.character.visible else "",
        eyes="",
        overlay=scene.speech_bubble.text or "" if scene.speech_bubble.visible else "",
        render_objects=scene_visible_node_count(scene),
    )


def _mark_node_dirty(node: SceneNode, flag: bool) -> None:
    node.dirty = flag and node.visible


def apply_dirty_flags(scene: Scene, dirty: DirtyFlags) -> None:
    if dirty == DirtyFlags.FULL:
        for node in (scene.background, scene.character, scene.hud, scene.speech_bubble):
            _mark_node_dirty(node, True)
        return

    _mark_node_dirty(scene.background, has_dirty(dirty, DirtyFlags.BACKGROUND))
    _mark_node_dirty(scene.character, has_dirty(dirty, DirtyFlags.CHARACTER))
    _mark_node_dirty(scene.hud, has_dirty(dirty, DirtyFlags.BEHAVIOR))
    if has_dirty(dirty, DirtyFlags.MESSAGE):
        scene.speech_bubble.dirty = True

    if has_dirty(dirty, DirtyFlags.BACKGROUND) and scene.character.visible:
        scene.character.dirty = True


class SceneBuilder:
    """Maps RenderState + pack defaults to a Scene graph."""

    @staticmethod
    def build(
        state: RenderState,
        *,
        default_background: str = "bg_office",
        anchor_x: int = 85,
        anchor_y: int = 80,
        dirty: DirtyFlags = DirtyFlags.FULL,
    ) -> Scene:
        bg_sprite = state.background_sprite_id or default_background
        body_sprite = state.body_sprite_id or "body_idle_01"
        label = state.behavior_label or ""
        overlay = state.overlay_text or ""

        scene = Scene(scene_id=scene_id_from_background(bg_sprite))
        scene.background = SceneNode(
            id=scene.scene_id,
            sprite_id=bg_sprite,
            x=0,
            y=0,
            z=SCENE_Z_BACKGROUND,
            visible=bool(bg_sprite),
        )
        scene.character = SceneNode(
            id=body_sprite,
            sprite_id=body_sprite,
            x=anchor_x,
            y=anchor_y,
            z=SCENE_Z_CHARACTER,
            visible=bool(body_sprite),
        )
        scene.hud = SceneNode(
            id="hud",
            text=label,
            x=4,
            y=8,
            z=SCENE_Z_HUD,
            visible=bool(label),
        )
        scene.speech_bubble = SceneNode(
            id="overlay" if overlay else "speech_bubble",
            text=overlay,
            x=4,
            y=0,
            z=SCENE_Z_SPEECH_BUBBLE,
            visible=bool(overlay),
        )
        scene.node_count = scene_visible_node_count(scene)
        apply_dirty_flags(scene, dirty)
        return scene
