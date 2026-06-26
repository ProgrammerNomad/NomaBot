# Asset Pipeline

> **Status:** Design specification - build before large character catalog.

## Overview

NomaBot treats assets like a game engine: **authors create PNGs; the platform compiles, validates, streams, and activates** device-ready binaries. Manual copying of PNGs to ESP32 flash is not supported in production workflows.

```text
PNG / PSD / Aseprite
        ↓
Character Editor (optional) + JSON graphs
        ↓
nomabot build-assets          ← CLI (nomabot-sdk)
        ↓
RGB565 · RLE/LZ compression · manifest.json
        ↓
Validate + semver + hardware profile check
        ↓
USB sync · chunk upload · OTA asset bundle
        ↓
ESP32 filesystem → Animation Engine
```

## Asset compiler CLI

Primary command:

```bash
nomabot build-assets --input ./characters/nomabot --output ./compiled/nomabot --profile esp32_240
```

### Pipeline stages

| Stage | Output |
|-------|--------|
| **Ingest** | Normalized PNGs, power-of-two hints, alpha handling |
| **Convert** | RGB565 (or profile-specific) raw tiles |
| **Compress** | RLE, LZ4, or profile-specific codec |
| **Atlas** | Optional sprite sheets + UV index |
| **Manifest** | `manifest.json` - ids, offsets, sizes, hashes |
| **Report** | Size budget, warnings, errors |

### Example manifest (compiled)

```json
{
  "pack_id": "nomabot",
  "version": "1.0.0",
  "profile": "esp32_240",
  "protocol_min": 1,
  "total_bytes": 524288,
  "sprites": [
    { "id": "body/coding_01", "file": "sprites/body_01.bin", "hash": "sha256:…", "w": 64, "h": 64 }
  ],
  "animations_ref": "../animations/",
  "graph_ref": "animation_graph.json"
}
```

### Size budgets

| Profile | Max pack size (guideline) |
|---------|---------------------------|
| `esp32_240` | 2 MB flash budget default |
| `esp32_240_sd` | 16 MB with SD card |

Compiler **fails** on budget exceed unless `--force`.

### Integration points

| Consumer | Usage |
|----------|--------|
| `CharacterService` | Runs compiler on install/upgrade |
| CI (`nomabot-assets`) | Validates all official packs on PR |
| Character Editor | Export invokes same compiler backend |
| Firmware | Loads only compiled manifests |

## Character Editor

A **PySide6 authoring tool**-“Unity Animator, but tiny”-shipped with desktop or standalone from SDK.

### Features

| Feature | Description |
|---------|-------------|
| Import PNG | Single frames or strip sheets |
| Timeline | Drag frames, set FPS per clip |
| Animation graph | Visual nodes: states + transition rules |
| Layer preview | Character + accessory + background + bubble |
| Anchor editor | Place hands/head/desk pivots on sprite |
| Validate | Schema + budget check inline |
| Export | Pack folder + run compiler |

### Theme Editor (planned)

Companion tool for theme authors (Milestone 4):

- Visual palette + font + bubble style editor
- Preview on 170×320 canvas (LILYGO profile)
- Export → `nomabot build-theme`

Shares preview infrastructure with Character Editor. See [Platform](./13_PLATFORM.md#theme-editor-planned).

### Workflow

```text
1. New pack from template (Character SDK)
2. Import art → arrange clips
3. Draw animation graph (Idle ↔ Typing ↔ Thinking)
4. Preview at target resolution (240×240)
5. Export → nomabot build-assets → install to device
```

Artists should **never** hand-edit animation graph JSON for routine work-only for merge/conflict resolution.

Editor shares libraries with `sdk/character/`; see [SDK](./12_SDK.md).

## Asset streaming (chunk upload)

Large packs cannot upload as one blob. Protocol supports **chunked sync**:

```text
sync_begin   { pack_id, version, total_bytes, chunk_size, hash }
sync_chunk   { offset, data_b64, chunk_hash }
sync_verify  { pack_id, manifest_hash }
sync_activate { pack_id }   ← atomic switch
sync_abort   { pack_id }
```

| Capability | Purpose |
|------------|---------|
| **Chunked** | 4–64 KB chunks per message |
| **Verify** | SHA-256 per chunk + whole manifest |
| **Resume** | `sync_begin` returns last acked offset |
| **Activate** | Swap active pack only after full verify |

Desktop `CharacterService` tracks upload state per device in SQLite.

On failure: device keeps previous pack; partial staging partition erased on `sync_abort`.

Full command schemas: [Communication](./04_COMMUNICATION.md#asset-streaming).

## Authoring source layout

Source packs (human-editable) vs compiled output (device):

```text
characters/nomabot/          ← source (Git-friendly)
├── config.json
├── metadata.json
├── animation_graph.json
├── animations/
├── sprites/                 ← PNG sources
└── preview.png

compiled/nomabot/            ← generated (Gitignored locally)
├── manifest.json
├── sprites/*.bin
└── pack.nomabundle          ← optional single-file artifact
```

Official packs in `nomabot-assets` repo store **sources**; CI produces release artifacts.

## Theme compilation

Themes follow a lighter pipeline:

```text
theme.json + fonts + icons + palette
        ↓
nomabot build-theme
        ↓
compiled theme bundle
```

Themes do not recompile character body sprites; they override tokens consumed by bubble renderer and UI. See [Platform & Ecosystem](./13_PLATFORM.md).

## Quality gates

Compiler and CI enforce:

- [ ] All sprite refs resolve
- [ ] Animation graph reachable from `default_state`
- [ ] No orphan states
- [ ] Transition durations within limits
- [ ] UTF-8 message font coverage for bundled strings
- [ ] `min_firmware` / profile compatibility

## Related documentation

- [Character System](./06_CHARACTER_SYSTEM.md)
- [Animation Engine](./05_ANIMATION_ENGINE.md)
- [Communication](./04_COMMUNICATION.md)
- [SDK](./12_SDK.md)
- [Firmware](./03_FIRMWARE.md)
