#include "WeaponImpact.h"

#include "3d.h"
#include "bitmap.h"
#include "grdefs.h"
#include "renderer.h"
#include "vecmat.h"

#include <cmath>

// Plasma impact fan: a center vertex plus a ring, drawn as a triangle fan.
// 24 ring segments make a smooth circle (6 showed visible hexagon edges).
#define PLASMA_RING_VERTS 24
#define PLASMA_TOTAL_VERTS (PLASMA_RING_VERTS + 1) // 25 total

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Procedural gradient textures: 64x64 1555 bitmaps generated once and
// cached. The ball texture is a white-hot core fading to a green rim (maps
// onto the sphere so poles glow white, equator runs green). The explosion
// texture is a circular soft-falloff disc: opaque white center fading
// through green to transparent edge.
#define PLASMA_TEX_SIZE 64

static int PlasmaBallTexHandle = -1;
static int PlasmaExplosionTexHandle = -1;

// Packs 8-bit RGB + opaque flag into a 1555 pixel
static uint16_t Pack1555(uint8_t r, uint8_t g, uint8_t b, bool opaque) {
  uint16_t px = (uint16_t)(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
  if (opaque)
    px |= OPAQUE_FLAG16;
  return px;
}

static int MakeRadialGradientTexture(bool hard_edge) {
  int bm = bm_AllocBitmap(PLASMA_TEX_SIZE, PLASMA_TEX_SIZE, 0);
  if (bm < 0)
    return -1;

  uint16_t *data = bm_data(bm, 0);
  if (!data) {
    bm_FreeBitmap(bm);
    return -1;
  }

  const float half = PLASMA_TEX_SIZE * 0.5f;
  for (int y = 0; y < PLASMA_TEX_SIZE; y++) {
    for (int x = 0; x < PLASMA_TEX_SIZE; x++) {
      float dx = (x + 0.5f - half) / half;
      float dy = (y + 0.5f - half) / half;
      float dist = sqrtf(dx * dx + dy * dy); // 0 center .. ~1.41 corner
      if (dist > 1.0f)
        dist = 1.0f;

      // White-hot core -> green mid -> deep green rim
      float core = 1.0f - dist;
      if (core < 0.0f)
        core = 0.0f;
      // Smoothstep the core for a hot center that falls off fast
      float hot = core * core * (3.0f - 2.0f * core);
      uint8_t r = (uint8_t)(60 + 195 * hot);
      uint8_t g = (uint8_t)(180 + 75 * hot);
      uint8_t b = (uint8_t)(70 + 60 * hot * hot);

      bool opaque = hard_edge ? true : (dist < 0.98f);
      data[y * PLASMA_TEX_SIZE + x] = Pack1555(r, g, b, opaque);
    }
  }

  bm_SetBitmapIfTransparent(bm);
  return bm;
}

int GetPlasmaBallTexture() {
  if (PlasmaBallTexHandle < 0)
    PlasmaBallTexHandle = MakeRadialGradientTexture(true);
  return PlasmaBallTexHandle;
}

int GetPlasmaExplosionTexture() {
  if (PlasmaExplosionTexHandle < 0)
    PlasmaExplosionTexHandle = MakeRadialGradientTexture(false);
  return PlasmaExplosionTexHandle;
}

void FreePlasmaImpactTextures() {
  if (PlasmaBallTexHandle >= 0) {
    bm_FreeBitmap(PlasmaBallTexHandle);
    PlasmaBallTexHandle = -1;
  }
  if (PlasmaExplosionTexHandle >= 0) {
    bm_FreeBitmap(PlasmaExplosionTexHandle);
    PlasmaExplosionTexHandle = -1;
  }
}

int GeneratePlasmaImpactMesh(const vector &center, const vector &normal, const vector &right,
                             const vector &up, float size, float age, vector *outVerts, g3UVL *outUvs) {
  // Push the mesh slightly off the wall so it doesn't z-fight
  vector offset = normal * 0.05f;

  // Center vertex
  outVerts[0] = center - offset;
  outUvs[0].u = 0.5f;
  outUvs[0].v = 0.5f;

  // Ring vertices — smooth circle (24 segments); radius pulses gently
  // with age for a living look
  for (int i = 0; i < PLASMA_RING_VERTS; i++) {
    float angle = (2.0f * (float)M_PI * i) / PLASMA_RING_VERTS;
    // Modulate radius with age for a pulse effect
    float pulse = 0.05f * sinf(age * 6.28318f + (float)i * 0.5f);
    float r = size * (1.0f + pulse);
    vector dir = (right * cosf(angle) + up * sinf(angle)) * r;
    outVerts[i + 1] = center + dir - offset;

    // UV: spread around the center at (0.5, 0.5)
    outUvs[i + 1].u = 0.5f + 0.5f * cosf(angle);
    outUvs[i + 1].v = 0.5f + 0.5f * sinf(angle);
  }

  return PLASMA_TOTAL_VERTS;
}

bool DrawPlasmaImpact3D(int roomnum, int facenum, const g3Point *quadPoints, float size, float age) {
  (void)roomnum;
  (void)facenum;

  // Derive the impact center and face basis from the 2D scorch quad.
  // quadPoints are the 4 rotated corners (TL, TR, BR, BL).
  vector center = (quadPoints[0].p3_vec + quadPoints[2].p3_vec) * 0.5f;
  vector right = (quadPoints[1].p3_vec - quadPoints[0].p3_vec) * 0.5f;
  vector up = (quadPoints[0].p3_vec - quadPoints[3].p3_vec) * 0.5f;

  // Face normal from the quad winding
  vector edge1 = quadPoints[1].p3_vec - quadPoints[0].p3_vec;
  vector edge2 = quadPoints[3].p3_vec - quadPoints[0].p3_vec;
  vector normal;
  vm_CrossProduct(&normal, &edge1, &edge2);
  if (vm_NormalizeVector(&normal) == 0.0f)
    return false; // degenerate quad — fall back to 2D sprite

  vector verts[PLASMA_TOTAL_VERTS];
  g3UVL uvs[PLASMA_TOTAL_VERTS];
  int nverts = GeneratePlasmaImpactMesh(center, normal, right, up, size, age, verts, uvs);

  // Rotate into view space and draw as a triangle fan
  g3Point points[PLASMA_TOTAL_VERTS];
  g3Point *pointlist[PLASMA_TOTAL_VERTS];
  int codes_and = 0xFF;
  for (int i = 0; i < nverts; i++) {
    codes_and &= g3_RotatePoint(&points[i], &verts[i]);
    pointlist[i] = &points[i];
    points[i].p3_u = uvs[i].u;
    points[i].p3_v = uvs[i].v;
    points[i].p3_a = 1.0f - age;
    points[i].p3_flags |= PF_UV | PF_RGBA;
  }
  if (codes_and)
    return true; // fully clipped — 3D path was used, nothing to draw

  // BUGFIX: Use the textured path (TT_LINEAR) so the fan keeps real UVs.
  // TT_FLAT routes through gpu_DrawFlatPolygon3D, which default-constructs
  // all UVs — the glow shader then sampled garbage and the fan rendered as
  // a solid planar slab. The mesh generator already emits radial UVs
  // (center 0.5,0.5, ring on the unit circle) for the glow falloff.
  rend_SetAlphaType(AT_SATURATE_TEXTURE_VERTEX);
  rend_SetAlphaValue(255);
  rend_SetLighting(LS_FLAT_GOURAUD);
  rend_SetColorModel(CM_MONO);
  rend_SetOverlayType(OT_NONE);
  rend_SetTextureType(TT_LINEAR);
  rend_SetZBias(-0.5f);
  rend_SetZBufferWriteMask(0);

  // Plasma green tints the textured sample; the fragment shader adds the
  // radial glow on top via u_plasma_glow.
  rend_SetFlatColor(GR_RGB(77, 255, 102));
  rend_SetPlasmaGlow(1.0f - age, 0.3f, 1.0f, 0.4f);
  rend_SetEffectAge(age);

  // NOTE: caller passes the scorch bitmap; 0 = untextured (glow only).
  g3_DrawPoly(nverts, pointlist, 0);

  rend_SetPlasmaGlow(0.0f, 0.0f, 0.0f, 0.0f);
  rend_SetEffectAge(0.0f);
  rend_SetZBias(0.0f);
  rend_SetZBufferWriteMask(1);

  return true;
}

void DrawPlasmaTrailSegment(const vector &pos, const vector &end_pos, float width, float age,
                            uint16_t color16, float alpha, int bm_handle) {
  // Trail axis (head -> tail)
  vector axis = end_pos - pos;
  float axis_len = vm_GetMagnitudeFast(&axis);
  if (axis_len < 0.001f) {
    axis = pos;
    axis_len = 1.0f;
  } else {
    axis /= axis_len;
  }

  // Camera-facing side vector: cross of axis with view direction
  vector viewer_pos;
  g3_GetViewPosition(&viewer_pos);
  vector to_viewer = viewer_pos - pos;
  vm_NormalizeVector(&to_viewer);

  vector side;
  vm_CrossProduct(&side, &axis, &to_viewer);
  if (vm_NormalizeVector(&side) == 0.0f) {
    // Axis points at the camera — pick any perpendicular
    vector fallback = {1.0f, 0.0f, 0.0f};
    vm_CrossProduct(&side, &axis, &fallback);
    vm_NormalizeVector(&side);
  }

  // Second plane perpendicular to the first for a volumetric cross
  vector side2;
  vm_CrossProduct(&side2, &axis, &side);
  vm_NormalizeVector(&side2);

  // Fade width with age; alpha comes from the caller (matches the 2D
  // billboard's lifeleft/lifetime * 0.7 curve)
  if (alpha < 0.0f)
    alpha = 0.0f;
  if (alpha > 1.0f)
    alpha = 1.0f;
  float w = width * (0.4f + 0.6f * (1.0f - age));

  // Decode the 16-bit plasma color to 0..1 floats for the glow uniform
  ddgr_color c32 = GR_16_TO_COLOR(color16);
  float cr = GR_COLOR_RED(c32) / 255.0f;
  float cg = GR_COLOR_GREEN(c32) / 255.0f;
  float cb = GR_COLOR_BLUE(c32) / 255.0f;

  // BUGFIX: Use the textured path (TT_LINEAR) with the weapon's own smoke
  // bitmap instead of TT_FLAT. The flat path (gpu_DrawFlatPolygon3D)
  // default-constructs all UVs, so the fragment shader's radial glow
  // (length(vertex_uv0 - 0.5)) sampled garbage and the quad rendered as a
  // solid planar slab. Textured quads keep real UVs, so the glow falloff
  // shapes the quad into a soft volumetric blob.
  rend_SetOverlayType(OT_NONE);
  if (bm_handle >= 0)
    rend_SetTextureType(TT_LINEAR);
  else
    rend_SetTextureType(TT_FLAT);
  rend_SetLighting(LS_FLAT_GOURAUD);
  rend_SetColorModel(CM_MONO);
  rend_SetAlphaType(AT_SATURATE_TEXTURE_VERTEX);
  rend_SetAlphaValue(255);
  rend_SetZBufferWriteMask(0);

  // Flat color tints the textured sample (LS_FLAT_GOURAUD path)
  rend_SetFlatColor(c32);

  rend_SetPlasmaGlow(1.0f - age, cr, cg, cb);
  rend_SetEffectAge(age);

  // Draw two crossed quads (volumetric cross) along the trail axis.
  // UVs span the full 0..1 range so the texture shows and the fragment
  // shader's radial glow math (length(uv - 0.5)) produces a soft blob
  // falloff toward the quad edges.
  const vector *sides[2] = {&side, &side2};
  for (int plane = 0; plane < 2; plane++) {
    const vector &s = *sides[plane];
    vector corners[4];
    corners[0] = pos + s * w;
    corners[1] = pos - s * w;
    corners[2] = end_pos - s * w;
    corners[3] = end_pos + s * w;

    g3Point pnts[4];
    g3Point *pntlist[4];
    int codes_and = 0xFF;
    static const float uvs[4][2] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    for (int i = 0; i < 4; i++) {
      codes_and &= g3_RotatePoint(&pnts[i], &corners[i]);
      pntlist[i] = &pnts[i];
      pnts[i].p3_u = uvs[i][0];
      pnts[i].p3_v = uvs[i][1];
      pnts[i].p3_a = alpha;
      pnts[i].p3_flags |= PF_UV | PF_RGBA;
    }
    if (!codes_and)
      g3_DrawPoly(4, pntlist, bm_handle >= 0 ? bm_handle : 0);
  }

  rend_SetPlasmaGlow(0.0f, 0.0f, 0.0f, 0.0f);
  rend_SetEffectAge(0.0f);
  rend_SetZBufferWriteMask(1);
}

void DrawPlasmaBlob3D(const vector &pos, float size, float age, uint16_t color16, float alpha,
                      int bm_handle) {
  // Camera basis: build a view-aligned frame at the blob center
  vector viewer_pos;
  g3_GetViewPosition(&viewer_pos);
  vector view_dir = viewer_pos - pos;
  if (vm_NormalizeVector(&view_dir) == 0.0f)
    return;

  // Pick a world-up reference that isn't parallel to the view direction
  vector world_up = {0.0f, 1.0f, 0.0f};
  if (fabsf(view_dir.y()) > 0.95f)
    world_up = {1.0f, 0.0f, 0.0f};

  vector side;
  vm_CrossProduct(&side, &view_dir, &world_up);
  vm_NormalizeVector(&side);
  vector up;
  vm_CrossProduct(&up, &side, &view_dir);
  vm_NormalizeVector(&up);

  // Octahedron vertices: ±view (poles), ±side / ±up (equator)
  float r = size * 0.5f;
  vector apex = pos + view_dir * r;
  vector base = pos - view_dir * r;
  vector eq[4] = {pos + side * r, pos + up * r, pos - side * r, pos - up * r};

  ddgr_color c32 = GR_16_TO_COLOR(color16);
  float cr = GR_COLOR_RED(c32) / 255.0f;
  float cg = GR_COLOR_GREEN(c32) / 255.0f;
  float cb = GR_COLOR_BLUE(c32) / 255.0f;

  if (alpha < 0.0f)
    alpha = 0.0f;
  if (alpha > 1.0f)
    alpha = 1.0f;

  rend_SetOverlayType(OT_NONE);
  if (bm_handle >= 0)
    rend_SetTextureType(TT_LINEAR);
  else
    rend_SetTextureType(TT_FLAT);
  rend_SetLighting(LS_FLAT_GOURAUD);
  rend_SetColorModel(CM_MONO);
  rend_SetAlphaType(AT_SATURATE_TEXTURE_VERTEX);
  rend_SetAlphaValue(255);
  rend_SetZBufferWriteMask(0);

  rend_SetFlatColor(c32);
  rend_SetPlasmaGlow(1.0f - age, cr, cg, cb);
  rend_SetEffectAge(age);

  // 8 triangles: 4 around the apex, 4 around the base.
  // UVs map each face into a quadrant of the texture so the smoke bitmap
  // shows on every face; the glow shader adds radial falloff per-face.
  const vector *tips[2] = {&apex, &base};
  for (int half = 0; half < 2; half++) {
    for (int e = 0; e < 4; e++) {
      vector corners[3] = {*tips[half], eq[e], eq[(e + 1) % 4]};
      // Winding: apex faces outward, base faces reversed
      g3Point pnts[3];
      g3Point *pntlist[3];
      int codes_and = 0xFF;
      static const float tri_uvs[3][2] = {{0.5f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}};
      for (int i = 0; i < 3; i++) {
        int idx = (half == 0) ? i : (2 - i + 3) % 3;
        codes_and &= g3_RotatePoint(&pnts[i], &corners[idx]);
        pntlist[i] = &pnts[i];
        pnts[i].p3_u = tri_uvs[i][0];
        pnts[i].p3_v = tri_uvs[i][1];
        pnts[i].p3_a = alpha;
        pnts[i].p3_flags |= PF_UV | PF_RGBA;
      }
      if (!codes_and)
        g3_DrawPoly(3, pntlist, bm_handle >= 0 ? bm_handle : 0);
    }
  }

  rend_SetPlasmaGlow(0.0f, 0.0f, 0.0f, 0.0f);
  rend_SetEffectAge(0.0f);
  rend_SetZBufferWriteMask(1);
}

// UV sphere resolution for the plasma ball / shockball meshes
#define PLASMA_BALL_SEGMENTS 10
#define PLASMA_BALL_RINGS 7

// Shared render-state setup for plasma 3D draws. Returns the decoded color.
static ddgr_color PlasmaBeginDraw(float age, uint16_t color16, int bm_handle, float *out_rgb) {
  ddgr_color c32 = GR_16_TO_COLOR(color16);
  out_rgb[0] = GR_COLOR_RED(c32) / 255.0f;
  out_rgb[1] = GR_COLOR_GREEN(c32) / 255.0f;
  out_rgb[2] = GR_COLOR_BLUE(c32) / 255.0f;

  rend_SetOverlayType(OT_NONE);
  if (bm_handle >= 0)
    rend_SetTextureType(TT_LINEAR);
  else
    rend_SetTextureType(TT_FLAT);
  rend_SetLighting(LS_FLAT_GOURAUD);
  rend_SetColorModel(CM_MONO);
  rend_SetAlphaType(AT_SATURATE_TEXTURE_VERTEX);
  rend_SetAlphaValue(255);
  rend_SetZBufferWriteMask(0);

  rend_SetFlatColor(c32);
  rend_SetPlasmaGlow(1.0f - age, out_rgb[0], out_rgb[1], out_rgb[2]);
  rend_SetEffectAge(age);
  return c32;
}

static void PlasmaEndDraw() {
  rend_SetPlasmaGlow(0.0f, 0.0f, 0.0f, 0.0f);
  rend_SetEffectAge(0.0f);
  rend_SetZBufferWriteMask(1);
}

// Emits a UV sphere centered at pos with the given radius. Assumes plasma
// render state is already set up. alpha is per-vertex fade.
static void PlasmaEmitSphere(const vector &pos, float radius, float alpha, int bm_handle) {
  vector grid[PLASMA_BALL_RINGS + 1][PLASMA_BALL_SEGMENTS + 1];
  for (int ring = 0; ring <= PLASMA_BALL_RINGS; ring++) {
    float theta = (float)M_PI * ring / PLASMA_BALL_RINGS;
    float sin_t = sinf(theta), cos_t = cosf(theta);
    for (int seg = 0; seg <= PLASMA_BALL_SEGMENTS; seg++) {
      float phi = 2.0f * (float)M_PI * seg / PLASMA_BALL_SEGMENTS;
      grid[ring][seg] =
          pos + vector{radius * sin_t * cosf(phi), radius * cos_t, radius * sin_t * sinf(phi)};
    }
  }

  for (int ring = 0; ring < PLASMA_BALL_RINGS; ring++) {
    float v0 = (float)ring / PLASMA_BALL_RINGS;
    float v1 = (float)(ring + 1) / PLASMA_BALL_RINGS;
    for (int seg = 0; seg < PLASMA_BALL_SEGMENTS; seg++) {
      float u0 = (float)seg / PLASMA_BALL_SEGMENTS;
      float u1 = (float)(seg + 1) / PLASMA_BALL_SEGMENTS;

      const vector *cp[4] = {&grid[ring][seg], &grid[ring][seg + 1], &grid[ring + 1][seg + 1],
                             &grid[ring + 1][seg]};
      float cu[4] = {u0, u1, u1, u0};
      float cv[4] = {v0, v0, v1, v1};

      g3Point pnts[4];
      g3Point *pntlist[4];
      int codes_and = 0xFF;
      int nv = 4;
      int src_idx[4] = {0, 1, 2, 3};
      if (ring == 0) {
        src_idx[0] = 0;
        src_idx[1] = 2;
        src_idx[2] = 3;
        nv = 3;
      } else if (ring == PLASMA_BALL_RINGS - 1) {
        src_idx[0] = 0;
        src_idx[1] = 1;
        src_idx[2] = 2;
        nv = 3;
      }
      for (int i = 0; i < nv; i++) {
        int s = src_idx[i];
        codes_and &= g3_RotatePoint(&pnts[i], const_cast<vector *>(cp[s]));
        pntlist[i] = &pnts[i];
        pnts[i].p3_u = cu[s];
        pnts[i].p3_v = cv[s];
        pnts[i].p3_a = alpha;
        pnts[i].p3_flags |= PF_UV | PF_RGBA;
      }
      if (!codes_and)
        g3_DrawPoly(nv, pntlist, bm_handle >= 0 ? bm_handle : 0);
    }
  }
}

void DrawPlasmaBall3D(const vector &pos, float size, float age, uint16_t color16, float alpha,
                      int bm_handle) {
  if (alpha < 0.0f)
    alpha = 0.0f;
  if (alpha > 1.0f)
    alpha = 1.0f;

  float rgb[3];
  PlasmaBeginDraw(age, color16, bm_handle, rgb);
  PlasmaEmitSphere(pos, size * 0.5f, alpha, bm_handle);
  PlasmaEndDraw();
}

void DrawPlasmaShockball3D(const vector &pos, const vector &wall_normal, float size, float age,
                           uint16_t color16, float alpha, int bm_handle) {
  if (alpha < 0.0f)
    alpha = 0.0f;
  if (alpha > 1.0f)
    alpha = 1.0f;

  float rgb[3];
  PlasmaBeginDraw(age, color16, bm_handle, rgb);

  // Expanding fireball: grows fast then fades. Radius peaks at ~60% life.
  float growth = (age < 0.6f) ? (age / 0.6f) : (1.0f - (age - 0.6f) / 0.4f * 0.4f);
  if (growth < 0.15f)
    growth = 0.15f;
  PlasmaEmitSphere(pos, size * 0.5f * growth, alpha, bm_handle);

  // Wall-aligned impact fan: a glow disc hugging the wall, expanding with age.
  vector n = wall_normal;
  if (vm_NormalizeVector(&n) != 0.0f) {
    // Build an orthonormal basis on the wall plane
    vector ref = {0.0f, 1.0f, 0.0f};
    if (fabsf(n.y()) > 0.95f)
      ref = {1.0f, 0.0f, 0.0f};
    vector right, up;
    vm_CrossProduct(&right, &n, &ref);
    vm_NormalizeVector(&right);
    vm_CrossProduct(&up, &right, &n);
    vm_NormalizeVector(&up);

    float fan_r = size * (0.4f + 0.9f * age);
    vector offset = n * 0.05f;
    vector center = pos + offset;

    vector verts[PLASMA_TOTAL_VERTS];
    g3UVL uvs[PLASMA_TOTAL_VERTS];
    // Reuse the impact mesh generator with the wall basis (age drives pulse)
    GeneratePlasmaImpactMesh(center, n, right * fan_r, up * fan_r, 1.0f, age, verts, uvs);

    g3Point points[PLASMA_TOTAL_VERTS];
    g3Point *pointlist[PLASMA_TOTAL_VERTS];
    int codes_and = 0xFF;
    for (int i = 0; i < PLASMA_TOTAL_VERTS; i++) {
      codes_and &= g3_RotatePoint(&points[i], &verts[i]);
      pointlist[i] = &points[i];
      points[i].p3_u = uvs[i].u;
      points[i].p3_v = uvs[i].v;
      points[i].p3_a = alpha;
      points[i].p3_flags |= PF_UV | PF_RGBA;
    }
    if (!codes_and)
      g3_DrawPoly(PLASMA_TOTAL_VERTS, pointlist, bm_handle >= 0 ? bm_handle : 0);
  }

  PlasmaEndDraw();
}
