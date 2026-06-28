"""DirtyTracker tests."""

from nomabot.render import DirtyFlags, DirtyTracker, RenderState, quantize_energy


def test_quantize_energy_buckets() -> None:
    assert quantize_energy(81) == 80
    assert quantize_energy(80) == 80
    assert quantize_energy(74) == 75


def test_no_dirty_when_unchanged() -> None:
    tracker = DirtyTracker()
    state = RenderState(behavior_label="Typing...")
    tracker.commit_rendered(state)
    assert tracker.collect_dirty_flags(state) == DirtyFlags.NONE


def test_behavior_dirty_on_label_change() -> None:
    tracker = DirtyTracker()
    before = RenderState(behavior_label="Typing...")
    after = RenderState(behavior_label="Coffee")
    tracker.commit_rendered(before)
    dirty = tracker.collect_dirty_flags(after)
    assert DirtyFlags.BEHAVIOR in dirty


def test_energy_not_dirty_within_bucket() -> None:
    tracker = DirtyTracker()
    before = RenderState(energy=81)
    after = RenderState(energy=80)
    tracker.commit_rendered(before)
    dirty = tracker.collect_dirty_flags(after)
    assert DirtyFlags.ENERGY not in dirty


def test_energy_dirty_crossing_bucket() -> None:
    tracker = DirtyTracker()
    before = RenderState(energy=80)
    after = RenderState(energy=74)
    tracker.commit_rendered(before)
    dirty = tracker.collect_dirty_flags(after)
    assert DirtyFlags.ENERGY in dirty


def test_forced_dirty_without_state_change() -> None:
    tracker = DirtyTracker()
    state = RenderState(behavior_label="Typing...")
    tracker.commit_rendered(state)
    tracker.force_dirty(DirtyFlags.BEHAVIOR)
    dirty = tracker.collect_dirty_flags(state)
    assert DirtyFlags.BEHAVIOR in dirty


def test_pending_dirty_merged() -> None:
    tracker = DirtyTracker()
    state = RenderState(behavior_label="Typing...")
    tracker.commit_rendered(state)
    tracker.note_pending(DirtyFlags.MESSAGE)
    dirty = tracker.collect_dirty_flags(state)
    assert DirtyFlags.MESSAGE in dirty


def test_forced_and_pending_cleared_after_collect() -> None:
    tracker = DirtyTracker()
    state = RenderState(behavior_label="Typing...")
    tracker.commit_rendered(state)
    tracker.force_dirty(DirtyFlags.BEHAVIOR)
    tracker.note_pending(DirtyFlags.MESSAGE)
    tracker.collect_dirty_flags(state)
    dirty = tracker.collect_dirty_flags(state)
    assert dirty == DirtyFlags.NONE
