"""DirtyTracker sprite field tests."""

from nomabot.render import DirtyFlags, DirtyTracker, RenderState


def test_character_dirty_on_body_sprite_change() -> None:
    tracker = DirtyTracker()
    before = RenderState(body_sprite_id="body_idle_01", clip_frame_index=0)
    after = RenderState(body_sprite_id="body_idle_02", clip_frame_index=1)
    tracker.commit_rendered(before)
    dirty = tracker.collect_dirty_flags(after)
    assert DirtyFlags.CHARACTER in dirty


def test_sprite_dirty_on_emotion_change() -> None:
    tracker = DirtyTracker()
    before = RenderState(emotion="neutral")
    after = RenderState(emotion="happy")
    tracker.commit_rendered(before)
    dirty = tracker.collect_dirty_flags(after)
    assert DirtyFlags.CHARACTER in dirty
    assert DirtyFlags.META in dirty


def test_background_dirty_on_sprite_change() -> None:
    tracker = DirtyTracker()
    before = RenderState(background_sprite_id="bg_office")
    after = RenderState(background_sprite_id="bg_travel")
    tracker.commit_rendered(before)
    dirty = tracker.collect_dirty_flags(after)
    assert DirtyFlags.BACKGROUND in dirty
