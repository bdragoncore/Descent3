/**
 * @file lightmap_info_real_tests.cpp
 * @brief Tests for lightmap_info.cpp (460 lines).
 *
 * @details
 * Covers InitLightmapInfo/AllocLightmapInfo/FreeLightmapInfo lifecycle, the
 * LIFO free-list reuse, lmi_w/lmi_h accessors, CloseLightmapInfos teardown,
 * and the two filter passes ShadeLightmapInfoEdges / BlurLightmapInfos.
 *
 * This harness validates the behavior of `Descent3/lightmap_info.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/lightmap_info.cpp`
 * @par Harness
 * `lightmap_info_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/lightmap_info.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <vector>

#include "lightmap_info.h"
#include "lightmap.h"
#include "grdefs.h"
#include "pserror.h"
#include "renderer.h"
#include "mem.h"
#include "dedicated_server.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
bool Dedicated_server = false;
int Num_lightmap_infos_read = 0;

// ---------------------------------------------------------------------------
// Lightmap pool stubs
// ---------------------------------------------------------------------------
static constexpr int kMaxLMs = 8;
static uint16_t g_lm_pool[kMaxLMs][64 * 64];
static int g_lm_next;
static std::vector<int> g_lm_freed;
static std::vector<std::pair<int, int>> g_rend_freed; // (handle, map_type)

int lm_AllocLightmap(int w, int h) {
  (void)w;
  (void)h;
  if (g_lm_next >= kMaxLMs)
    return BAD_LM_INDEX;
  return g_lm_next++;
}
void lm_FreeLightmap(int handle) { g_lm_freed.push_back(handle); }
uint16_t *lm_data(int handle) { return g_lm_pool[handle]; }

void rend_FreePreUploadedTexture(int n, int map_type) { g_rend_freed.push_back({n, map_type}); }

/**
 * @brief GTest fixture for LightmapInfoTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class LightmapInfoTest : public ::testing::Test {
protected:
  void SetUp() override {
    Dedicated_server = false;
    g_lm_next = 0;
    g_lm_freed.clear();
    g_rend_freed.clear();
    memset(g_lm_pool, 0xEE, sizeof(g_lm_pool)); // sentinel garbage
    InitLightmapInfo(0);
  }
  void TearDown() override { CloseLightmapInfos(); }

  // Allocates an lmi of w*h backed by real lm pool storage; returns handle.
  int Alloc(int w, int h, int type = 0, bool alloc_lm = true) {
    int n = AllocLightmapInfo(w, h, type, alloc_lm);
    EXPECT_GE(n, 0);
    return n;
  }
};

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

/**
 * @test LightmapInfoTest.InitBuildsIdentityFreeList
 * @brief Verifies init Builds Identity Free List.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, InitBuildsIdentityFreeList) {
  ASSERT_NE(LightmapInfo, nullptr);
  EXPECT_EQ(Num_of_lightmap_info, 0);
  // Free list is identity-ordered: first allocs come back in slot order.
  EXPECT_EQ(Alloc(2, 2), 0);
  EXPECT_EQ(Alloc(2, 2), 1);
  CloseLightmapInfos();
  InitLightmapInfo(0); // reset for later tests
}

/**
 * @test LightmapInfoTest.AllocFillsAllFields
 * @brief Verifies alloc Fills All Fields.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, AllocFillsAllFields) {
  int n = Alloc(4, 2, /*type*/ 3);
  EXPECT_EQ(n, 0);
  EXPECT_EQ(Num_of_lightmap_info, 1);
  EXPECT_EQ(LightmapInfo[n].used, 1);
  EXPECT_EQ(LightmapInfo[n].type, 3);
  EXPECT_EQ(LightmapInfo[n].dynamic, BAD_LM_INDEX);
  EXPECT_EQ(LightmapInfo[n].spec_map, -1);
  EXPECT_EQ(LightmapInfo[n].width, 4);
  EXPECT_EQ(LightmapInfo[n].height, 2);
  EXPECT_EQ(LightmapInfo[n].x1, 0);
  EXPECT_EQ(LightmapInfo[n].y1, 0);
  EXPECT_NE(LightmapInfo[n].lm_handle, BAD_LM_INDEX);

  // Freshly allocated lightmap data is fully transparent
  uint16_t *data = lm_data(LightmapInfo[n].lm_handle);
  for (int i = 0; i < 8; i++)
    EXPECT_EQ(data[i], NEW_TRANSPARENT_COLOR);
}

/**
 * @test LightmapInfoTest.AllocWithoutBackingLightmapLeavesHandleZero
 * @brief Verifies alloc Without Backing Lightmap Leaves Handle Zero.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, AllocWithoutBackingLightmapLeavesHandleZero) {
  // Quirk: alloc_lightmap=false skips lm_AllocLightmap, but the struct was
  // memset to zero - lm_handle is 0, which looks like a valid lightmap.
  int n = Alloc(2, 2, 0, /*alloc_lm*/ false);
  EXPECT_EQ(LightmapInfo[n].lm_handle, 0);
  EXPECT_EQ(g_lm_next, 0); // no lm allocated from pool
}

/**
 * @test LightmapInfoTest.AllocsPopSequentialSlots
 * @brief Verifies allocs Pop Sequential Slots.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, AllocsPopSequentialSlots) {
  EXPECT_EQ(Alloc(2, 2), 0);
  EXPECT_EQ(Alloc(2, 2), 1);
  EXPECT_EQ(Alloc(2, 2), 2);
  EXPECT_EQ(Num_of_lightmap_info, 3);
}

/**
 * @test LightmapInfoTest.FreedSlotIsReusedLifo
 * @brief Verifies freed Slot Is Reused Lifo.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, FreedSlotIsReusedLifo) {
  int a = Alloc(2, 2);
  int b = Alloc(2, 2);
  int c = Alloc(2, 2);
  ASSERT_EQ(b, 1);

  FreeLightmapInfo(b);
  EXPECT_EQ(Num_of_lightmap_info, 2);

  int d = Alloc(2, 2);
  EXPECT_EQ(d, 1); // freed slot comes back around
  EXPECT_EQ(Num_of_lightmap_info, 3);
  (void)a;
  (void)c;
}

/**
 * @test LightmapInfoTest.FreeReleasesRendererTextureAndLightmap
 * @brief Verifies free Releases Renderer Texture And Lightmap.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, FreeReleasesRendererTextureAndLightmap) {
  int n = Alloc(2, 2);
  int lm_handle = LightmapInfo[n].lm_handle;

  FreeLightmapInfo(n);
  EXPECT_EQ(LightmapInfo[n].used, 0);
  ASSERT_EQ(g_rend_freed.size(), 1u);
  EXPECT_EQ(g_rend_freed[0].first, lm_handle);
  ASSERT_EQ(g_lm_freed.size(), 1u);
  EXPECT_EQ(g_lm_freed[0], lm_handle);
}

/**
 * @test LightmapInfoTest.RefcountDecrementBeforeFullRelease
 * @brief Verifies refcount Decrement Before Full Release.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, RefcountDecrementBeforeFullRelease) {
  int n = Alloc(2, 2);
  LightmapInfo[n].used = 3; // pretend multiple refs

  FreeLightmapInfo(n);
  EXPECT_EQ(LightmapInfo[n].used, 2);
  EXPECT_TRUE(g_lm_freed.empty()); // still referenced

  FreeLightmapInfo(n);
  FreeLightmapInfo(n);
  EXPECT_EQ(LightmapInfo[n].used, 0);
  ASSERT_EQ(g_lm_freed.size(), 1u);
}

/**
 * @test LightmapInfoTest.DoubleFreeIsNoop
 * @brief Verifies double Free Is Noop.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, DoubleFreeIsNoop) {
  int n = Alloc(2, 2);
  FreeLightmapInfo(n);
  FreeLightmapInfo(n);
  EXPECT_EQ(g_lm_freed.size(), 1u);
  // Slot went back on the free list once only
  EXPECT_EQ(Num_of_lightmap_info, 0);
}

/**
 * @test LightmapInfoTest.FreeOfNeverUsedHandleIsNoop
 * @brief Verifies free Of Never Used Handle Is Noop.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, FreeOfNeverUsedHandleIsNoop) {
  FreeLightmapInfo(50); // never allocated
  EXPECT_TRUE(g_lm_freed.empty());
}

/**
 * @test LightmapInfoTest.FreeAfterShutdownDoesNotCrash
 * @brief Verifies free After Shutdown Does Not Crash.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, FreeAfterShutdownDoesNotCrash) {
  Alloc(2, 2);
  CloseLightmapInfos();
  EXPECT_EQ(LightmapInfo, nullptr);
  FreeLightmapInfo(0); // early-return path for NULL array
}

/**
 * @test LightmapInfoTest.DedicatedServerSkipsInit
 * @brief Verifies dedicated Server Skips Init.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, DedicatedServerSkipsInit) {
  CloseLightmapInfos();
  Dedicated_server = true;
  InitLightmapInfo(0);
  Dedicated_server = false;
  EXPECT_EQ(LightmapInfo, nullptr); // nothing allocated
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

/**
 * @test LightmapInfoTest.WidthHeightAccessors
 * @brief Verifies width Height Accessors.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, WidthHeightAccessors) {
  int n = Alloc(4, 7);
  EXPECT_EQ(lmi_w(n), 4);
  EXPECT_EQ(lmi_h(n), 7);
}

/**
 * @test LightmapInfoTest.AccessorsReturnZeroForUnused
 * @brief Verifies accessors Return Zero For Unused.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, AccessorsReturnZeroForUnused) {
  int n = Alloc(4, 7);
  LightmapInfo[n].used = 0;
  EXPECT_EQ(lmi_w(n), 0);
  EXPECT_EQ(lmi_h(n), 0);
}

// ---------------------------------------------------------------------------
// Filter passes
// ---------------------------------------------------------------------------

static const int RED16 = GR_RGB16(248, 0, 0) | OPAQUE_FLAG;   // 5-bit max red
static const int BLUE16 = GR_RGB16(0, 0, 248) | OPAQUE_FLAG;  // 5-bit max blue

/**
 * @test LightmapInfoTest.ShadeEdgesOpaquefiesTransparentPixelFromNeighbors
 * @brief Verifies shade Edges Opaquefies Transparent Pixel From Neighbors.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, ShadeEdgesOpaquefiesTransparentPixelFromNeighbors) {
  int n = Alloc(3, 3, /*type*/ 5);
  uint16_t *d = lm_data(LightmapInfo[n].lm_handle);
  for (int i = 0; i < 9; i++)
    d[i] = (uint16_t)RED16;
  d[4] = NEW_TRANSPARENT_COLOR; // transparent hole in the middle

  ShadeLightmapInfoEdges(5);

  // Center averages its 8 opaque red neighbors -> becomes opaque red.
  // Quirk: GR_16_TO_COLOR expands 5-bit channels with <<3 (31 -> 248), so the
  // average of eight 248s is exactly 248 again.
  EXPECT_EQ(d[4], OPAQUE_FLAG | GR_RGB16(248, 0, 0));
  EXPECT_EQ(d[4] & OPAQUE_FLAG, OPAQUE_FLAG);
}

/**
 * @test LightmapInfoTest.ShadeEdgesIgnoresTransparentNeighborsAndBounds
 * @brief Verifies shade Edges Ignores Transparent Neighbors And Bounds.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, ShadeEdgesIgnoresTransparentNeighborsAndBounds) {
  int n = Alloc(3, 3, /*type*/ 5);
  uint16_t *d = lm_data(LightmapInfo[n].lm_handle);
  for (int i = 0; i < 9; i++)
    d[i] = (uint16_t)RED16;
  d[4] = NEW_TRANSPARENT_COLOR;
  d[0] = NEW_TRANSPARENT_COLOR; // corner transparent too

  ShadeLightmapInfoEdges(5);

  // Corner (0,0): only opaque neighbor is (1,0) -> still gets filled red.
  EXPECT_EQ(d[0], OPAQUE_FLAG | GR_RGB16(248, 0, 0));
}

/**
 * @test LightmapInfoTest.ShadeEdgesLeavesFullyIsolatedTransparentAlone
 * @brief Verifies shade Edges Leaves Fully Isolated Transparent Alone.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, ShadeEdgesLeavesFullyIsolatedTransparentAlone) {
  int n = Alloc(2, 2, /*type*/ 5);
  uint16_t *d = lm_data(LightmapInfo[n].lm_handle);
  d[0] = NEW_TRANSPARENT_COLOR; // everything else left as raw garbage that
  for (int i = 1; i < 4; i++)
    d[i] = 0x0001;              // lacks OPAQUE_FLAG -> counts as transparent

  ShadeLightmapInfoEdges(5);

  EXPECT_EQ(d[0], NEW_TRANSPARENT_COLOR); // num==0 -> untouched
}

/**
 * @test LightmapInfoTest.ShadeEdgesMixesColorsAcrossBoundary
 * @brief Verifies shade Edges Mixes Colors Across Boundary.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, ShadeEdgesMixesColorsAcrossBoundary) {
  int n = Alloc(3, 1, /*type*/ 5);
  uint16_t *d = lm_data(LightmapInfo[n].lm_handle);
  d[0] = (uint16_t)RED16;
  d[1] = NEW_TRANSPARENT_COLOR;
  d[2] = (uint16_t)BLUE16;

  ShadeLightmapInfoEdges(5);

  // Center sees red(left) + blue(right): avg r=124,b=124 in 8-bit, but the
  // result is re-quantized to 5-bit by GR_RGB16 (124>>3=15) and expanded back
  // with <<3 when read -> 15*8=120.
  ddgr_color c = GR_16_TO_COLOR((uint16_t)(d[1] & ~OPAQUE_FLAG));
  EXPECT_EQ(GR_COLOR_RED(c), 120);
  EXPECT_EQ(GR_COLOR_GREEN(c), 0);
  EXPECT_EQ(GR_COLOR_BLUE(c), 120);
  EXPECT_TRUE(d[1] & OPAQUE_FLAG);
}

/**
 * @test LightmapInfoTest.BlurAveragesSelfWithOpaqueNeighbors
 * @brief Verifies blur Averages Self With Opaque Neighbors.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, BlurAveragesSelfWithOpaqueNeighbors) {
  int n = Alloc(3, 1, /*type*/ 5);
  uint16_t *d = lm_data(LightmapInfo[n].lm_handle);
  d[0] = (uint16_t)RED16;
  d[1] = (uint16_t)RED16;
  d[2] = (uint16_t)BLUE16;

  BlurLightmapInfos(5);

  // Pixel 1: self red + left red + right blue -> r=(248*2+0)/3=165,
  // b=248/3=82; both re-quantized to 5-bit then expanded (165>>3<<3=160,
  // 82>>3<<3=80).
  uint16_t p1 = (uint16_t)(d[1] & ~OPAQUE_FLAG);
  ddgr_color c = GR_16_TO_COLOR(p1);
  EXPECT_EQ(GR_COLOR_RED(c), 160);
  EXPECT_EQ(GR_COLOR_BLUE(c), 80);
  EXPECT_EQ(d[0], OPAQUE_FLAG | GR_RGB16(248, 0, 0));
  // Pixel 2: self blue + left red -> avg 124 each, re-quantized (>>3<<3=120)
  EXPECT_EQ(d[2], OPAQUE_FLAG | GR_RGB16(124, 0, 124));
}

/**
 * @test LightmapInfoTest.BlurSkipsTransparentNeighbors
 * @brief Verifies blur Skips Transparent Neighbors.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, BlurSkipsTransparentNeighbors) {
  int n = Alloc(3, 1, /*type*/ 5);
  uint16_t *d = lm_data(LightmapInfo[n].lm_handle);
  d[0] = (uint16_t)RED16;
  d[1] = (uint16_t)RED16;
  d[2] = NEW_TRANSPARENT_COLOR; // not counted in average

  BlurLightmapInfos(5);

  EXPECT_EQ(d[1], OPAQUE_FLAG | GR_RGB16(248, 0, 0));
  EXPECT_EQ(d[2], NEW_TRANSPARENT_COLOR); // transparent pixels untouched
}

/**
 * @test LightmapInfoTest.FiltersOnlyProcessMatchingType
 * @brief Verifies filters Only Process Matching Type.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, FiltersOnlyProcessMatchingType) {
  int a = Alloc(2, 2, /*type*/ 1);
  int b = Alloc(2, 2, /*type*/ 2);
  uint16_t *da = lm_data(LightmapInfo[a].lm_handle);
  uint16_t *db = lm_data(LightmapInfo[b].lm_handle);
  da[0] = NEW_TRANSPARENT_COLOR;
  db[0] = NEW_TRANSPARENT_COLOR;

  ShadeLightmapInfoEdges(1);
  BlurLightmapInfos(1);

  EXPECT_EQ(db[0], NEW_TRANSPARENT_COLOR); // type 2 untouched by both passes
}

/**
 * @test LightmapInfoTest.CloseFreesArraysAndNullsPointers
 * @brief Verifies close Frees Arrays And Nulls Pointers.
 *
 * @details
 * Exercises the LightmapInfoTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/lightmap_info.cpp
 * @ingroup descent3_tests
 */
TEST_F(LightmapInfoTest, CloseFreesArraysAndNullsPointers) {
  Alloc(2, 2);
  CloseLightmapInfos();
  EXPECT_EQ(LightmapInfo, nullptr);
}
