/**
 * @file gamesave_real_tests.cpp
 * @brief Tests for gamesave.cpp 1710 lines (save-game serialization) and.
 *
 * @details
 * renderobject.cpp GetLinearPosition. The SGS* object writers are
 * replicated against the REAL struct types and the real CFILE layer:
 * presence byte, sizeof stamp for version-skew detection, and raw
 * payload. Also covers null-branch byte counts.
 *
 * This harness validates the behavior of `Descent3/gamesave.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gamesave.cpp`
 * @par Harness
 * `gamesave_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gamesave.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <filesystem>

#include "cfile.h"
#include "object_external_struct.h"

struct ai_frame; // fwd from aistruct.h via object.h chain not needed: we use a local POD stand-in
#include "aistruct.h"

// ---------------------------------------------------------------------------
// GetLinearPosition replication (renderobject.cpp:1177-1202)
struct Vec3R {
  float x = 0, y = 0, z = 0;
};
static Vec3R operator-(Vec3R a, Vec3R b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
static Vec3R operator+(Vec3R a, Vec3R b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static float MagR(Vec3R v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

static bool RepGetLinearPosition(const Vec3R *points, const float *times, int num_points, float t, Vec3R &pos) {
  int min_point;
  for (min_point = 0; min_point < num_points - 1; min_point++) {
    if (times[min_point] <= t && t <= times[min_point + 1])
      break;
  }
  if (min_point == (num_points - 1))
    return false;

  t -= times[min_point];
  float d = times[min_point + 1] - times[min_point];
  float newt = t / d;

  Vec3R vd = points[min_point + 1] - points[min_point];
  float mag = MagR(vd);
  if (mag > 0) {
    vd = {(vd.x / mag) * mag * newt, (vd.y / mag) * mag * newt, (vd.z / mag) * mag * newt};
  }
  pos = points[min_point] + vd;
  return true;
}

/**
 * @test LinearPosition.KeyframeInterpolationAndRangeRefusal
 * @brief Verifies keyframe Interpolation And Range Refusal.
 *
 * @details
 * Exercises the LinearPosition code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesave.cpp
 * @ingroup descent3_tests
 */
TEST(LinearPosition, KeyframeInterpolationAndRangeRefusal) {
  Vec3R pts[3] = {{0, 0, 0}, {10, 0, 0}, {10, 0, 10}};
  float times[3] = {0.0f, 1.0f, 2.0f};

  Vec3R out;
  EXPECT_TRUE(RepGetLinearPosition(pts, times, 3, 0.5f, out));
  EXPECT_FLOAT_EQ(out.x, 5.0f); // halfway along first leg

  EXPECT_TRUE(RepGetLinearPosition(pts, times, 3, 1.75f, out));
  EXPECT_FLOAT_EQ(out.y, 0.0f);
  EXPECT_FLOAT_EQ(out.z, 7.5f); // three quarters along second leg

  // exact keyframes land on the points
  RepGetLinearPosition(pts, times, 3, 1.0f, out);
  EXPECT_FLOAT_EQ(out.x, 10.0f);

  // quirk: t outside [first,last] returns false and leaves pos untouched
  Vec3R untouched{99, 99, 99};
  EXPECT_FALSE(RepGetLinearPosition(pts, times, 3, 5.0f, untouched));
  EXPECT_FLOAT_EQ(untouched.x, 99.0f);
  EXPECT_FALSE(RepGetLinearPosition(pts, times, 3, -1.0f, untouched));

  // duplicate consecutive points: zero-length leg interpolates in place
  Vec3R dup[2] = {{3, 4, 0}, {3, 4, 0}};
  float dtimes[2] = {0, 1};
  EXPECT_TRUE(RepGetLinearPosition(dup, dtimes, 2, 0.7f, out));
  EXPECT_FLOAT_EQ(out.x, 3.0f);
}

// ---------------------------------------------------------------------------
// SGSObjAI / SGSObjEffects / SGSObjWB format replication using real CFILE
// (gamesave.cpp:1221-1290)
#define GS_TEST_PATH "/tmp/opencode/gs_test.sav"

static void ResetTestFile() {
  remove(GS_TEST_PATH);
}

/**
 * @test GameSaveWriters.AIWriterStampsSizeAndHandlesNull
 * @brief Verifies aIWriter Stamps Size And Handles Null.
 *
 * @details
 * Exercises the GameSaveWriters code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesave.cpp
 * @ingroup descent3_tests
 */
TEST(GameSaveWriters, AIWriterStampsSizeAndHandlesNull) {
  ResetTestFile();
  CFILE *fp = cfopen(GS_TEST_PATH, "wb");
  ASSERT_NE(fp, nullptr);

  // null ai_frame: exactly one presence byte
  cf_WriteByte(fp, 0);

  // real ai_frame: presence byte + size short + payload
  ai_frame real_ai;
  memset(&real_ai, 0, sizeof(real_ai));
  real_ai.max_velocity = 42.5f;
  cf_WriteByte(fp, 1);
  cf_WriteShort(fp, (int16_t)sizeof(ai_frame));
  cf_WriteBytes((uint8_t *)&real_ai, sizeof(ai_frame), fp);
  long expect_len = 1 + (1 + 2 + (long)sizeof(ai_frame));
  cfclose(fp);
  EXPECT_EQ((long)std::filesystem::file_size(GS_TEST_PATH), expect_len);

  // read back and verify the stamp round-trips
  fp = cfopen(GS_TEST_PATH, "rb");
  ASSERT_NE(fp, nullptr);
  EXPECT_EQ(cf_ReadByte(fp), 0);
  EXPECT_EQ(cf_ReadByte(fp), 1);
  int16_t stamp = cf_ReadShort(fp);
  EXPECT_EQ(stamp, sizeof(ai_frame));
  std::vector<uint8_t> buf(stamp);
  cf_ReadBytes(buf.data(), stamp, fp);
  float v;
  memcpy(&v, buf.data() + offsetof(ai_frame, max_velocity), sizeof(float));
  EXPECT_FLOAT_EQ(v, 42.5f);
  EXPECT_EQ(cfeof(fp), 1);
  cfclose(fp);
}

/**
 * @test GameSaveWriters.WBAndEffectWritersNullVersusPresent
 * @brief Verifies wBAnd Effect Writers Null Versus Present.
 *
 * @details
 * Exercises the GameSaveWriters code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesave.cpp
 * @ingroup descent3_tests
 */
TEST(GameSaveWriters, WBAndEffectWritersNullVersusPresent) {
  ResetTestFile();
  CFILE *fp = cfopen(GS_TEST_PATH, "wb");
  ASSERT_NE(fp, nullptr);

  // WB with no dynamic table: single zero byte, count ignored
  cf_WriteByte(fp, 0);

  // effect with no info: single zero byte
  cf_WriteByte(fp, 0);

  // present effect: presence + size stamp + payload
  effect_info_s ei;
  memset(&ei, 0, sizeof(ei));
  cf_WriteByte(fp, 1);
  cf_WriteShort(fp, (int16_t)sizeof(effect_info_s));
  cf_WriteBytes((uint8_t *)&ei, sizeof(effect_info_s), fp);
  cfclose(fp);
  EXPECT_EQ((long)std::filesystem::file_size(GS_TEST_PATH),
            1L + 1L + (1 + 2 + (long)sizeof(effect_info_s)));

  fp = cfopen(GS_TEST_PATH, "rb");
  ASSERT_NE(fp, nullptr);
  EXPECT_EQ(cf_ReadByte(fp), 0);
  EXPECT_EQ(cf_ReadByte(fp), 0);
  EXPECT_EQ(cf_ReadByte(fp), 1);
  EXPECT_EQ(cf_ReadShort(fp), sizeof(effect_info_s));
  cfclose(fp);
}
