# Descent 3 — The `g3` 3D Library

The `g3` library is Descent 3's classic software 3D pipeline. It is the direct
descendant of the `g3` library used in Descent 1 and Descent 2 (Parallax
Software's in-house 3D engine). It performs the full fixed-function transform
pipeline — view transform, perspective projection, view-frustum clipping, and
polygon submission — and hands the resulting geometry to the hardware renderer.

In the modern codebase the library lives in the `renderer/` directory:

| File | Contents |
|------|----------|
| `lib/3d.h` | Public API, `g3Point`/`g3Codes`/`g3UVL` structs, clip-code & point-flag constants |
| `renderer/HardwareSetup.cpp` | Frame setup, viewport/projection matrices, aspect ratio |
| `renderer/HardwarePoints.cpp` | Point rotation, projection, coding, delta vectors |
| `renderer/HardwareClipper.cpp` | View-frustum polygon/line clipping, temp-point pool, clip planes |
| `renderer/HardwareDraw.cpp` | Polygon/line/bitmap/sphere/box drawing |
| `renderer/HardwareInstance.cpp` | Object instancing (model transform stack) |
| `renderer/HardwareTransforms.cpp` | 4×4 model-view/projection/viewport matrices, GL transform refresh |
| `renderer/HardwareBaseGPU.cpp` | Vertex-array assembly and dispatch to the GPU |

---

## 1. Coordinate Systems and Conventions

The library uses a **left-handed** world/view convention with **+Z into the
screen** (this is the opposite of OpenGL's right-handed eye space, where the
camera looks down −Z).

### World space
Arbitrary 3D coordinates. The camera is at `View_position` with orientation
`View_matrix`.

### View (camera) space
The origin is the camera. A world point `P` is transformed by first
subtracting the camera position, then rotating by the view matrix:

```
p_view = (P − View_position) · View_matrix
```

`View_matrix` is a rotation matrix whose rows are the camera's right (`rvec`),
up (`uvec`), and forward (`fvec`) vectors. Because the matrix is orthonormal,
multiplying a vector by it rotates the vector into camera space. The forward
vector points **into** the screen, so visible geometry has `p_view.z > 0`.

The view matrix is **scaled** by `Matrix_scale` to implement the field of view
(see §3). The unscaled matrix is kept separately as `Unscaled_matrix`.

### Screen space
After perspective projection (§4), a point has integer screen coordinates
`(p3_sx, p3_sy)` with the origin at the **top-left** of the viewport and +Y
pointing **down** (raster convention).

### The view frustum
The default frustum (before zoom scaling) is the pyramid

```
−z ≤ x ≤ z        (left/right planes)
−z ≤ y ≤ z        (bottom/top planes)
 0 ≤ z ≤ Far_clip_z
```

i.e. a symmetric 90° field of view. Zooming scales `x` and `y` in view space,
which narrows or widens the effective FOV. Points outside the frustum are
flagged with clip codes (§5) and clipped (§6).

---

## 2. Core Data Structures

### `g3Point` — a transformed vertex

```cpp
struct g3Point {
  float p3_sx, p3_sy;   // screen x & y (after projection)
  uint8_t p3_codes;     // clipping codes (which frustum planes it is outside)
  uint8_t p3_flags;     // state flags (projected? has UVs? temp point? ...)
  int16_t p3_pad;       // alignment
  vector p3_vec;        // rotated (view-space) coordinates
  vector p3_vecPreRot;  // original world coordinates
  g3UVL p3_uvl;         // texture coordinates + lighting
};
```

The `p3_vec` field holds the **view-space** (rotated) position; `p3_vecPreRot`
holds the original world position. The hardware renderer uses
`p3_vecPreRot` and lets OpenGL re-apply the model-view transform, so both are
kept.

### Point flags (`p3_flags`)

| Flag | Value | Meaning |
|------|-------|---------|
| `PF_PROJECTED` | 1 | `p3_sx`/`p3_sy` are valid (already projected) |
| `PF_FAR_ALPHA` | 2 | point is past the fog zone |
| `PF_TEMP_POINT` | 4 | point was allocated by the clipper (must be freed) |
| `PF_UV` | 8 | `p3_u`/`p3_v` texture coordinates are set |
| `PF_L` | 16 | `p3_l` intensity lighting is set |
| `PF_RGBA` | 32 | `p3_r/g/b/a` RGBA lighting is set |
| `PF_UV2` | 64 | `p3_u2`/`p3_v2` lightmap coordinates are set |
| `PF_ORIGPOINT` | 128 | `p3_vecPreRot` is valid |

### Clip codes (`p3_codes`)

| Code | Value | Meaning |
|------|-------|---------|
| `CC_OFF_LEFT` | 1 | x < −z (left of frustum) |
| `CC_OFF_RIGHT` | 2 | x > z (right of frustum) |
| `CC_OFF_BOT` | 4 | y < −z (below frustum) |
| `CC_OFF_TOP` | 8 | y > z (above frustum) |
| `CC_OFF_FAR` | 16 | z > `Far_clip_z` (beyond far plane) |
| `CC_OFF_CUSTOM` | 32 | outside the custom clip plane |
| `CC_BEHIND` | 128 | z < 0 (behind the camera) |

### `g3Codes` — aggregate clip state for a polygon

```cpp
struct g3Codes {
  uint8_t cc_or;   // OR of all vertices' codes (any vertex off a plane)
  uint8_t cc_and;  // AND of all vertices' codes (all vertices off a plane)
};
```

- `cc_and != 0` → the whole polygon is outside one plane → reject.
- `cc_or == 0` → every vertex is inside → no clipping needed.
- otherwise → clip.

### `g3UVL` — texture + lighting attributes

```cpp
struct g3UVL {
  float u, v;    // base texture coordinates
  float u2, v2;  // lightmap coordinates (second texture unit)
  union { float l; float r; };  // intensity (mono) or red (RGBA)
  float g, b, a; // green, blue, alpha
};
```

---

## 3. Frame Setup and the View Transform

### `g3_StartFrame(view_pos, view_matrix, zoom)`

Begins a 3D frame. It:

1. Builds the viewport matrix from the renderer's clip rectangle.
2. Builds the projection matrix from `zoom` (§4).
3. Builds the model-view matrix from `view_pos`/`view_matrix`.
4. Stores `View_position`, `View_zoom`, `Unscaled_matrix`.
5. Computes `Matrix_scale` to fold the aspect ratio and FOV into the view
   matrix, then scales the view matrix:

```cpp
scalar s = Window_height / Window_width;          // aspect (h/w)
Matrix_scale = { s <= 1 ? s : 1/s, 1, 1 };        // aspect correction
zoom *= 3/4;                                       // zoom → vertical FOV
Matrix_scale.x *= 1/zoom;                          // FOV applied to x
Matrix_scale.y *= 1/zoom;                          // FOV applied to y

View_matrix.rvec = Unscaled_matrix.rvec * Matrix_scale.x;
View_matrix.uvec = Unscaled_matrix.uvec * Matrix_scale.y;
View_matrix.fvec = Unscaled_matrix.fvec * Matrix_scale.z;  // z scale = 1
```

The FOV is implemented by scaling the camera-space x/y axes rather than by
changing the projection. A larger `zoom` (smaller `1/zoom`) shrinks the
scaled x/y, so a fixed screen extent covers a smaller world angle — i.e. a
narrower FOV (zoom in). This is the classic Descent approach: the projection
is always the fixed 90° pyramid, and zoom is a pre-projection scale.

6. Resets the temp-point pool (`InitFreePoints`) and the far clip plane.

### `g3_EndFrame()`

Closes the frame. In debug builds it verifies the clipper freed all temp
points (`CheckTempPoints`).

### Accessors

| Function | Returns |
|----------|---------|
| `g3_GetViewPosition(vp)` | `View_position` (camera world position) |
| `g3_GetViewMatrix(mat)` | `View_matrix` (scaled view matrix) |
| `g3_GetUnscaledMatrix(mat)` | `Unscaled_matrix` (unscaled view matrix) |
| `g3_GetMatrixScale(v)` | `Matrix_scale` |
| `g3_SetAspectRatio(a)` / `g3_GetAspectRatio()` | user override of the renderer aspect ratio (stored as w/h) |

### `g3_GetModelViewMatrix(viewPos, viewMatrix, mvMat)`

Builds the 4×4 column-major model-view matrix used by the GL transform stack.
The rotation is the view matrix; the translation is `−viewPos` rotated into
camera space (dot products with each basis vector), plus `Z_bias` on the z
translation. In row-major notation:

```
mvMat = [ rvec.x  rvec.y  rvec.z  −P·rvec ]
        [ uvec.x  uvec.y  uvec.z  −P·uvec ]
        [ fvec.x  fvec.y  fvec.z  −P·fvec + Z_bias ]
        [   0       0       0        1    ]          P = viewPos
```

Applied to a column vector `v = (x,y,z,1)` this produces
`(v − P)·rvec, (v − P)·uvec, (v − P)·fvec + Z_bias`, which is exactly the
view-space transform `(P − View_position) · View_matrix`.

---

## 4. Projection

### `g3_ProjectPoint(point)`

Performs the perspective divide. Given a view-space point with depth `z`:

```
one_over_z = 1 / z
sx = Window_w2 + x · Window_w2 · one_over_z
sy = Window_h2 − y · Window_h2 · one_over_z
```

This is the classic pinhole projection onto a plane at distance `z = 1`
(Window_w2 = width/2). Because the frustum is `−z ≤ x ≤ z`, a point on the
frustum edge (`x = z`) maps to `sx = Window_w2 + Window_w2 = width`, i.e. the
right edge of the screen. The `−` on `sy` flips Y from the math convention
(+Y up) to the raster convention (+Y down).

The function is a no-op if the point is already projected (`PF_PROJECTED`) or
behind the camera (`CC_BEHIND`).

### `g3_GetProjectionMatrix(zoom, projMat)`

Builds the OpenGL-style column-major projection matrix. With
`vertical_fov = zoom·3/4` and `oOT = 1/vertical_fov`:

```
if (aspect s = w/h ≤ 1):   projMat[0] = oOT,  projMat[5] = oOT·s
else:                      projMat[0] = oOT/s, projMat[5] = oOT
projMat[10] = 1, projMat[11] = 1, projMat[14] = −1
```

i.e.

```
[ oOT     0     0  0 ]
[  0    oOT·s   0  0 ]
[  0      0     1 −1 ]
[  0      0     1  0 ]
```

This is the standard symmetric OpenGL perspective matrix
(see [Song Ho Ahn, "OpenGL Projection Matrix"](https://www.songho.ca/opengl/gl_projectionmatrix.html))
with the near plane at 0 and the far plane at infinity. The w row
`(0,0,1,0)` makes clip-space `w = z`, so the perspective divide
`x_ndc = x_clip / w` reproduces the `x/z` division of `g3_ProjectPoint`.

### `g3_GetViewPortMatrix(viewMat)`

Builds the viewport matrix mapping NDC `[−1,1]²` to the renderer's clip
rectangle:

```
[ w/2  0   0  w/2 + x ]
[ 0  −h/2  0  h/2 + y ]
[ 0   0    1    0     ]
[ 0   0    0    1     ]
```

### `g3_Point2Vec(v, sx, sy)`

Inverse projection: given a screen pixel, computes the world-space unit
vector through that pixel (used for picking). It un-projects the pixel into
view space, normalizes, and rotates by the inverse of the unscaled view
matrix:

```
tempv.x = ((sx − w2)/w2) · (scale.z / scale.x)
tempv.y = −((sy − h2)/h2) · (scale.z / scale.y)
tempv.z = 1
normalize(tempv)
v = tempv · ~Unscaled_matrix
```

### `g3_CalcPointDepth(pnt)`

Returns the view-space depth of a world point without a full rotation — the
dot product of the camera-relative vector with the forward vector:

```
depth = (P − View_position) · View_matrix.fvec
```

---

## 5. Point Coding (Frustum Classification)

### `g3_CodePoint(point)`

Classifies a view-space point against the frustum planes and stores the
result in `p3_codes`:

```
x >  z  → CC_OFF_RIGHT      x < −z → CC_OFF_LEFT
y >  z  → CC_OFF_TOP        y < −z → CC_OFF_BOT
z <  0  → CC_BEHIND         z > Far_clip_z → CC_OFF_FAR
```

If a custom clip plane is active, the point is also tested against it. The
plane is stored in view space (`Clip_plane`), so the point is transformed
back to the unscaled frame before the signed-distance test:

```
vec = (p3_vec − Clip_plane_point) / Matrix_scale
if (vec · Clip_plane < −0.005) → CC_OFF_CUSTOM
```

### `g3_RotatePoint(dest, src)`

Transforms a world point into view space and codes it:

```
dest.p3_vecPreRot = src
dest.p3_vec = (src − View_position) · View_matrix
dest.p3_flags = PF_ORIGPOINT
return g3_CodePoint(dest)
```

### Delta rotation functions

These rotate **direction vectors** (not positions) by the view matrix, used
for muzzle flashes, sparks, and other small offsets:

| Function | Result |
|----------|--------|
| `g3_RotateDeltaX(dest, dx)` | `dest = (rvec.x·dx, uvec.x·dx, fvec.x·dx)` — the world-space X axis rotated into view space, scaled by `dx` |
| `g3_RotateDeltaY(dest, dy)` | same using the Y axis |
| `g3_RotateDeltaZ(dest, dz)` | same using the Z axis |
| `g3_RotateDeltaVec(dest, src)` | `dest = src · View_matrix` |
| `g3_AddDeltaVec(dest, src, deltav)` | `dest.p3_vec = src.p3_vec + deltav` (view space), clears `PF_PROJECTED`, re-codes |

---

## 6. Clipping

The clipper is a classic **Sutherland–Hodgman** polygon clipper. It clips a
polygon against each frustum plane in turn, producing a new vertex list in a
double buffer (`Vbuf0`/`Vbuf1`). New vertices are allocated from a fixed pool
of `MAX_POINTS_IN_POLY` (100) temp points.

### Temp-point pool

- `InitFreePoints()` — resets the pool (called by `g3_StartFrame`).
- `GetTempPoint()` — allocates a point, sets `PF_TEMP_POINT`.
- `FreeTempPoint(p)` — returns a point to the pool.
- `CheckTempPoints()` — debug assert that the pool is empty at frame end.

### `g3_ClipPolygon(pointlist, nv, cc)`

Clips the polygon described by `pointlist`/`nv` against every plane flagged
in `cc->cc_or`. Returns a pointer to the clipped vertex list and updates
`*nv`. If `cc->cc_and` becomes non-zero the polygon is entirely outside a
plane and is rejected.

**Caller must call `g3_FreeTempPoints()` on the returned list when done.**

### `g3_FreeTempPoints(pointlist, nv)`

Frees every `PF_TEMP_POINT` in the list back to the pool.

### `ClipPlane(plane_flag, src, dest, nv, cc)`

Clips one polygon against one plane. For each edge `(prev, i, next)` it
emits the intersection point when an edge crosses the plane, and frees
discarded temp points. This is the Sutherland–Hodgman "output the
intersection when crossing" rule.

### `ClipEdge(plane_flag, on_pnt, off_pnt)`

Computes the intersection of the edge from `on_pnt` (inside) to `off_pnt`
(outside) with the given plane, using linear interpolation. For the
left/right/top/bottom planes the interpolation factor is derived from the
plane equation `x = ±z` (or `y = ±z`):

```
k = (a − z_on) / ((a − z_on) − b + z_off)     // a,b = signed x or y
```

The new point's position, UVs, lightmap UVs, and lighting are all linearly
interpolated by `k`. `ClipFarEdge` and `ClipCustomEdge` are specialized
variants for the far plane and the custom plane.

### `ClipLine(p0, p1, codes_or)`

Clips a line segment against all flagged planes (used by `g3_DrawLine` and
`g3_DrawSpecialLine`). It repeatedly swaps so the inside point is `p0`,
clips the edge, and frees the replaced temp point.

### Clip-plane control

| Function | Effect |
|----------|--------|
| `g3_SetFarClipZ(z)` | sets the far-plane distance `Far_clip_z` |
| `g3_ResetFarClipZ()` | sets `Far_clip_z = FLT_MAX` (disables far clipping) |
| `g3_SetCustomClipPlane(state, pnt, normal)` | enables/disables a custom clip plane. The plane is transformed into view space: the point becomes `(pnt − View_position)·View_matrix` and the normal becomes `normal · Unscaled_matrix` (normalized). Used for mirror/water clipping. |

---

## 7. Drawing

### `g3_DrawPoly(nv, pointlist, bm, map_type, clip_codes)`

The core polygon draw. In the current hardware renderer it is a thin wrapper
that dispatches directly to `rend_DrawPolygon3D` (the software clipper is
bypassed because the GL pipeline clips in hardware). The historical software
implementation (kept in a comment block) performed the full pipeline:

1. Compute `cc_or`/`cc_and` from the vertices (or use the passed-in codes).
2. Reject if `cc_and` (all vertices off one plane).
3. Clip if `cc_or` (some vertex off a plane) via `g3_ClipPolygon`.
4. Project any unprojected vertices.
5. Submit to `rend_DrawPolygon3D`.
6. Free temp points.

Returns 1 if drawn, 0 if clipped away.

### `g3_DrawPolyList(ntri, pointlist, bm, map_type)`

Added for bug #560. Draws `ntri` triangles (3 vertices each) in a single
draw call, dispatching to `rend_DrawPolygonList3D`. Used by the terrain
renderer to batch many cells sharing a texture/lightmap into one
`GL_TRIANGLES` call instead of one `GL_TRIANGLE_FAN` per cell.

### `g3_CheckAndDrawPoly(nv, pointlist, bm, norm, pnt)`

Backface culling + draw. If `norm` is given it uses `g3_CheckNormalFacing`;
otherwise it computes the normal from the first three vertices with
`vm_GetPerp` (cross product) and tests its orientation. Draws only if facing.

### `g3_CheckNormalFacing(v, norm)`

Returns true if the plane at world point `v` with normal `norm` faces the
viewer:

```
facing = (View_position − v) · norm > 0
```

### `g3_DrawLine(color, p0, p1)`

Draws a 3D line. Rejects if both endpoints share any off-screen code,
clips if either is off-screen, projects, and calls `rend_DrawLine` with the
rounded screen coordinates. Frees temp points after clipping.

### `g3_DrawSpecialLine(p0, p1)`

Like `g3_DrawLine` but uses the current render states (texture, alpha, etc.)
via `rend_DrawSpecialLine`, which submits `GL_LINES` with a depth value
derived from `1 − 1/(z + Z_bias)`.

### `g3_DrawSphere(color, pnt, rad)`

Draws a screen-space filled circle at the projected center of a 3D point.
The 2D radius is proportional to the 3D radius and inversely proportional to
depth:

```
r2d = rad · Matrix_scale.x · Window_w2 / z
```

### `g3_DrawBitmap(pos, width, height, bm, color)`

Draws a camera-facing textured quad centered at `pos`. The four corners are
computed parallel to the view frame using the unscaled view matrix's right
and up vectors, rotated, UV-mapped to the full texture, and drawn as a
4-vertex polygon. Used for sprites, gauges, and billboards.

### `g3_DrawRotatedBitmap(pos, rot_angle, width, height, bm, color)`

Like `g3_DrawBitmap` but the quad is rotated about its center by
`rot_angle` (a 2D rotation in the view plane, built with
`vm_AnglesToMatrix`).

### `g3_DrawPlanarRotatedBitmap(pos, norm, rot_angle, width, height, bm)`

Draws a textured quad lying on an arbitrary world plane (defined by `norm`).
The plane's orientation matrix is built with `vm_VectorToMatrix` and
transposed; the quad is rotated by `rot_angle` within that plane. Used for
decal-style effects on arbitrary surfaces.

### `g3_DrawBox(color, pnt, rad)`

Draws a screen-aligned wireframe box (4 lines) centered on the projected
point, with half-extent `rad` scaled by depth. Used by the editor to show
the selected object.

### `g3_SetTriangulationTest(state)`

Toggles the (historical) fan-to-triangle triangulation path in
`g3_DrawPoly` for polygons with more than 3 vertices.

---

## 8. Instancing (Model Transforms)

Instancing lets the renderer draw an object at an arbitrary position and
orientation without re-rotating every vertex by hand. It works by
**re-basing the view transform** so that the object appears at the origin.

### `g3_StartInstanceMatrix(pos, orient)`

Pushes the current view state onto a stack (depth ≤ 30), then:

```
View_position = (View_position − pos) · orient      // camera relative to object
View_matrix   = ~orient · View_matrix               // rotate view by object
Unscaled_matrix = ~orient · Unscaled_matrix
```

After this, world points are transformed as if the object were at the origin
with identity orientation, so the caller can submit object-local vertices
directly. The model-view matrix is rebuilt and the full transform refreshed.

### `g3_StartInstanceAngles(pos, angles)`

Same, but builds the orientation matrix from pitch/heading/bank angles
(`vm_AnglesToMatrix`). A `NULL` angles pointer uses the identity matrix.

### `g3_DoneInstance()`

Pops the previous view state and refreshes the transform.

---

## 9. Transform Matrices (GL Integration)

The library maintains four 4×4 column-major matrices:

| Matrix | Contents |
|--------|----------|
| `gTransformModelView` | model-view (camera) transform |
| `gTransformProjection` | perspective projection |
| `gTransformViewPort` | NDC → screen viewport |
| `gTransformFull` | `ModelView · Projection · ViewPort` |

### `g3_TransformMult(res, a, b)`

Standard 4×4 matrix multiply: `res = a · b`.

### `g3_TransformTrans(res, t)`

4×4 transpose.

### `g3_UpdateFullTransform()`

Recomputes `gTransformFull = ModelView · Projection · ViewPort`.

### `g3_RefreshTransforms(usePassthru)`

Synchronizes the GL transform stack with the g3 matrices:

- `usePassthru == true` → sets GL to pass-through (identity), used for 2D
  drawing and special lines that already carry screen coordinates.
- `usePassthru == false` → uploads the viewport, projection, and model-view
  matrices to GL, used for 3D polygon drawing.

The state is cached so repeated calls with the same mode are cheap.

---

## 10. The Complete Pipeline

Putting it together, a typical frame renders like this:

```
g3_StartFrame(&view_pos, &view_matrix, zoom)     // set camera, FOV, matrices
  for each object:
    g3_StartInstanceMatrix(&obj.pos, &obj.orient) // object-local space
    for each polygon:
      for each vertex:
        g3_RotatePoint(&pt, &world_vertex)        // world → view space + code
      g3_CheckAndDrawPoly(nv, pts, bm, norm, pnt) // cull, clip, project, draw
    g3_DoneInstance()
g3_EndFrame()
```

The hardware renderer offloads clipping and projection to the GPU: vertices
are submitted with their **world** positions (`p3_vecPreRot`) and the GL
transform stack applies the model-view + projection matrices. The software
`g3_RotatePoint`/`g3_CodePoint`/`g3_ClipPolygon`/`g3_ProjectPoint` path
remains for CPU-side culling, picking, and the 2D/overlay paths.

---

## 11. References

- [Song Ho Ahn — OpenGL Projection Matrix](https://www.songho.ca/opengl/gl_projectionmatrix.html) — the projection matrix math used by `g3_GetProjectionMatrix`.
- [Song Ho Ahn — OpenGL Transformation](https://www.songho.ca/opengl/gl_transform.html) — the model-view/projection/viewport transform chain.
- Sutherland, I. E. and Hodgman, G. W., *Reentrant Polygon Clipping*, Communications of the ACM, 1974 — the polygon clipping algorithm used by `g3_ClipPolygon`.
- Descent 1/2 source (Parallax Software) — the original `g3` library this code descends from.