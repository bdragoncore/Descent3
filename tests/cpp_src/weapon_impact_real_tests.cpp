/**
 * @file weapon_impact_real_tests.cpp
 * @brief Procedural 3D plasma impact mesh tests (Descent3/WeaponImpact.cpp).
 *
 * @details
 * Links the real WeaponImpact.cpp. The mesh generator is pure math over
 * vecmat/3d types; renderer/3d entry points are captured by stubs so
 * DrawPlasmaImpact3D / DrawPlasmaTrailSegment behavior can be asserted
 * without a GPU.
 *
 * This harness validates the behavior of `Descent3/WeaponImpact.cpp`. It
 * compiles the real implementation against stubbed engine subsystems and
 * checks the observable contract documented in WeaponImpact.h.
 *
 * @par Source
 * `Descent3/WeaponImpact.cpp`
 * @par Harness
 * `weapon_impact_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/WeaponImpact.cpp
 */

#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "3d.h"
#include "grdefs.h"
#include "renderer.h"
#include "vecmat.h"
#include "WeaponImpact.h"

// Pull in the real implementation so mesh generation and draw paths
// are test-visible.
#include "../../Descent3/WeaponImpact.cpp"

// ---------------------------------------------------------------------------
// Stubs with capture state
// ---------------------------------------------------------------------------
struct RendState {
  int plasma_glow_calls = 0;
  float last_glow = 0.0f;
  float last_r = 0.0f, last_g = 0.0f, last_b = 0.0f;
  int age_calls = 0;
  float last_age = 0.0f;
  int zbias_calls = 0;
  float last_zbias = 0.0f;
  int flat_color_calls = 0;
  ddgr_color last_flat_color = 0;
  int texture_type_calls = 0;
  texture_type last_texture_type = TT_FLAT;
};
static RendState g_rend;

void rend_SetAlphaType(int8_t) {}
void rend_SetAlphaValue(uint8_t) {}
void rend_SetLighting(light_state) {}
void rend_SetColorModel(color_model) {}
void rend_SetOverlayType(uint8_t) {}
void rend_SetTextureType(texture_type tt) {
  g_rend.texture_type_calls++;
  g_rend.last_texture_type = tt;
}
void rend_SetZBufferWriteMask(int) {}
void rend_SetFlatColor(ddgr_color color) {
  g_rend.flat_color_calls++;
  g_rend.last_flat_color = color;
}
void rend_SetZBias(float z_bias) {
  g_rend.zbias_calls++;
  g_rend.last_zbias = z_bias;
}
void rend_SetPlasmaGlow(float glow, float r, float g, float b) {
  g_rend.plasma_glow_calls++;
  g_rend.last_glow = glow;
  g_rend.last_r = r;
  g_rend.last_g = g;
  g_rend.last_b = b;
}
void rend_SetEffectAge(float age) {
  g_rend.age_calls++;
  g_rend.last_age = age;
}

// 3D stubs: rotate copies the input vector through, draw captures polys.
uint8_t g3_RotatePoint(g3Point *dest, vector *src) {
  dest->p3_vec = *src;
  return 0;
}
void g3_GetViewPosition(vector *vp) {
  vp->x() = 0.0f;
  vp->y() = 0.0f;
  vp->z() = -50.0f;
}

// Bitmap stubs: fake 64x64 buffers for procedural texture generation.
static uint16_t g_fake_texels[64 * 64];
static int g_bm_next = 100;
static int g_bm_alloc_calls = 0;
int bm_AllocBitmap(int, int, int) {
  g_bm_alloc_calls++;
  memset(g_fake_texels, 0, sizeof(g_fake_texels));
  return g_bm_next;
}
uint16_t *bm_data(int, int) { return g_fake_texels; }
void bm_FreeBitmap(int) {}
int bm_SetBitmapIfTransparent(int) { return 0; }

struct PolyRec {
  int nv = 0;
  std::vector<vector> corners;
  std::vector<float> us, vs;
};
static std::vector<PolyRec> g_polys;
int g3_DrawPoly(int nv, g3Point **pointlist, int, int, g3Codes *) {
  PolyRec rec;
  rec.nv = nv;
  for (int i = 0; i < nv; i++) {
    rec.corners.push_back(pointlist[i]->p3_vec);
    rec.us.push_back(pointlist[i]->p3_u);
    rec.vs.push_back(pointlist[i]->p3_v);
  }
  g_polys.push_back(rec);
  return 1;
}

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
class WeaponImpactTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_polys.clear();
    g_rend = RendState{};
  }

  static vector MakeVec(float x, float y, float z) {
    vector v;
    v.x() = x;
    v.y() = y;
    v.z() = z;
    return v;
  }
};

// Mesh generation returns center + 6 ring vertices
TEST_F(WeaponImpactTest, MeshGeneratesSevenVertices) {
  vector center = MakeVec(0, 0, 0);
  vector normal = MakeVec(0, 1, 0);
  vector right = MakeVec(1, 0, 0);
  vector up = MakeVec(0, 0, 1);
  vector verts[26];
  g3UVL uvs[26];

  int n = GeneratePlasmaImpactMesh(center, normal, right, up, 2.0f, 0.0f, verts, uvs);
  EXPECT_EQ(n, 25); // center + 24 ring segments = smooth circle
}

// Center vertex sits at the impact point (pushed slightly along -normal)
TEST_F(WeaponImpactTest, CenterVertexAtImpactPoint) {
  vector center = MakeVec(10, 20, 30);
  vector normal = MakeVec(0, 1, 0);
  vector right = MakeVec(1, 0, 0);
  vector up = MakeVec(0, 0, 1);
  vector verts[26];
  g3UVL uvs[26];

  GeneratePlasmaImpactMesh(center, normal, right, up, 2.0f, 0.0f, verts, uvs);

  EXPECT_NEAR(verts[0].x(), 10.0f, 0.001f);
  EXPECT_NEAR(verts[0].z(), 30.0f, 0.001f);
  // Pushed 0.05 along -normal (y)
  EXPECT_NEAR(verts[0].y(), 20.0f - 0.05f, 0.001f);
  // Center UV is (0.5, 0.5)
  EXPECT_NEAR(uvs[0].u, 0.5f, 0.001f);
  EXPECT_NEAR(uvs[0].v, 0.5f, 0.001f);
}

// Ring vertices lie at approximately `size` distance from center
TEST_F(WeaponImpactTest, RingVerticesAtMeshRadius) {
  vector center = MakeVec(0, 0, 0);
  vector normal = MakeVec(0, 1, 0);
  vector right = MakeVec(1, 0, 0);
  vector up = MakeVec(0, 0, 1);
  vector verts[26];
  g3UVL uvs[26];

  GeneratePlasmaImpactMesh(center, normal, right, up, 2.0f, 0.0f, verts, uvs);

  for (int i = 1; i < 25; i++) {
    vector d = verts[i] - center;
    float dist = vm_GetMagnitude(&d);
    // Radius is size * (1 +/- 0.05 pulse)
    EXPECT_GT(dist, 2.0f * 0.9f);
    EXPECT_LT(dist, 2.0f * 1.1f);
  }
}

// Ring UVs lie on the unit circle around (0.5, 0.5)
TEST_F(WeaponImpactTest, RingUvsOnUnitCircle) {
  vector center = MakeVec(0, 0, 0);
  vector normal = MakeVec(0, 1, 0);
  vector right = MakeVec(1, 0, 0);
  vector up = MakeVec(0, 0, 1);
  vector verts[26];
  g3UVL uvs[26];

  GeneratePlasmaImpactMesh(center, normal, right, up, 2.0f, 0.0f, verts, uvs);

  for (int i = 1; i < 25; i++) {
    float dx = uvs[i].u - 0.5f;
    float dy = uvs[i].v - 0.5f;
    float r = std::sqrt(dx * dx + dy * dy);
    EXPECT_NEAR(r, 0.5f, 0.001f);
  }
}

// DrawPlasmaImpact3D draws one fan and sets/restores glow state
TEST_F(WeaponImpactTest, DrawImpactSetsGlowAndDrawsFan) {
  // Build a unit quad in the XZ plane (normal +Y)
  g3Point quad[4];
  vector corners[4] = {MakeVec(-1, 0, 1), MakeVec(1, 0, 1), MakeVec(1, 0, -1), MakeVec(-1, 0, -1)};
  for (int i = 0; i < 4; i++)
    quad[i].p3_vec = corners[i];

  bool used = DrawPlasmaImpact3D(0, 0, quad, 2.0f, 0.25f);

  EXPECT_TRUE(used);
  ASSERT_EQ(g_polys.size(), 1u);
  EXPECT_EQ(g_polys[0].nv, 25); // center + 24 ring fan (smooth circle)
  // Glow was enabled then disabled
  EXPECT_EQ(g_rend.plasma_glow_calls, 2);
  EXPECT_NEAR(g_rend.last_glow, 0.0f, 0.001f); // restored to off
  EXPECT_EQ(g_rend.age_calls, 2);
  EXPECT_NEAR(g_rend.last_age, 0.0f, 0.001f); // restored to zero
  // Textured path keeps real UVs for the glow falloff (not TT_FLAT)
  EXPECT_EQ(g_rend.last_texture_type, TT_LINEAR);
  // Flat color tints the textured sample
  EXPECT_EQ(g_rend.flat_color_calls, 1);
  EXPECT_NE(g_rend.last_flat_color, 0u); // not black
}

// DrawPlasmaTrailSegment draws a two-quad volumetric cross
TEST_F(WeaponImpactTest, DrawTrailDrawsVolumetricCross) {
  vector pos = MakeVec(0, 0, 0);
  vector end = MakeVec(0, 0, 10);
  uint16_t green = GR_RGB16(0, 255, 0);

  DrawPlasmaTrailSegment(pos, end, 2.0f, 0.0f, green, 0.7f, 4242);

  ASSERT_EQ(g_polys.size(), 2u);
  for (const auto &p : g_polys)
    EXPECT_EQ(p.nv, 4);
  // Glow enabled with the plasma color, then disabled
  EXPECT_EQ(g_rend.plasma_glow_calls, 2);
  EXPECT_NEAR(g_rend.last_glow, 0.0f, 0.001f);
  // Textured path with the weapon's bitmap (not TT_FLAT)
  EXPECT_EQ(g_rend.last_texture_type, TT_LINEAR);
  // Flat color must match the passed plasma color
  EXPECT_EQ(g_rend.flat_color_calls, 1);
  EXPECT_EQ(g_rend.last_flat_color, GR_16_TO_COLOR(green));
  // UVs span the full 0..1 range so the texture shows
  for (const auto &p : g_polys) {
    bool has_zero = false, has_one = false;
    for (float u : p.us) {
      if (u < 0.01f)
        has_zero = true;
      if (u > 0.99f)
        has_one = true;
    }
    EXPECT_TRUE(has_zero && has_one);
  }
}

// Degenerate quad (zero area) falls back to 2D sprite path
TEST_F(WeaponImpactTest, DegenerateQuadFallsBack) {
  g3Point quad[4];
  for (int i = 0; i < 4; i++)
    quad[i].p3_vec = MakeVec(5, 5, 5);

  bool used = DrawPlasmaImpact3D(0, 0, quad, 2.0f, 0.0f);

  EXPECT_FALSE(used);
  EXPECT_TRUE(g_polys.empty());
}

// DrawPlasmaBlob3D draws an 8-triangle octahedron with the textured path
TEST_F(WeaponImpactTest, DrawBlobDrawsOctahedron) {
  vector pos = MakeVec(0, 0, 0);
  uint16_t green = GR_RGB16(0, 255, 0);

  DrawPlasmaBlob3D(pos, 4.0f, 0.0f, green, 0.7f, 4242);

  ASSERT_EQ(g_polys.size(), 8u);
  for (const auto &p : g_polys)
    EXPECT_EQ(p.nv, 3);
  EXPECT_EQ(g_rend.last_texture_type, TT_LINEAR);
  EXPECT_EQ(g_rend.flat_color_calls, 1);
  EXPECT_EQ(g_rend.last_flat_color, GR_16_TO_COLOR(green));
}

// DrawPlasmaBall3D draws a UV sphere (10 segs x 7 rings = 60 quads + 20 pole tris)
TEST_F(WeaponImpactTest, DrawBallDrawsSphere) {
  vector pos = MakeVec(0, 0, 0);
  uint16_t green = GR_RGB16(0, 255, 0);

  DrawPlasmaBall3D(pos, 4.0f, 0.0f, green, 1.0f, 4242);

  // 7 rings: top pole row (10 tris) + 5 middle rows (50 quads) + bottom pole row (10 tris)
  ASSERT_EQ(g_polys.size(), 70u);
  int tri_count = 0, quad_count = 0;
  for (const auto &p : g_polys) {
    if (p.nv == 3)
      tri_count++;
    else if (p.nv == 4)
      quad_count++;
  }
  EXPECT_EQ(tri_count, 20);
  EXPECT_EQ(quad_count, 50);
  EXPECT_EQ(g_rend.last_texture_type, TT_LINEAR);
}

// DrawPlasmaShockball3D draws a sphere plus a wall-aligned fan
TEST_F(WeaponImpactTest, DrawShockballDrawsBallPlusFan) {
  vector pos = MakeVec(0, 0, 0);
  vector normal = MakeVec(0, 1, 0);
  uint16_t green = GR_RGB16(0, 255, 0);

  DrawPlasmaShockball3D(pos, normal, 4.0f, 0.2f, green, 0.8f, 4242);

  // 70 sphere polys + 1 wall fan (25-vertex circle fan)
  ASSERT_EQ(g_polys.size(), 71u);
  EXPECT_EQ(g_polys.back().nv, 25);
}

// Shockball with zero normal draws ball only (open-air explosion)
TEST_F(WeaponImpactTest, DrawShockballNoFanWithoutNormal) {
  vector pos = MakeVec(0, 0, 0);
  vector zero = MakeVec(0, 0, 0);
  uint16_t green = GR_RGB16(0, 255, 0);

  DrawPlasmaShockball3D(pos, zero, 4.0f, 0.2f, green, 0.8f, 4242);

  ASSERT_EQ(g_polys.size(), 70u);
}

// Procedural ball texture: white-hot center, green rim, fully opaque
TEST_F(WeaponImpactTest, BallTextureIsWhiteCoreGreenRim) {
  FreePlasmaImpactTextures();
  int bm = GetPlasmaBallTexture();
  EXPECT_GE(bm, 0);

  // Center texel (32,32) should be near-white
  uint16_t center = g_fake_texels[32 * 64 + 32];
  EXPECT_TRUE(center & 0x8000); // opaque
  int cr = ((center >> 10) & 0x1F) << 3;
  int cg = ((center >> 5) & 0x1F) << 3;
  EXPECT_GT(cr, 200);
  EXPECT_GT(cg, 200);

  // Edge texel should be green-dominant (r < g)
  uint16_t edge = g_fake_texels[32 * 64 + 63];
  int er = ((edge >> 10) & 0x1F) << 3;
  int eg = ((edge >> 5) & 0x1F) << 3;
  EXPECT_LT(er, eg);

  // Second call returns the cached handle without reallocating
  int allocs_before = g_bm_alloc_calls;
  EXPECT_EQ(GetPlasmaBallTexture(), bm);
  EXPECT_EQ(g_bm_alloc_calls, allocs_before);
  FreePlasmaImpactTextures();
}

// Procedural explosion texture: circular falloff with transparent rim
TEST_F(WeaponImpactTest, ExplosionTextureHasTransparentRim) {
  FreePlasmaImpactTextures();
  int bm = GetPlasmaExplosionTexture();
  EXPECT_GE(bm, 0);

  // Center opaque white-hot
  uint16_t center = g_fake_texels[32 * 64 + 32];
  EXPECT_TRUE(center & 0x8000);

  // Corner (outside the disc) transparent
  uint16_t corner = g_fake_texels[0];
  EXPECT_FALSE(corner & 0x8000);
  FreePlasmaImpactTextures();
}
