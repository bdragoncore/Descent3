# Proposal — Replacing the `g3` 3D Library with a Modern GLM-Based Pipeline

**Status:** In progress — Phases 1–3 (transform matrices, GPU clip/project, instancing) landed on `feat/g3-glm-replacement`; Phase 4 (Mesa removal) investigated and closed as not applicable (§4.4); Phase 5 (face batching) landed (§4.5); Phase 6 (state caching) landed (§4.6)
**Companion doc:** [`g3_functions.md`](g3_functions.md) — the current library's full reference

---

## 1. Background

The `g3` library is Descent 3's classic software 3D pipeline, descended from the
Descent 1/2 engine. It performs the full fixed-function transform chain on the
CPU — view transform, perspective projection, view-frustum clipping, and polygon
submission — and hands the resulting geometry to the hardware renderer.

The codebase is **already half-modernized**:

- `renderer/HardwareOpenGL.cpp` uses **GLM** (`glm::mat4x4`, `glm::ortho`,
  `glm::make_mat4x4`) and a **shader-based pipeline** (`ShaderProgram.h`,
  dynamic VBO via `addVertexData`, `dglDrawArrays` with `GL_TRIANGLES` /
  `GL_TRIANGLE_FAN`).
- `renderer/shaders/vertex.glsl` already does `gl_Position = u_projection *
  (u_modelview * in_pos)` — the GPU already applies the model-view and
  projection matrices and clips in clip space.
- `renderer/MesaOpenGL.cpp` has `glBegin(GL_TRIANGLE_FAN)` fixed-function code,
  but it is the **test harness backend** (only linked into `d3_render_tests_game`),
  not a production path — see §4.4.

What `g3` still does on the **CPU** is the part we want to eliminate or shrink:

| Subsystem | File | CPU work today |
|-----------|------|----------------|
| Frame setup / FOV | `HardwareSetup.cpp` | builds viewport/projection/model-view matrices by hand |
| Point rotation + coding | `HardwarePoints.cpp` | `g3_RotatePoint`, `g3_CodePoint`, delta vectors |
| Frustum clipping | `HardwareClipper.cpp` | Sutherland–Hodgman polygon/line clipping, temp-point pool |
| Projection | `HardwarePoints.cpp` | pinhole `sx = w2 + x·w2/z` |
| Instancing | `HardwareInstance.cpp` | re-bases the view transform on a stack |
| Transform matrices | `HardwareTransforms.cpp` | hand-rolled 4×4 mult/transpose, GL refresh |
| Vertex assembly | `HardwareBaseGPU.cpp` | packs `g3Point` → `PosColorUV2Vertex` |

---

## 2. Recommendation

**Do not swap in a new library.** GLM is already vendored and already the
transform math in use. The right move is to **reimplement `g3`'s internals on
top of the existing GLM + shader pipeline**, keeping the `g3_*` API surface
intact so the ~hundreds of call sites in game code don't change.

This is a **refactor, not a rewrite**: the public API (`g3Point`, `g3Codes`,
`g3_StartFrame`, `g3_DrawPoly`, `g3_StartInstanceMatrix`, …) stays; only the
internals change.

### Why not the alternatives

| Option | Verdict |
|--------|---------|
| **bgfx** (17.5k★, D3D11/12/Metal/GL/Vulkan) | Overkill. Full render-abstraction layer; huge dependency; would replace the working shader pipeline we already have. |
| **sokol_gfx** | Lighter, but still a full renderer swap for no benefit over the existing GL path. |
| **DirectXMath** | Windows-only SIMD math; we need cross-platform (Linux/macOS/Windows CI). GLM already covers this. |
| **Modern GL core profile alone** | Already partially done (the shader path). The remaining work is *removing* the CPU transform/clip, not adding GL features. |

GLM is the right choice because it is **already a dependency**, is header-only
(no build/link changes), and maps 1:1 onto the matrix math `g3` already
computes by hand.

---

## 3. Target Architecture

After the refactor, the pipeline is:

```
game code (g3_* API, unchanged)
        │
        ▼
g3 front-end (thin): g3Point, g3Codes, culling decisions
        │
        ▼
vertex assembly: g3Point → PosColorUV2Vertex (world-space pos + UVs + color)
        │
        ▼
GPU (vertex.glsl): u_modelview * in_pos → u_projection → clip → rasterize
```

The CPU no longer projects or clips. It only:
1. decides **what** to submit (backface cull, portal/occlusion culling via
   `g3Codes`),
2. assembles world-space vertices,
3. lets the GPU do projection + clipping.

---

## 4. Work Items (in dependency order)

### 4.1 Replace hand-rolled matrix math with GLM

**Files:** `HardwareSetup.cpp`, `HardwareTransforms.cpp`, `HardwareInstance.cpp`

Replace the manual `g3_GetModelViewMatrix`, `g3_GetProjectionMatrix`,
`g3_GetViewPortMatrix`, `g3_TransformMult`, `g3_TransformTrans`,
`g3_UpdateFullTransform` with GLM equivalents:

- `g3_GetProjectionMatrix` → `glm::perspective` (or keep the exact current
  matrix, expressed as a `glm::mat4x4`). **Must preserve the current near=0 /
  far=∞ convention** so depth behavior is unchanged.
- `g3_GetModelViewMatrix` → `glm::lookAt`-style construction, keeping the
  `Z_bias` translation.
- `g3_GetViewPortMatrix` → `glm::ortho` (already used at
  `HardwareOpenGL.cpp:1888`).
- `g3_TransformMult` / `g3_TransformTrans` → `glm::operator*` / `glm::transpose`.

The `gTransform*` arrays become `glm::mat4x4` (or stay as `float[16]` fed via
`glm::make_mat4x4`, which is already used at `HardwareOpenGL.cpp:1900`).

**Why:** removes ~100 lines of hand-rolled, error-prone matrix code; the math
is identical, so behavior is preserved.

### 4.2 Move projection + clipping to the GPU

**Files:** `HardwareDraw.cpp`, `HardwarePoints.cpp`, `HardwareClipper.cpp`, `HardwareBaseGPU.cpp`

The vertex shader already projects (`u_projection * vertex_modelview_pos`) and
the GPU already clips in clip space. The CPU-side `g3_ProjectPoint` and
`g3_ClipPolygon` / `ClipLine` are **unused for the 3D polygon draw path**.

- `g3_DrawPoly` / `g3_DrawPolyList` dispatch straight to
  `rend_DrawPolygon3D` / `rend_DrawPolygonList3D` with **world-space**
  `p3_vecPreRot` vertices (see `HardwareBaseGPU.cpp:479`, `:533`). The GPU
  does the rest. The old software clip/project block inside `g3_DrawPoly`
  (commented out since the GPU path landed) has been **deleted**; the
  `g3_drawpath_real_tests` suite pins this behavior.

**Caveat — what still needs CPU clip/project (NOT dead code):**
- **Portal / mirror culling** (`Descent3/render.cpp`) calls `g3_ClipPolygon`
  + `g3_ProjectPoint` to clip portal polygons to the view frustum and compute
  screen-space clip windows for `BuildRoomListSub` / `BuildMirroredRoomListSub`.
  This is real polygon clipping, not just `g3_CodePoint` culling — keep it.
- **2D line drawing** (`g3_DrawLine` / `g3_DrawSpecialLine` in
  `HardwareDraw.cpp`) uses `ClipLine` + `g3_ProjectPoint` to draw screen-space
  lines. Keep it.
- **2D/overlay/picking** paths (`g3_DrawSphere`, `g3_DrawBox`, `g3_Point2Vec`,
  `PF_PROJECTED` points) still need `g3_ProjectPoint` and the inverse-projection
  math. Keep those.
- **Editor radiosity** (`editor/rad_hemicube.cpp`) calls `g3_ClipPolygon`.
- **DLL API ABI** (`Descent3/Game2DLL.cpp` fp[305]–fp[331]) exports
  `g3_ProjectPoint`, `g3_ClipPolygon`, `g3_FreeTempPoints`, `ClipLine`-adjacent
  helpers, `g3_RotatePoint`, `g3_CodePoint`, `g3_CalcPointDepth`, `g3_Point2Vec`,
  `g3_RotateDelta*`, `g3_AddDeltaVec`, `g3_SetCustomClipPlane`, `g3_SetFarClipZ`,
  `g3_StartInstance*`, `g3_DoneInstance`, `g3_SetTriangulationTest`. These are
  consumed by DMFC netgame DLLs and must keep their symbols.
- **Lightmap UVs** are interpolated per-vertex; if a polygon straddles the
  near plane, the GPU clips the *position* but the lightmap UVs are
  interpolated in screen space, which is correct. So no CPU clip needed for
  the draw path.

### 4.3 Replace the instancing stack with GLM matrices

**Files:** `HardwareInstance.cpp`, `HardwarePoints.cpp`, `HardwareDraw.cpp`,
`HardwareSetup.cpp`, `HardwareBaseGPU.cpp`

**Done.** The old `g3_StartInstanceMatrix` re-based the global
`View_position`/`View_matrix`/`Unscaled_matrix` on a stack so the CPU g3
functions would work on object-space vertices. That trick is removed:

- `g3_StartInstanceMatrix(pos, orient)` pushes `glm::mat4x4 model =
  translate(pos) * mat4(orient)` (composed with the parent when nested) onto a
  GLM stack; `g3_DoneInstance()` pops. The globals now **always** hold the true
  view state.
- `g3_UpdateModelViewMatrix()` recomputes `gTransformModelView = view * model`
  (identity model when no instance is active). It is shared by
  `g3_StartInstanceMatrix`, `g3_DoneInstance` and `rend_SetZBias` — the last is
  called *during* instancing (`newstyle.cpp:755` for `SOF_CUSTOM` submodels) and
  would otherwise have dropped the model transform.
- `g3_GetInstanceTransform(orient, pos)` exposes the composed transform in the
  g3 row-vector convention (`world = src * ~orient + pos`; identity/zero when no
  instance is active). The consumers that previously relied on the re-based
  globals now apply it:
  - `g3_RotatePoint` — transforms the object-space vertex to world space first.
  - `g3_CheckNormalFacing` — transforms the point to world space, then rotates
    the viewer-minus-point vector back into the object's local frame so the dot
    product with the object-space normal is still valid.
  - `g3_GetViewPosition` / `g3_GetUnscaledMatrix` — return the view position /
    unscaled matrix in the object's local frame.
- `g3_GetViewMatrix`, `g3_CalcPointDepth`, `g3_Point2Vec`, `g3_RotateDelta*`,
  `g3_AddDeltaVec`, `g3_ProjectPoint`, `g3_CodePoint`, `g3_SetCustomClipPlane`
  are unchanged: none of their callers run during instancing, and they now
  operate on the always-true view state.

**Why:** removes the view-rebasing trick and the `InstanceContext` stack
bookkeeping; the GPU handles the object transform via the existing
`u_modelview = view_ * model_` uniform (`HardwareOpenGL.cpp:105`).

**Tests:** `g3_instance_real_tests` (12 tests) compiles the real
`HardwareInstance.cpp`/`HardwareSetup.cpp`/`HardwareTransforms.cpp`/
`HardwareGlobalVars.cpp`/`HardwarePoints.cpp`/`HardwareClipper.cpp`/
`HardwareDraw.cpp` with stubbed `rend_*` entry points. Equivalence tests pin
that the new `view * model` model-view, `g3_RotatePoint`, `g3_CheckNormalFacing`,
`g3_GetViewPosition` and `g3_GetUnscaledMatrix` results match the old re-based
formulas (they pass against both old and new code); the refactor-pinning test
`ViewStateUnchangedDuringInstance` fails on the old code and passes on the new.

### 4.4 Delete the legacy fixed-function path — **investigated, not applicable**

**File:** `MesaOpenGL.cpp`

**Finding (2026-09-04):** the premise of this phase was wrong. `MesaOpenGL.cpp`
is **not** a production fallback renderer — it is the **test harness backend**
for the render test framework (added in `68ef64a0`). It is built as a separate
`mesaopengl` static library and only linked into `d3_render_tests_game` via
`--allow-multiple-definition` (first definition wins, overriding the `descent3`
renderer symbols). The `glBegin(GL_TRIANGLE_FAN)` / `glEnd` calls in
`gpu_RenderPolygon` / `gpu_RenderPolygonUV2` / `rend_DrawLine` are the polygon
rendering implementation for that test renderer, and the render tests
(`CubeRenders`, `TexturedPolygonD3`, `MipmapFiltering`, `TextureWrap*`,
`Multitexture*`, …) draw through them.

The runtime-selection check the phase asked for was performed:

- **Production never selects `RENDERER_MESA`.** The game and editor always use
  `RENDERER_OPENGL` (`Descent3/init.cpp:1241,1614`, `Descent3/game.cpp:712`,
  `editor/editorView.cpp:1143`, `editor/gameeditor.cpp:605`). `RENDERER_MESA`
  is only used by the test harness (`tests/render/render_test_base.cpp:85`).
- **The production renderer already has zero fixed-function code.** A scan of
  `renderer/` for every fixed-function GL call (`glBegin`/`glEnd`,
  `glMatrixMode`, `glLoadMatrix*`, `glOrtho`, `glLight*`, `glMaterial*`,
  `glFog*`, `glTexEnv*`, `glShadeModel`, …) matches only `MesaOpenGL.cpp` (plus
  one commented-out line in `HardwareOpenGL.cpp`). `HardwareOpenGL.cpp` is
  already fully shader-based (`ShaderProgram.h`, dynamic VBO, `dglDrawArrays`).

**Conclusion:** there is no production fixed-function path to delete. Removing
the `glBegin`/`glEnd` code from `MesaOpenGL.cpp` would break the passing render
test suite (`d3_render_tests_game` passes in ~4s) with zero production benefit.
The fixed-function calls are intentionally kept in the test harness, which uses
a Mesa compatibility profile. No code change was made; this phase is closed as
not applicable.

### 4.5 (Optional) Batch instanced draws

**Research finding:** True GPU instancing (`glDrawArraysInstanced` /
`gl_InstanceID`) is architecturally infeasible for polymodels. Per-object
submodel animation, per-vertex CPU lighting, per-face state changes, and
per-object effects all prevent sharing vertex buffers across instances.

**Implemented instead — polymodel face batching.** The practical optimization
extends the #560 terrain batching pattern to polymodel faces. In
`RenderSubmodelFacesUnsorted` (model/newstyle.cpp), when `StateLimited` is true
**and** `Polymodel_light_type != POLYMODEL_LIGHTING_LIGHTMAP` **and**
`gpu_Overlay_type == OT_NONE`, consecutive same-texture faces are accumulated
into a single `GL_TRIANGLES` draw call instead of one `GL_TRIANGLE_FAN` per
face:

- `gpu_SetBatchMode(bool)` / `gpu_FlushBatch()` added to
  `renderer/HardwareOpenGL.cpp` with a static `PosColorUVVertex` buffer.
- `gpu_RenderPolygon` fan-triangulates and accumulates when batch mode is
  active; `gpu_FlushBatch` submits all accumulated triangles in one draw call.
- `RenderSubmodelFacesUnsorted` enables batch mode before the sorted face loop,
  flushes at texture-change boundaries (via `sort_key`), and flushes + disables
  after the loop.

Batching is gated off for lightmap lighting (per-face overlay UVs) and when an
overlay is active (multitexture path uses `PosColorUV2Vertex`). Shared vertices
across faces are not a problem because vertex packing reads from `Robot_points`
at call time. Delivered as `polymodel_batch_real_tests` (7 tests) verifying the
orchestration: batch enabled for gouraud, disabled for lightmap/overlay, and
flushed at texture boundaries.

### 4.6 State caching optimizations

Three behavior-preserving optimizations that eliminate redundant driver calls
and per-vertex computations in the rendering hot path:

1. **Instance transform decomposition caching** (`HardwareInstance.cpp`):
   `g3_GetInstanceTransform` previously decomposed the 4×4 model matrix into
   orient/pos on every call. For a mesh with N vertices, this performed N×12
   matrix reads + N×12 vector writes. Now the decomposed form is cached at
   push time (`g3_StartInstanceMatrix`), reducing the per-vertex cost to an
   array lookup. For a 100-vertex mesh, this eliminates ~1188 redundant reads
   and ~1188 redundant writes per instance level.

2. **`setTextureEnabled` early-out** (`HardwareOpenGL.cpp`): The `Renderer`
   struct now uses `ComputeTextureEnable()` to check whether the texture-enable
   bitmask has actually changed before calling `glUniform1i`. Every polygon draw
   previously called `setTextureEnabled(1, false)` unconditionally, issuing a
   uniform upload even when multitexture was already disabled (the common case).
   With the early-out, this driver call is skipped when the bitmask is unchanged.

3. **Remove redundant `dglActiveTexture` from non-texture state setters**
   (`HardwareOpenGL.cpp`): `rend_SetLighting`, `rend_SetTextureType`, and
   `rend_SetAlphaType` previously called `dglActiveTexture(GL_TEXTURE0_ARB + 0)`
   on every invocation. These functions only touch uniform or blend state — not
   texture binding — so the `glActiveTexture` call was pure overhead. Removed.

Delivered as `renderer_state_cache_real_tests` (13 tests):
- `ComputeTextureEnableTest` (8 tests): verifies the bitmask helper for all
  combinations of set/clear/toggle on both bit indices.
- `RendererStateCacheTest` (5 tests): verifies the instance transform cache
  returns correct decomposed orient/pos for single, nested, repeated, and
  pop-push sequences, and that identity is restored after popping all instances.

---

## 5. What Stays (API surface preserved)

The following must remain callable and behavior-compatible, because game code
depends on them:

- `g3Point`, `g3Codes`, `g3UVL` structs and the `PF_*` / `CC_*` constants.
- `g3_StartFrame` / `g3_EndFrame`, `g3_GetViewPosition` / `g3_GetViewMatrix` /
  `g3_GetUnscaledMatrix` / `g3_GetMatrixScale` / `g3_SetAspectRatio`.
- `g3_RotatePoint`, `g3_CodePoint`, `g3_ProjectPoint` (used by picking/2D),
  `g3_CalcPointDepth`, `g3_Point2Vec`, the `g3_RotateDelta*` family.
- `g3_DrawPoly`, `g3_DrawPolyList`, `g3_CheckAndDrawPoly`,
  `g3_CheckNormalFacing`, `g3_DrawLine`, `g3_DrawSpecialLine`, `g3_DrawSphere`,
  `g3_DrawBitmap`, `g3_DrawRotatedBitmap`, `g3_DrawPlanarRotatedBitmap`,
  `g3_DrawBox`.
- `g3_StartInstanceMatrix`, `g3_StartInstanceAngles`, `g3_DoneInstance`.
- `g3_SetFarClipZ`, `g3_ResetFarClipZ`, `g3_SetCustomClipPlane`.

The **only** things that change are the internals of the above and the removal
of the now-dead software clip/project code.

---

## 6. Risks and Mitigations

| Risk | Mitigation |
|------|------------|
| **Depth behavior changes** if the projection matrix is rewritten | Keep the exact current near=0/far=∞ matrix, just expressed in GLM. Add a test asserting the matrix values match the current hand-built ones. |
| **Lightmap UVs wrong** if a polygon crosses the near plane | The GPU interpolates UVs in screen space after clipping, which is correct. Add a test with a polygon straddling the near plane. |
| **Culling regressions** if `g3Codes` semantics change | Keep `g3_CodePoint` byte-for-byte; only the clip/project internals change. |
| **2D/overlay paths** rely on `PF_PROJECTED` screen coords | Keep `g3_ProjectPoint` for these; only the 3D draw path stops using it. |
| **Mesa fixed-function path** | Investigated and closed as not applicable: `MesaOpenGL.cpp` is the test harness backend (only linked into `d3_render_tests_game`), production never selects `RENDERER_MESA`, and `HardwareOpenGL.cpp` already has zero fixed-function code. See §4.4. |
| **Instancing re-basing** is relied on by game code reading `View_position`/`View_matrix` mid-instance | Removed in Phase 3: the globals stay true and the consumers (`g3_RotatePoint`, `g3_CheckNormalFacing`, `g3_GetViewPosition`, `g3_GetUnscaledMatrix`) apply the composed model transform via `g3_GetInstanceTransform`. `rend_SetZBias` recomputes through `g3_UpdateModelViewMatrix` so the model is preserved. Equivalence tests prove the results match the old re-based formulas. |

---

## 7. Testing Strategy

Every change must have a test that fails before and passes after (per
`AGENTS.md`). Proposed tests in `tests/cpp_src/`:

1. **`ProjectionMatrixMatchesLegacy`** — `g3_GetProjectionMatrix` (GLM) equals
   the hand-built matrix for several zoom/aspect values.
2. **`ModelViewMatrixMatchesLegacy`** — `g3_GetModelViewMatrix` (GLM) equals
   the hand-built matrix for several view pos/orient/Z_bias values.
3. **`NearPlaneStraddlingPolygon`** — a polygon crossing the near plane still
   renders (GPU clip) with correct lightmap UVs.
4. **`InstanceStackMatchesLegacy`** — `g3_StartInstanceMatrix`/`g3_DoneInstance`
   produce the same model-view matrix as the old re-based state, and the
   consumers (`g3_RotatePoint`, `g3_CheckNormalFacing`, `g3_GetViewPosition`,
   `g3_GetUnscaledMatrix`) agree with the old formulas. Delivered as
   `g3_instance_real_tests` (12 tests), including a refactor-pinning test that
   fails on the old re-based code.
5. **`CodePointUnchanged`** — `g3_CodePoint` returns identical `p3_codes` for a
   set of representative points (guards culling).
6. **`DrawCallCount`** — reuse the #560 batching test to confirm draw-call
   counts don't regress.

Run: `cmake --build build && ctest --test-dir build --output-on-failure`.

---

## 8. Suggested Phasing

| Phase | Scope | Risk |
|-------|-------|------|
| **1** | GLM-ify the transform matrices (§4.1) | Low — pure math swap, tests prove equivalence |
| **2** | Remove dead CPU clip/project from the 3D draw path (§4.2) | Low — the clip/project code stays for culling/lines/2D/DLL ABI; only the dead block in `g3_DrawPoly` is removed, guarded by `g3_drawpath_real_tests` |
| **3** | Instancing as model matrices (§4.3) | Medium — done; `g3_instance_real_tests` (12 tests) proves equivalence with the old re-based state and pins the new invariant |
| **4** | Remove Mesa fixed-function path (§4.4) | N/A — investigated and closed: `MesaOpenGL.cpp` is the test harness backend, production never selects `RENDERER_MESA`, and `HardwareOpenGL.cpp` already has zero fixed-function code |
| **5** | Polymodel face batching (§4.5) | Low — pure perf win; `polymodel_batch_real_tests` (7 tests) verify batch orchestration |
| **6** | State caching optimizations (§4.6) | Low — pure state-caching; `renderer_state_cache_real_tests` (13 tests) verify bitmask helper and instance transform cache |

Each phase lands on `main` independently with its own tests, so a regression is
isolated and reversible.

---

## 9. References

- [`g3_functions.md`](g3_functions.md) — the current library's full reference.
- [Song Ho Ahn — OpenGL Projection Matrix](https://www.songho.ca/opengl/gl_projectionmatrix.html) — the projection math `g3` already uses.
- [glm](https://github.com/g-truc/glm) — the header-only math library already vendored.
- [bgfx](https://github.com/bkaradzic/bgfx) — considered and rejected (overkill).