# Testing Strategy

> **Status:** Design specification - implement alongside Milestone 1.5 (Developer Experience).

## Overview

Testing is not an afterthought. NomaBot spans desktop Python, ESP32 firmware, compiled binaries, JSON protocol, plugins, and visual output-each layer needs explicit test types and CI gates.

This document is the **single source of truth** for testing. Individual docs link here instead of duplicating strategy.

```text
                    CI Pipeline
                         │
    ┌────────────────────┼────────────────────┐
    ▼                    ▼                    ▼
 Unit tests      Integration tests     Hardware / E2E
 (fast, mocked)  (protocol, runtime)   (nightly, manual)
```

## Test pyramid

| Layer | Scope | Speed | Runs on |
|-------|-------|-------|---------|
| **Unit** | Pure functions, parsers, graph logic | ms | Every PR |
| **Integration** | Runtime + mock device, SDK client | seconds | Every PR |
| **Protocol conformance** | Golden NDJSON fixtures | seconds | Every PR |
| **Golden render** | Framebuffer hash vs baseline PNG | seconds–minutes | PR + nightly |
| **Firmware HW** | Flash + serial loopback | minutes | Nightly / manual |
| **E2E desktop** | Tray smoke, settings open | minutes | Nightly |

## Unit tests

### Desktop (`desktop/tests/`)

| Target | Examples |
|--------|----------|
| Event bus | Publish/subscribe, immutability, unknown topic logging |
| SchedulerService | Cron parsing, fire events, SQLite persistence |
| Render coalescer | Priority merge, layer conflict resolution |
| Config migrations | v0 → v1 settings upgrade |
| Permission gate | Plugin denied without `internet` |

**Rule:** Services tested with mocked bus-never import sibling services in tests either.

### SDK (`sdk/tests/`)

| Target | Examples |
|--------|----------|
| `nomabot.protocol` | Envelope build/parse, `v` required |
| `CharacterValidator` | Broken refs, orphan graph states |
| `AssetCompiler` | PNG → manifest size budget |
| `AnimationGraph` | Transition evaluation, timeout edges |

### Firmware (`firmware/test/` host-side)

| Target | Examples |
|--------|----------|
| JSON parser | Malformed lines, oversize payload |
| Graph engine | State transitions, blend timing |
| Renderer mock | Blit call sequence |

Use **host-compiled** test binaries (PlatformIO `native` or Google Test) where possible-no hardware required.

## Integration tests

### Noma Runtime + mock device

```python
async with MockDevice(profile="lilygo_tdisplay_s3") as dev:
    runtime = NomaRuntime(devices=[dev])
    await runtime.submit(RenderRequest(animation="idle", priority=Priority.NORMAL))
    assert dev.last_command["cmd"] == "play_animation"
```

Mock device implements Transport interface and records commands-see Milestone 1.5.

### Desktop ↔ SDK contract

Integration tests import **only** `nomabot-sdk` public API, not `desktop/core/` internals. Ensures SDK stays honest.

### Plugin contract tests

Sample plugin in `sdk/plugin/tests/fixtures/`:

- Load manifest → enable → receive fixture event → assert `RenderRequest` emitted
- Missing permission → `PermissionError`

## Protocol tests

Golden fixture files in `sdk/firmware/test_vectors/`:

```text
test_vectors/
├── hello.json
├── play_animation.json
├── invalid_version.json
├── sync_chunk_sequence/
│   ├── 01_sync_begin.json
│   ├── 02_sync_chunk_0.json
│   └── 03_sync_activate.json
```

CI runs:

```bash
nomabot protocol lint test_vectors/
nomabot firmware test-vectors --host   # parser-only mode
```

Firmware build must pass **100% host parser tests** before HW tests run.

## Firmware simulation

| Mode | Purpose |
|------|---------|
| **Mock renderer** | Headless framebuffer; no LovyanGFX HW |
| **Desktop emulator** | PySide6 window mimics 170×320 device ([Developer Experience](./10_ROADMAP.md)) |
| **Record/replay** | Capture serial session; replay in CI |

Emulator displays last frame + active graph state for debugging plugins without hardware.

## Golden image rendering

Visual regression for animation engine:

1. Load compiled pack fixture
2. Step graph through scripted events
3. Render via `PreviewRenderer` (SDK) or firmware mock
4. Compare SHA-256 of RGB565 buffer to committed baseline
5. Fail CI on drift; update baselines via explicit `accept-golden` PR label

Store baselines in `sdk/character/tests/golden/`-small 170×320 frames only.

Tolerance: optional per-pixel threshold for compression variants (document in PR).

## Character validation tests

Every pack in `nomabot-assets` CI:

```bash
nomabot character validate characters/nomabot
nomabot build-assets --input characters/nomabot --output /tmp/out --profile lilygo_tdisplay_s3
nomabot character validate /tmp/out --compiled
```

Gates:

- Schema valid
- Graph reachable
- Size budget
- Personality block present (see [Character System](./06_CHARACTER_SYSTEM.md))

## Plugin tests

| Check | Command / location |
|-------|-------------------|
| Manifest schema | `nomabot plugin lint ./plugins/git` |
| Unit handlers | `pytest plugins/git/tests` |
| Permission declaration | Fixture attempts forbidden API → fail |
| No network in CI | `responses` / `httpx` mocks |

Bundled plugins: minimum 80% handler coverage before merge (target).

## CI pipeline (target)

```yaml
# Conceptual GitHub Actions jobs
jobs:
  sdk-unit:          # pytest sdk/
  desktop-unit:      # pytest desktop/tests (no GUI)
  protocol-lint:     # golden vectors
  firmware-host:     # native parser + graph tests
  compile-assets:    # all official packs → lilygo_tdisplay_s3 profile
  golden-render:     # PreviewRenderer baselines
  plugin-contract:   # fixture plugins
  desktop-smoke:     # nightly: launch headless tray 30s
  firmware-hw:       # nightly: serial test on self-hosted runner (optional)
```

Path filters: firmware jobs skip on docs-only PRs.

## Local developer workflow

```bash
# Fast loop (no hardware)
pytest sdk/tests desktop/tests
nomabot protocol lint sdk/firmware/test_vectors/
nomabot mock-device --profile lilygo_tdisplay_s3   # Milestone 1.5

# With hardware
nomabot firmware test-vectors --port COM7
```

## Coverage targets (pre-1.0)

| Area | Target |
|------|--------|
| SDK protocol + validator | ≥ 90% |
| Noma Runtime render queue | ≥ 85% |
| Plugin SDK base | ≥ 80% |
| Firmware host tests | All command handlers |
| UI | Smoke only (no % gate) |

## Related documentation

- [Architecture - Noma Runtime](./01_ARCHITECTURE.md#noma-runtime)
- [SDK - Testing module](./12_SDK.md)
- [Security - verification tests](./17_SECURITY.md)
- [Roadmap - Milestone 1.5](./10_ROADMAP.md)
- [Contributing](./CONTRIBUTING.md)
