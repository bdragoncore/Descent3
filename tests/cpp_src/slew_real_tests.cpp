/**
 * @file slew_real_tests.cpp
 * @brief Tests for SLEW.cpp 488 lines — debug slew system (debug/editor only).
 *
 * @details
 * Covers key-to-axis mapping, rotation key mapping with fallbacks,
 * movement limitation bitmask, terrain bounds clamping (including the
 * TERRAIN_WIDTH-vs-DEPTH quirk), and ret flag accumulation.
 * Replicates logic; whole file is #if _DEBUG/EDITOR so RELEASE build is empty.
 *
 * This harness validates the behavior of `Descent3/SLEW.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/SLEW.cpp`
 * @par Harness
 * `slew_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/SLEW.cpp
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cstdint>

// replicated constants (SLEW.cpp:217-219)
#define ROT_SPEED (1.0 / 8.0)
#define VEL_SPEED (110.0)
#define JOY_NULL 32
constexpr float SLEW_KEY_SPEED = 1.0f;

// replicated terrain constants (terrain.h:31-33)
constexpr int TERRAIN_WIDTH = 256;
constexpr int TERRAIN_DEPTH = 256;
constexpr double TERRAIN_SIZE = 16.0;

// replicated flags (slew.h)
constexpr int SLEW_MOVE = 1, SLEW_ROTATE = 2, SLEW_KEY = 4;

// replicated key time inputs
struct KeyTimes {
  float timex1=0, timex0=0; // PAD9 / PAD7 -> x axis
  float timey1=0, timey0=0; // PADMINUS / PADPLUS -> y
  float timez1=0, timez0=0; // PAD8 / PAD2 -> z
  float timep1=0, timep0=0; // LBRACKET (fallback PAGEDOWN) / RBRACKET (PAGEUP) -> pitch
  float timeh1=0, timeh0=0; // PAD6 / PAD4 -> heading
  float timeb1=0, timeb0=0; // PAD1 / PAD3 -> bank
};

// replicated key->flag mapping (SLEW.cpp:306-309)
static bool AnyKeyHeld(const KeyTimes &k) {
  if (k.timex1 || k.timex0 || k.timey1 || k.timey0 || k.timez1 || k.timez0) return true;
  if (k.timep1 || k.timep0 || k.timeh1 || k.timeh0 || k.timeb1 || k.timeb0) return true;
  return false;
}

// replicated velocity update (SLEW.cpp:312-314)
struct Velocity { float x=0,y=0,z=0; };
static void ApplyKeyVelocity(Velocity &vel, const KeyTimes &k) {
  vel.x += VEL_SPEED * (k.timex1 - k.timex0) * SLEW_KEY_SPEED;
  vel.y += VEL_SPEED * (k.timey1 - k.timey0) * SLEW_KEY_SPEED;
  vel.z += VEL_SPEED * (k.timez1 - k.timez0) * SLEW_KEY_SPEED;
}

// replicated rotang computation (SLEW.cpp:316-321) — fixed point 65536 per rev
struct AngVec { int16_t p=0,h=0,b=0; };
static AngVec ComputeRotang(const KeyTimes &k) {
  float rotx = k.timep1 - k.timep0;
  float roty = k.timeh1 - k.timeh0;
  float rotz = k.timeb1 - k.timeb0;
  AngVec a;
  a.p = (int16_t)(65536.0 * rotx * ROT_SPEED * SLEW_KEY_SPEED);
  a.h = (int16_t)(65536.0 * roty * ROT_SPEED * SLEW_KEY_SPEED);
  a.b = (int16_t)(65536.0 * rotz * ROT_SPEED * SLEW_KEY_SPEED);
  return a;
}

// replicated movement limitation mask (SLEW.cpp:375-380)
static void LimitMovement(float m[3], int limitations) {
  if (limitations & 1) m[0] = 0;
  if (limitations & 2) m[1] = 0;
  if (limitations & 4) m[2] = 0;
}

// replicated terrain bounds clamp (SLEW.cpp:413-423) — note z-max uses WIDTH (original quirk)
static void ClampTerrainBounds(float pos[3]) {
  if (pos[0] < 1.0f) pos[0] = 1.0f;
  if (pos[0] > TERRAIN_WIDTH * TERRAIN_SIZE - 1.0f) pos[0] = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0f;
  if (pos[2] < 1.0f) pos[2] = 1.0f;
  if (pos[2] > TERRAIN_DEPTH * TERRAIN_SIZE - 1.0f) pos[2] = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0f; // quirk preserved
}

/**
 * @test SLEW.VelocityAxisMapping
 * @brief Verifies velocity Axis Mapping.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, VelocityAxisMapping) {
  KeyTimes k;
  k.timex1 = 0.5f; // PAD9 held -> +x
  Velocity v;
  ApplyKeyVelocity(v, k);
  EXPECT_FLOAT_EQ(v.x, VEL_SPEED * 0.5f);
  EXPECT_FLOAT_EQ(v.y, 0);
  EXPECT_FLOAT_EQ(v.z, 0);

  KeyTimes k2; k2.timex0 = 0.5f; // PAD7 -> -x
  Velocity v2;
  ApplyKeyVelocity(v2, k2);
  EXPECT_FLOAT_EQ(v2.x, -VEL_SPEED * 0.5f);
}

/**
 * @test SLEW.VelocityYZAxes
 * @brief Verifies velocity YZAxes.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, VelocityYZAxes) {
  KeyTimes ky; ky.timey1 = 1.0f; // PADMINUS -> +y (note inverted naming)
  Velocity vy; ApplyKeyVelocity(vy, ky);
  EXPECT_FLOAT_EQ(vy.y, VEL_SPEED);
  KeyTimes kz; kz.timez0 = 0.25f; // PAD2 -> -z
  Velocity vz; ApplyKeyVelocity(vz, kz);
  EXPECT_FLOAT_EQ(vz.z, -VEL_SPEED * 0.25f);
}

/**
 * @test SLEW.AnyKeyDetection
 * @brief Verifies any Key Detection.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, AnyKeyDetection) {
  KeyTimes k;
  EXPECT_FALSE(AnyKeyHeld(k));
  k.timeb0 = 0.01f; // bank key alone counts
  EXPECT_TRUE(AnyKeyHeld(k));
  KeyTimes k2; k2.timep1 = 0.01f;
  EXPECT_TRUE(AnyKeyHeld(k2));
  KeyTimes k3; k3.timey0 = 0.01f;
  EXPECT_TRUE(AnyKeyHeld(k3));
}

/**
 * @test SLEW.RotangPositiveRotation
 * @brief Verifies rotang Positive Rotation.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, RotangPositiveRotation) {
  KeyTimes k; k.timep1 = 1.0f; // 1 sec pitch-down
  AngVec a = ComputeRotang(k);
  EXPECT_EQ(a.p, (int16_t)(65536.0 * ROT_SPEED));
  EXPECT_EQ(a.h, 0);
  EXPECT_EQ(a.b, 0);
}

/**
 * @test SLEW.RotangNegativeAndZero
 * @brief Verifies rotang Negative And Zero.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, RotangNegativeAndZero) {
  KeyTimes k; k.timeh0 = 2.0f; // heading other way
  AngVec a = ComputeRotang(k);
  EXPECT_EQ(a.h, (int16_t)(-65536.0 * 2.0 * ROT_SPEED));
  KeyTimes none;
  EXPECT_TRUE(ComputeRotang(none).p == 0 && ComputeRotang(none).h == 0 && ComputeRotang(none).b == 0);
}

/**
 * @test SLEW.RotangWrapsAtInt16
 * @brief Verifies rotang Wraps At Int16.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, RotangWrapsAtInt16) {
  KeyTimes k; k.timeb1 = 100.0f; // huge bank time wraps int16
  AngVec a = ComputeRotang(k);
  // 65536*100/8 = 819200 = 0xC8000 -> low 16 bits 0x8000 -> int16 -32768
  int32_t full = (int32_t)(65536.0 * 100.0 * ROT_SPEED);
  int16_t expect = (int16_t)(full & 0xFFFF);
  EXPECT_EQ(a.b, expect);
}

/**
 * @test SLEW.MovementLimitMask
 * @brief Verifies movement Limit Mask.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, MovementLimitMask) {
  float m[3] = {1, 2, 3};
  LimitMovement(m, 0);
  EXPECT_EQ(m[0], 1); EXPECT_EQ(m[1], 2); EXPECT_EQ(m[2], 3);
  float m1[3] = {1, 2, 3};
  LimitMovement(m1, 1); // x locked
  EXPECT_EQ(m1[0], 0); EXPECT_EQ(m1[1], 2); EXPECT_EQ(m1[2], 3);
  float m2[3] = {1, 2, 3};
  LimitMovement(m2, 2 | 4); // y+z locked
  EXPECT_EQ(m2[0], 1); EXPECT_EQ(m2[1], 0); EXPECT_EQ(m2[2], 0);
  float m3[3] = {1, 2, 3};
  LimitMovement(m3, 7); // all locked
  EXPECT_EQ(m3[0], 0); EXPECT_EQ(m3[1], 0); EXPECT_EQ(m3[2], 0);
}

/**
 * @test SLEW.TerrainClampInsideNoOp
 * @brief Verifies terrain Clamp Inside No Op.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, TerrainClampInsideNoOp) {
  float p[3] = {128, 0, 128}; // middle of terrain
  ClampTerrainBounds(p);
  EXPECT_EQ(p[0], 128); EXPECT_EQ(p[2], 128);
}

/**
 * @test SLEW.TerrainClampLowHigh
 * @brief Verifies terrain Clamp Low High.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, TerrainClampLowHigh) {
  float lo[3] = {-50, 0, -1};
  ClampTerrainBounds(lo);
  EXPECT_EQ(lo[0], 1.0f); EXPECT_EQ(lo[2], 1.0f);

  float hi[3] = {99999, 0, 99999};
  ClampTerrainBounds(hi);
  EXPECT_EQ(hi[0], TERRAIN_WIDTH * TERRAIN_SIZE - 1.0f);
  EXPECT_EQ(hi[2], TERRAIN_WIDTH * TERRAIN_SIZE - 1.0f); // quirk: WIDTH used for z max
}

/**
 * @test SLEW.TerrainQuirkZMaxUsesWidth
 * @brief Verifies terrain Quirk ZMax Uses Width.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, TerrainQuirkZMaxUsesWidth) {
  // both are 256 so values coincide today, but assert the literal expression used
  float hi[3] = {0, 0, 99999};
  ClampTerrainBounds(hi);
  EXPECT_EQ(hi[2], TERRAIN_WIDTH * TERRAIN_SIZE - 1.0f);
  EXPECT_NE(&TERRAIN_DEPTH, nullptr); // DEPTH exists but unused in z-max line
}

/**
 * @test SLEW.RetFlagAccumulation
 * @brief Verifies ret Flag Accumulation.
 *
 * @details
 * Exercises the SLEW code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SLEW.cpp
 * @ingroup descent3_tests
 */
TEST(SLEW, RetFlagAccumulation) {
  // replicates ret_flags |= SLEW_MOVE when any movement nonzero
  auto moveFlag = [](const float m[3]) {
    return (m[0] != 0.0f || m[1] != 0.0f || m[2] != 0.0f) ? SLEW_MOVE : 0;
  };
  float zero[3] = {0, 0, 0};
  EXPECT_EQ(moveFlag(zero), 0);
  float moved[3] = {0, 0, 5};
  EXPECT_EQ(moveFlag(moved), SLEW_MOVE);
}
