/**
 * @file newpyrogauges_real_tests.cpp
 * @brief Tests for Descent3/NewPyroGauges.cpp — NewPyro init + GetCenterPoint.
 *
 * @details
 * NewPyroGauges.cpp is 90% @@-disabled gauges; live code is:
 *   - NewPyroInitGauges(): FindTextureName("GaugeShieldRing") -> GetTextureBitmap
 *   - GetCenterPoint(): g3_RotatePoint + g3_ProjectPoint
 * Other gauge handlers are empty no-ops.
 * This TU #includes the real .cpp so its file-static NewPyroGaugeData
 * is visible for assertions.
 *
 * This harness validates the behavior of `Descent3/NewPyroGauges.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/NewPyroGauges.cpp`
 * @par Harness
 * `newpyrogauges_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/NewPyroGauges.cpp
 */

#include <gtest/gtest.h>

#include <cstring>
#include <string>

// Headers that NewPyroGauges.cpp includes – included first so the
// included cpp sees their declarations.
#include "NewPyroGauges.h"
#include "3d.h"
#include "pserror.h"
#include "renderer.h"
#include "gametexture.h"
#include "vecmat.h"

// ---------------------------------------------------------------------------
// Capture stubs for the two gametexture calls NewPyroInitGauges uses
// ---------------------------------------------------------------------------
static int g_find_calls = 0;
static std::string g_last_find_name;
static int g_find_ret = -1; // what FindTextureName returns

static int g_get_calls = 0;
static int g_last_get_handle = -1;
static int g_last_get_frame = -1;
static int g_get_ret = 5;

// Must match Descent3/gametexture.h exactly: 3 params with default force
int FindTextureName(const char *name) {
  ++g_find_calls;
  g_last_find_name = name ? name : "";
  return g_find_ret;
}
int GetTextureBitmap(int handle, int framenum, bool force) {
  ++g_get_calls;
  g_last_get_handle = handle;
  g_last_get_frame = framenum;
  (void)force;
  return g_get_ret;
}

// ---------------------------------------------------------------------------
// Stubs for g3_* used by GetCenterPoint
// ---------------------------------------------------------------------------
static vector g_last_rotate_src{};
static g3Point g_rotate_dest{};
static bool g_rotate_called = false;
static bool g_project_called = false;
static float g_project_sx = 0, g_project_sy = 0;

uint8_t g3_RotatePoint(g3Point *dest, vector *src) {
  g_rotate_called = true;
  if (src) g_last_rotate_src = *src;
  if (dest && src) dest->p3_vec = *src;
  if (dest) g_rotate_dest = *dest;
  return 0;
}
void g3_ProjectPoint(g3Point *pt) {
  g_project_called = true;
  if (pt) {
    pt->p3_sx = g_project_sx;
    pt->p3_sy = g_project_sy;
    pt->p3_flags |= PF_PROJECTED;
  }
}

// Minimal renderer stubs – not used by live code but satisfy headers
// (NewPyroGauges.cpp includes renderer.h)
void rend_SetLighting(int) {}
void rend_SetTextureType(int) {}
void rend_SetAlphaType(int) {}
void rend_SetAlphaValue(float) {}
void rend_SetZBufferState(int) {}
void rend_DrawPolygon(int, void**, int) {}

// ---------------------------------------------------------------------------
// Include the real implementation – its static NewPyroGaugeData is now
// visible in this TU for direct inspection.
// ---------------------------------------------------------------------------
#include "../../Descent3/NewPyroGauges.cpp"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static void ResetStubs() {
  g_find_calls = 0;
  g_last_find_name.clear();
  g_get_calls = 0;
  g_last_get_handle = -1;
  g_last_get_frame = -1;
  g_rotate_called = false;
  g_project_called = false;
  g_last_rotate_src = vector{};
  g_project_sx = 123.0f;
  g_project_sy = 456.0f;
}

// ---------------------------------------------------------------------------
// NewPyroInitGauges
// ---------------------------------------------------------------------------
/**
 * @test NewPyroInitGaugesTest.TextureNotFoundSetsMinusOne
 * @brief Verifies texture Not Found Sets Minus One.
 *
 * @details
 * Exercises the NewPyroInitGaugesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(NewPyroInitGaugesTest, TextureNotFoundSetsMinusOne) {
  ResetStubs();
  g_find_ret = -1;
  g_get_ret = 99;
  // Ensure a clean start – reset static to a sentinel via direct access
  NewPyroGaugeData.ringbmp = 999;
  NewPyroInitGauges();
  EXPECT_EQ(g_find_calls, 1);
  EXPECT_EQ(g_last_find_name, "GaugeShieldRing");
  EXPECT_EQ(g_get_calls, 0) << "GetTextureBitmap must not be called when Find returns -1";
  EXPECT_EQ(NewPyroGaugeData.ringbmp, -1);
}

/**
 * @test NewPyroInitGaugesTest.TextureFoundSetsRingBmp
 * @brief Verifies texture Found Sets Ring Bmp.
 *
 * @details
 * Exercises the NewPyroInitGaugesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(NewPyroInitGaugesTest, TextureFoundSetsRingBmp) {
  ResetStubs();
  g_find_ret = 7;
  g_get_ret = 42;
  NewPyroInitGauges();
  EXPECT_EQ(g_find_calls, 1);
  EXPECT_EQ(g_get_calls, 1);
  EXPECT_EQ(g_last_get_handle, 7);
  EXPECT_EQ(g_last_get_frame, 0);
  EXPECT_EQ(NewPyroGaugeData.ringbmp, 42);
}

/**
 * @test NewPyroInitGaugesTest.TextureFoundWithZeroHandle
 * @brief Verifies texture Found With Zero Handle.
 *
 * @details
 * Exercises the NewPyroInitGaugesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(NewPyroInitGaugesTest, TextureFoundWithZeroHandle) {
  ResetStubs();
  g_find_ret = 0;
  g_get_ret = 3;
  NewPyroInitGauges();
  EXPECT_EQ(NewPyroGaugeData.ringbmp, 3);
  EXPECT_EQ(g_last_get_handle, 0);
}

/**
 * @test NewPyroInitGaugesTest.RepeatedInitOverwrites
 * @brief Verifies repeated Init Overwrites.
 *
 * @details
 * Exercises the NewPyroInitGaugesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(NewPyroInitGaugesTest, RepeatedInitOverwrites) {
  ResetStubs();
  g_find_ret = 1;
  g_get_ret = 10;
  NewPyroInitGauges();
  EXPECT_EQ(NewPyroGaugeData.ringbmp, 10);
  g_get_ret = 20;
  NewPyroInitGauges();
  EXPECT_EQ(NewPyroGaugeData.ringbmp, 20);
  EXPECT_EQ(g_find_calls, 2);
  EXPECT_EQ(g_get_calls, 2);
}

/**
 * @test NewPyroInitGaugesTest.ZfactorDefault
 * @brief Verifies zfactor Default.
 *
 * @details
 * Exercises the NewPyroInitGaugesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(NewPyroInitGaugesTest, ZfactorDefault) {
  // Zfactor is an exported global in NewPyroGauges.cpp, default 2.0f
  EXPECT_FLOAT_EQ(Zfactor, 2.0f);
}

// ---------------------------------------------------------------------------
// GetCenterPoint
// ---------------------------------------------------------------------------
/**
 * @test GetCenterPointTest.ForwardsToG3AndReturnsProjected
 * @brief Verifies forwards To G3And Returns Projected.
 *
 * @details
 * Exercises the GetCenterPointTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(GetCenterPointTest, ForwardsToG3AndReturnsProjected) {
  ResetStubs();
  g_project_sx = 100.0f;
  g_project_sy = 200.0f;
  vector wpos{{1.0f, 2.0f, 3.0f}};
  int x = -1, y = -1;
  GetCenterPoint(&x, &y, &wpos);
  EXPECT_TRUE(g_rotate_called);
  EXPECT_TRUE(g_project_called);
  EXPECT_FLOAT_EQ(g_last_rotate_src.x(), 1.0f);
  EXPECT_FLOAT_EQ(g_last_rotate_src.y(), 2.0f);
  EXPECT_FLOAT_EQ(g_last_rotate_src.z(), 3.0f);
  EXPECT_EQ(x, 100);
  EXPECT_EQ(y, 200);
}

/**
 * @test GetCenterPointTest.TruncatesFloat
 * @brief Verifies truncates Float.
 *
 * @details
 * Exercises the GetCenterPointTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(GetCenterPointTest, TruncatesFloat) {
  ResetStubs();
  g_project_sx = 99.9f;
  g_project_sy = -10.7f;
  vector wpos{{0,0,0}};
  int x=-1,y=-1;
  GetCenterPoint(&x,&y,&wpos);
  EXPECT_EQ(x, 99);  // (int) cast truncates toward zero
  EXPECT_EQ(y, -10);
}

/**
 * @test GetCenterPointTest.PassesWposToRotate
 * @brief Verifies passes Wpos To Rotate.
 *
 * @details
 * Exercises the GetCenterPointTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(GetCenterPointTest, PassesWposToRotate) {
  ResetStubs();
  vector wpos{{5,6,7}};
  int x,y;
  GetCenterPoint(&x,&y,&wpos);
  EXPECT_FLOAT_EQ(g_last_rotate_src.x(), 5);
  EXPECT_FLOAT_EQ(g_last_rotate_src.y(), 6);
  EXPECT_FLOAT_EQ(g_last_rotate_src.z(), 7);
}

// ---------------------------------------------------------------------------
// Empty gauge handlers must not crash when called with nullptr or dummy
// ---------------------------------------------------------------------------
/**
 * @test NewPyroNoopGaugesTest.AllHandlersAreNoOps
 * @brief Verifies all Handlers Are No Ops.
 *
 * @details
 * Exercises the NewPyroNoopGaugesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/NewPyroGauges.cpp
 * @ingroup descent3_tests
 */
TEST(NewPyroNoopGaugesTest, AllHandlersAreNoOps) {
  // These are @@-disabled in the source; they should be safe to call
  // with null and not touch g_find/g_get.
  ResetStubs();
  EXPECT_NO_THROW(NewPyroShieldGauge(nullptr, nullptr));
  EXPECT_NO_THROW(NewPyroShipGauge(nullptr, nullptr));
  EXPECT_NO_THROW(NewPyroEnergyDigitalGauge(nullptr, nullptr));
  EXPECT_NO_THROW(NewPyroEnergyAnalogGauge(nullptr, nullptr));
  EXPECT_NO_THROW(NewPyroPrimaryWeaponGauge(nullptr, nullptr));
  EXPECT_NO_THROW(NewPyroSecondaryWeaponGauge(nullptr, nullptr));
  EXPECT_EQ(g_find_calls, 0);
  EXPECT_EQ(g_get_calls, 0);
}
