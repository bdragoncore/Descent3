# revamp.md — Architectural Improvements for Descent3

This document tracks major architectural issues that require significant refactoring.
These are separate from quick bug fixes and should be tackled as dedicated projects.

---

## #560 — Terrain Rendering Overhaul

**Problem:** Outdoor areas cause 1,000–5,000 draw calls with avg 2 triangles each,
dropping framerates to half/third of indoor and tripling GPU usage.

**Root Cause:** The renderer draws polygons via `g3_DrawPoly` (~65 callsites) rather
than batching geometry into VBOs. Each polygon is a separate draw call.

**Suggested Approach:**
1. **VBO/Meshing** — Batch terrain polygons into Vertex Buffer Objects. Group
   neighboring terrain faces into single draw calls. A single 1000-face terrain
   chunk should be 1 draw call, not 1000.
2. **Polygon Stripification** — Convert triangle lists to triangle strips to reduce
   vertex count and improve GPU throughput.
3. **Frustum Culling** — Cull terrain chunks outside the view frustum before
   issuing draw calls.
4. **LOD System** — The Lindstrom 1996 height-field LOD paper was referenced but
   may be broken after terrain limits were expanded. Either fix or replace with
   a simpler distance-based LOD.
5. **Object Batching** — Apply the same VBO approach to other small geometry
   (doors, faces) that currently issue per-polygon calls.

**Impact:** Would fix outdoor performance on all levels, especially Bedlam
(Plutonium, Apparition) and Dementia's Geodomes.

**Estimated Effort:** Large (2–4 weeks for a focused contributor).

**References:**
- `renderer/HardwareOpenGL.cpp` — main rendering backend
- `g3_DrawPoly` — ~65 callsites across the codebase
- Terrain LOD code — based on Lindstrom 1996

---

## #279 — Savegame/Demo Serialization Rewrite

**Problem:** x86 savegames and demos can't load on x64. Structs are directly
serialized without packing or versioning. The `player` struct is 536 bytes on
x86 vs ~1280 on x64.

**Root Cause:** The save/load system does raw `memcpy` of struct contents,
including pointers and vtable-containing classes. Struct layouts differ between
x86 and x64 due to pointer size, alignment, and padding differences.

**Suggested Approach:**
1. **New Serialization Format** — Define an endian-agnostic, versioned binary
   format. Each struct gets explicit read/write methods.
2. **Field-by-Field Serialization** — Replace raw `memcpy` with explicit field
   reads/writes. Use fixed-size types (uint32_t instead of int, etc.).
3. **Version Header** — Add a format version number to the file header so old
   formats can be migrated or rejected gracefully.
4. **Pointer Exclusion** — Never serialize pointers. Serialize object IDs or
   indices instead, and reconstruct pointers on load.
5. **Testing** — Generate x86 savegames, load them on x64, verify correctness.
   Use the struct dumper tool from GravisZro to validate sizes.

**Impact:** Would enable cross-platform savegame/demo sharing, which is
important for multiplayer and speedrunning communities.

**Estimated Effort:** Large (3–6 weeks). ~50+ structs need individual
attention. Could be done incrementally (one subsystem at a time).

**Key Structs (from GravisZro's analysis):**
- `player` — 536 bytes (x86) vs ~1280 bytes (x64)
- `ai_frame` — 3364 bytes (x86) vs 3488 bytes (x64)
- `object.rtype` — size differs on x64
- Plus ~50 more structs with size mismatches

**Tools:**
- [Struct dumper](https://github.com/GravisZro/Descent3/blob/f1503d2e7fe370f5d504741aa6a33d2f2970fff1/Descent3/structdumper.cpp)
- [Offset JSON data](https://gist.githubusercontent.com/GravisZro/d4d62b74079d210a37aaaf921e6a05bb)

**References:**
- `loadstate.cpp` — demo/savegame loading (~line 950, `LGSObjects()`)
- `player` struct — contains pointers, non-packed
- `IOOps.h` (HogMaker) — suggested by winterheart for serialization

---

## #487 — Audio Subsystem Unification

**Problem:** Descent3 uses two independent sound devices: one for MVE video
cutscenes (`libmve/mve_audio`) and one for game audio (`sndlib/sdlsound`).
MVE playback ignores all user volume/sound settings.

**Root Cause:** The MVE player has its own audio output that bypasses the
game's sound system entirely. Volume, buffer, and device settings are not
shared.

**Suggested Approach:**
1. **Unified Audio Backend** — Route MVE audio through the game's sound system
   (`sndlib/sdlsound`) instead of its own device.
2. **Shared Configuration** — MVE audio should respect the same volume,
   sample rate, and buffer settings as game audio.
3. **Fallback Path** — If the game sound system isn't initialized (e.g., during
   startup before full init), MVE can fall back to its own device.
4. **Volume Control** — Expose MVE volume through the same UI controls as
   game sound (master volume, SFX volume, etc.).

**Impact:** Fixes MVE audio ignoring volume settings (the "2 AM thunder"
problem). Also simplifies the audio codebase.

**Estimated Effort:** Medium (1–2 weeks). The MVE audio path is relatively
contained in `libmve/mve_audio`.

**References:**
- `libmve/mve_audio` — MVE video cutscene audio
- `sndlib/sdlsound` — game audio backend

---

## General Modernization Notes

These are not tied to specific issues but would improve maintainability:

1. **Renderer Backend** — The OpenGL 1.x fixed-function pipeline should be
   migrated to modern OpenGL 2+ with shaders. This would enable VBO batching,
   instancing, and better GPU utilization.

2. **Build System** — CMake modernization (target-based properties, proper
   dependency tracking) would improve build times and developer experience.

3. **Input Abstraction** — The controller/keyboard/mouse input system could
   benefit from an abstraction layer that normalizes axis ranges, button
   mappings, and platform differences.

4. **Memory Management** — Replace raw pointer usage with smart pointers where
   feasible, especially in serialization code where pointer lifetimes are
   tricky.
