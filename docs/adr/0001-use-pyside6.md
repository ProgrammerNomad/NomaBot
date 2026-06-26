# ADR 0001: Use PySide6 from day one

**Status:** Accepted  
**Date:** 2025-06 (planning)

## Context

NomaBot needs a professional desktop UI: system tray, settings, device manager, character editor, log viewer, and dockable panels. Python offers many UI options (Tkinter, CustomTkinter, Kivy, Flet, DearPyGui, PySide6).

Some projects start with a simpler toolkit and migrate later.

## Decision

Use **Python 3.13 + PySide6 (Qt6) + Qt Designer** from the first desktop milestone. No interim UI framework.

## Consequences

**Positive**

- Native Windows look, dark mode, tray, charts, trees, SVG
- Qt Designer reduces layout maintenance cost
- No full UI rewrite when the app grows

**Negative**

- Larger installer than Tkinter (~50–80 MB acceptable with Nuitka)
- Qt learning curve for contributors unfamiliar with signals/slots

**Neutral**

- CustomTkinter and similar remain valid for other projects—not this one

## References

- [Desktop App](../02_DESKTOP_APP.md)
- [Architecture — PySide6](../01_ARCHITECTURE.md#pyside6-from-day-one)
