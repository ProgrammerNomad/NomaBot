# ADR 0003: Renderer abstraction in firmware

**Status:** Accepted  
**Date:** 2025-06 (planning)

## Context

Official prototype hardware is **LILYGO T-Display S3** (170×320 ST7789). Community may port to ESP32-2432 boards, OLED, AMOLED, or future HDMI targets. Tying animation code to one LovyanGFX board config creates fork pressure.

## Decision

Introduce a **Renderer interface** in firmware. Animation engine blits to abstract framebuffer; concrete renderers (`LilygoTDisplayS3Renderer`, `St7789Renderer`, …) flush to hardware via LovyanGFX.

Hardware profile id (e.g. `lilygo_tdisplay_s3`) selects renderer + asset compile profile at build time.

## Consequences

**Positive**

- New displays = new renderer backend, not fork of graph engine
- Asset compiler targets profiles with explicit resolution budgets
- Host-side mock renderer enables golden tests without hardware

**Negative**

- Thin abstraction layer to maintain
- Each new board needs pin config + QA

## References

- [Firmware](../03_FIRMWARE.md)
- [Hardware - Supported devices](../09_HARDWARE.md)
- [Testing - golden render](../14_TESTING.md)
