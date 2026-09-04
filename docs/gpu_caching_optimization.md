# GPU Caching Optimization — Research & Recommendations

**Status:** Research complete — not yet implemented  
**Date:** 2026-09-05  
**Branch:** `feat/g3-glm-replacement`  

---

## 1. Current State — What the CPU Does Per Frame

The renderer pushes **all geometry to the GPU from scratch every frame**. There is a
single 64K-vertex orphaning VBO ring buffer (`GL_STREAM_DRAW`) that receives every
polygon, every frame. The CPU performs:

| Work Item | Where | Per-Frame Cost |
|-----------|-------|----------------|
| Terrain vertex transform | `GetPreRotatedPoint()` in `TerrainSearch.cpp:320` | CPU matrix×vector for every visible terrain vertex |
| Room vertex transform | `g3_RotatePoint()` in `render.cpp:500` | CPU matrix×vector for every vertex of every visible room |
| Object vertex transform | `RotateModelPoints()` in `newstyle.cpp:903` | CPU matrix×vector + per-vertex lighting for every submodel |
| g3Point → PosColorUVVertex | `rend_DrawPolygon3D()` in `HardwareBaseGPU.cpp:503` | Format conversion per polygon vertex |
| PosColorUVVertex → PosColorUV2Vertex | `Renderer::addVertexData()` in `HardwareOpenGL.cpp:129` | Stack alloc + std::transform per polygon |
| VBO map/copy/unmap | `OrphaningVertexBuffer::AddVertexData()` in `ShaderProgram.h:118` | 3 GL calls per polygon |
| Draw call | `dglDrawArrays(GL_TRIANGLE_FAN, ...)` | 1 GL call per polygon face |

The terrain mesh is **entirely static** (heightmap never changes at runtime) but
every visible vertex is CPU-transformed every frame. Room geometry is similarly
static. Object geometry is static per-frame (only the per-object transform changes).

---

## 2. Current GPU Resource Management

### What's Already Cached

| Resource | Cache Strategy | Effectiveness |
|----------|---------------|---------------|
| **Textures** | OpenGL texture objects via `OpenGL_bitmap_remap[]` / `OpenGL_lightmap_remap[]`. Uploaded once, re-uploaded only when `BF_CHANGED` / `LF_CHANGED` flags are set. | Good — static textures upload once |
| **Texture bind** | `OpenGL_last_bound[tn]` guard skips redundant `dglBindTexture` | Good — saves ~50% of binds |
| **Wrap/filter state** | Per-texture `OpenGL_bitmap_states[]` / `OpenGL_lightmap_states[]` packed bytes | Good — skips redundant `glTexParameteri` |
| **Shader program** | Single program bound once at init | Perfect — zero shader switches |
| **Blend/zbuffer state** | Early-return guards on all `rend_Set*` functions | Good — only fires on actual state change |
| **Texture enable bitmask** | `setTextureEnabled` early-out (Phase 6) | Good — skips uniform upload when unchanged |

### What's NOT Cached

| Resource | Current Behavior | Waste |
|----------|-----------------|-------|
| **Terrain vertices** | Re-transformed + re-uploaded every frame | Hundreds of CPU matrix multiplies + VBO writes |
| **Room vertices** | Re-transformed + re-uploaded every frame | Same |
| **Object vertices** | Re-transformed + re-uploaded every frame | Same |
| **Lightmap sub-regions** | Full 128×128+ texture re-uploaded when any pixel changes | Dirty rectangles tracked (`cx1/cx2/cy1/cy2`) but never used |
| **Vertex format** | 44–60 bytes per vertex, shader reads 24–32 bytes | ~47% wasted bandwidth |

---

## 3. Vertex Format Analysis — The Biggest Easy Win

### Current Format

```cpp
struct color_array { float r, g, b, a; };  // 16 bytes
struct tex_array   { float s, t, r, w; };  // 16 bytes (r, w always 0.0, 1.0)
struct PosColorUVVertex  { vector pos; color_array color; tex_array uv;   };  // 44 bytes
struct PosColorUV2Vertex { vector pos; color_array color; tex_array uv0; tex_array uv1; };  // 60 bytes
```

The shader (`vertex.glsl`) reads:
```glsl
in vec3 in_pos;       // 3 floats from pos     (12 bytes read, 12 bytes written ✓)
in vec4 in_color;     // 4 floats from color   (16 bytes read, 16 bytes written ✓)
in vec2 in_uv0;       // 2 floats from uv0     ( 8 bytes read, 16 bytes written ✗ — 8 bytes wasted)
in vec2 in_uv1;       // 2 floats from uv1     ( 8 bytes read, 16 bytes written ✗ — 8 bytes wasted)
```

**Waste:** `tex_array.r` and `tex_array.w` are always set to `0.0f` and `1.0f` —
they're padding the shader never reads. That's 8 bytes per UV set, 16 bytes total.
Plus `color_array` uses 4×float (16 bytes) when `GLubyte[4]` normalized (4 bytes)
gives identical precision (the game's color data is 8-bit per channel).

### Proposed Format

```cpp
// Vertex attributes for the GPU
struct PosColorUVVertex_GPU {
  glm::vec3 pos;           // 12 bytes (unchanged)
  GLubyte color[4];        //  4 bytes (was 16) — GL_TRUE normalized → [0,1] in shader
  float s, t;              //  8 bytes (was 16) — only the 2 UV components the shader reads
};                         // 24 bytes total (was 44, -45%)

struct PosColorUV2Vertex_GPU {
  glm::vec3 pos;           // 12 bytes
  GLubyte color[4];        //  4 bytes
  float s0, t0;            //  8 bytes
  float s1, t1;            //  8 bytes
};                         // 32 bytes total (was 60, -47%)
```

### Impact

- **2× less memory bandwidth** per VBO upload
- **2× ring buffer capacity** before orphaning (131K vertices instead of 65K)
- **Better GPU cache locality** — more vertices fit in L1/L2 cache
- **~30-line code change**: struct definitions in `HardwareInternal.h`, `vertexAttrib`
  calls in `HardwareOpenGL.cpp:78-81`, shader `in_color` stays `vec4` (GLSL auto-normalizes
  `GL_UNSIGNED_BYTE`)

### Risk: LOW

The shader already declares `in vec4 in_color` and `in vec2 in_uv0`/`in_uv1`.
`glVertexAttribPointer` with `GL_UNSIGNED_BYTE` + `GL_TRUE` normalized produces the
same `[0,1]` float values the shader expects. No shader changes needed.

---

## 4. Static Geometry VBOs — The Biggest Structural Win

### Terrain

The terrain mesh is a fixed grid (`TERRAIN_GRID*2 + 1` vertices per side). The
heightmap is set at level load and never changes. Currently, every visible vertex
undergoes:

1. CPU-side view-space transform (`GetPreRotatedPoint`) — matrix×vector per vertex
2. CPU-side UV fill from precomputed tables
3. Format conversion to `PosColorUVVertex`
4. VBO upload (map/copy/unmap)
5. `glDrawArrays` call

**Proposed:** Upload terrain vertices to a `GL_STATIC_DRAW` VBO at level load.
The vertex shader already applies `u_modelview` to `in_pos` — if we pass
**world-space** positions instead of pre-rotated view-space positions, the GPU
handles the camera transform. Only the `u_modelview` uniform changes per frame.

The remaining per-frame work is:
- Update a small `GL_DYNAMIC_DRAW` color buffer for per-vertex lighting (gouraud)
- One `glDrawElements` call per terrain batch (index buffer for the grid is also static)

**Eliminates:** All CPU vertex transforms, UV fills, and format conversion for terrain.

**Complexity:** Medium. Requires restructuring `DisplayTerrainList` to separate
static geometry (positions, UVs, indices) from dynamic data (vertex colors for
lighting). The UVs are precomputed in `TerrainUSpeedup`/`TerrainVSpeedup` tables
and are static, so they can be baked into the VBO.

### Rooms

Room geometry is similarly static (loaded from `.room` files, never modified at
runtime — except for door triggers, which are rare). The same approach applies:
upload room vertex positions + UVs to a `GL_STATIC_DRAW` VBO at room load time.
Only vertex colors (lighting) update per-frame.

**Eliminates:** All CPU vertex transforms, UV fills, and format conversion for rooms.

**Complexity:** Medium-High. Room rendering has more edge cases (portal/mirror
clipping, fog plane, custom clip planes). The clipping path still needs CPU-side
`g3Point` data, so the VBO path would need to coexist with the current path for
clipped geometry.

### Objects (Polymodels)

Objects have per-object transforms (position + orientation) that change every frame.
The mesh itself is static per-object-type, but:
- Per-vertex lighting is computed from `Polymodel_light_direction` (changes per frame)
- Submodel hierarchy means different parts of the mesh have different transforms
- Animation (gun points, thruster flames) modifies some vertices

**Proposed:** Upload each polymodel's base mesh to a `GL_STATIC_DRAW` VBO.
Use per-object uniforms for the transform + lighting direction. The vertex shader
would compute lighting instead of the CPU.

**Complexity:** High. Requires a shader-side lighting model that replicates the
current CPU gouraud lighting. The submodel hierarchy complicates instancing.
**Recommendation:** Defer this until terrain + room VBOs are proven.

---

## 5. Lightmap Sub-Region Uploads

### Current State

When dynamic lights modify lightmap texels, `ClearDynamicLightmaps()` (lighting.cpp:1364)
marks the affected lightmaps with `LF_CHANGED`. At the next texture bind,
`opengl_TranslateBitmapToOpenGL()` uploads the **entire** lightmap via `glTexSubImage2D`,
even though the dirty rectangle is tracked:

```cpp
// In GameLightmaps[handle]:
uint8_t cx1, cy1, cx2, cy2;  // dirty rect (set by lighting code)
```

Terrain lightmaps are 128×128 (32KB at RGBA8). Room lightmaps vary but are
typically 64×64 to 256×256.

### Proposed

Use the dirty rectangle to upload only the modified sub-region:

```cpp
glTexSubImage2D(GL_TEXTURE_2D, 0,
    cx1, cy1,                    // x/y offset
    cx2 - cx1, cy2 - cy1,       // width/height
    GL_RGBA, GL_UNSIGNED_BYTE,
    &translated_data[cy1 * texWidth + cx1]);
```

**Impact:** If a dynamic light modifies a 32×32 region of a 128×128 lightmap,
this uploads 4KB instead of 32KB — an 8× reduction in lightmap upload bandwidth.

**Complexity:** Low. The dirty rect tracking already exists. The change is in
`opengl_TranslateBitmapToOpenGL()` to read the rect and issue a sub-region upload.

**Risk:** Low. The translated pixel data is already in a contiguous CPU buffer.
The only gotcha is that the translate table operates on the full texture, so
we'd need to translate only the dirty region or accept the full-translate +
sub-upload pattern (still saves GPU bandwidth).

---

## 6. Draw Call Reduction — Extend Batching

### Current State

| Path | Batching | Draw Calls |
|------|----------|------------|
| Terrain (BUGFIX #560) | ✅ Batched by (texture, lightmap) | 1 per batch |
| Polymodel gouraud (Phase 5) | ✅ Batched same-texture faces | 1 per texture run |
| Polymodel lightmap | ❌ One `GL_TRIANGLE_FAN` per face | 1 per face |
| Room faces | ❌ One `GL_TRIANGLE_FAN` per face | 1 per face |
| UI/lines | ❌ Individual draw calls | 1 per element |

### Proposed: Extend Batching to Lightmap Faces

The Phase 5 batch mode (`gpu_SetBatchMode`/`gpu_FlushBatch`) already handles
the single-texture case. For lightmap faces, the approach is:

1. Sort room faces by (diffuse texture, lightmap) — same as terrain
2. Enable batch mode with a new flag that includes lightmap UVs (`PosColorUV2Vertex`)
3. Accumulate triangulated faces into the batch buffer
4. Flush at texture-change boundaries

**Impact:** Room rendering typically has hundreds of faces. If 80% share a
texture+lightmap pair, draw calls drop from ~200 to ~20.

**Complexity:** Medium. The `PosColorUV2Vertex` path needs the same fan-triangulate
logic that `gpu_RenderPolygon` already has for `PosColorUVVertex`.

---

## 7. What NOT To Do

| Technique | Why Skip |
|-----------|----------|
| **Persistent Mapped Buffers (PMB)** | The current orphan+map pattern is fine for this draw volume. PMB adds complexity (sync objects, fences) for negligible gain when the ring buffer isn't stalling. |
| **Uniform Buffer Objects (UBOs)** | Only 1 shader program, ~6 uniforms. The overhead of `glUniform*` is negligible vs the draw call count. |
| **Compute Shader Pre-transform** | Would require rewriting the entire `g3` pipeline (clipping, culling, lighting) to run on GPU. Massive rewrite for marginal gain over static VBOs. |
| **Texture Arrays / Bindless** | Descent3 textures are variable-size and non-square — texture arrays require uniform size. Bindless requires GL 4.4+ baseline. |
| **16-bit Normalized Positions** | World coordinates span large ranges; 16-bit gives only ~65K discrete values. Would need per-mesh scale/offset, adding complexity for marginal gain. |
| **GL_ARB_indirect_draw** | The batch buffer is already accumulated on CPU. Indirect draw adds a GPU-side command buffer that must be synchronized — no benefit over the current direct `glDrawArrays`. |

---

## 8. Recommended Implementation Order

### Phase 7: Vertex Format Compression
**Impact:** High | **Effort:** Low | **Risk:** Low

Compress `color_array` from 4×float to `GLubyte[4]` normalized. Shrink `tex_array`
from 4×float to 2×float (drop unused `r`/`w` fields). Update `vertexAttrib` calls
and struct definitions.

**Files changed:** `HardwareInternal.h`, `HardwareOpenGL.cpp`, `HardwareBaseGPU.cpp`
**Test:** Verify shader output matches before/after for representative vertex data.

### Phase 8: Static Terrain VBO
**Impact:** High | **Effort:** Medium | **Risk:** Medium

Upload terrain positions + UVs + indices to a `GL_STATIC_DRAW` VBO at level load.
Keep a `GL_DYNAMIC_DRAW` color buffer for per-vertex lighting. Restructure
`DisplayTerrainList` to use `glDrawElements` from the static VBO instead of
CPU-transforming + uploading per frame.

**Files changed:** `terrainrender.cpp`, `TerrainSearch.cpp`, `HardwareOpenGL.cpp`
**Test:** Verify terrain renders identically with static VBO vs current path.

### Phase 9: Lightmap Sub-Region Upload
**Impact:** Medium | **Effort:** Low | **Risk:** Low

Use the existing dirty rectangle (`cx1/cx2/cy1/cy2`) in `opengl_TranslateBitmapToOpenGL`
to issue sub-region `glTexSubImage2D` instead of full-texture uploads.

**Files changed:** `HardwareOpenGL.cpp`
**Test:** Verify dynamic lighting renders identically.

### Phase 10: Room Static VBOs
**Impact:** Medium | **Effort:** Medium-High | **Risk:** Medium-High

Upload room geometry to per-room VBOs at room load time. Coexist with the current
CPU path for clipped/portal geometry.

**Files changed:** `render.cpp`, `HardwareOpenGL.cpp`
**Test:** Verify room rendering with portals, mirrors, and fog planes.

### Phase 11: Extend Batching to Lightmap Faces
**Impact:** Medium | **Effort:** Medium | **Risk:** Medium

Extend the Phase 5 batch mode to handle `PosColorUV2Vertex` (lightmap overlay).
Sort room faces by (diffuse, lightmap) and accumulate.

**Files changed:** `HardwareOpenGL.cpp`, `render.cpp`
**Test:** Verify room rendering with lightmaps.

---

## 9. Expected Impact Summary

| Phase | CPU Work Eliminated | Draw Call Reduction | GPU Bandwidth Savings |
|-------|--------------------|--------------------|-----------------------|
| 7 (Vertex format) | — | — | ~45% per vertex upload |
| 8 (Terrain VBO) | All terrain CPU transforms | Same (already batched) | Eliminated per-frame terrain upload |
| 9 (Lightmap sub-region) | — | — | ~80% for dynamic lightmaps |
| 10 (Room VBOs) | All room CPU transforms | Same | Eliminated per-frame room upload |
| 11 (Lightmap batching) | — | ~80% fewer room draw calls | Fewer state changes |

The combined effect of Phases 7–11 would transform the renderer from a
"push everything every frame" model to a "cache static geometry on GPU,
only update what changes" model — the standard approach in modern engines.

---

## 10. References

- [Khronos — Buffer Object Streaming](https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming)
- [Khronos — Vertex Specification](https://www.khronos.org/opengl/wiki/Vertex_Specification)
- [Song Ho Ahn — OpenGL Projection Matrix](https://www.songho.ca/opengl/gl_projectionmatrix.html)
- [g3_replacement.md](g3_replacement.md) — Phase 1–6 history
- [g3_functions.md](g3_functions.md) — current g3 API reference
