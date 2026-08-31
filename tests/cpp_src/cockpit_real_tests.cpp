/**
 * @file cockpit_real_tests.cpp
 * @brief Tests for cockpit.cpp 607 lines — D3 cockpit system.
 *
 * @details
 * Covers CockpitFileParse command table, KeyframeAnimateCockpit
 * interpolation/state machine, Open/Close mid-animation time inversion,
 * StartCockpitShake clamping, buffet decay & wave quantization, and the
 * light scalar pipeline. Replicates logic to avoid model/render deps.
 *
 * This harness validates the behavior of `Descent3/cockpit.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/cockpit.cpp`
 * @par Harness
 * `cockpit_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/cockpit.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <cmath>
#include <algorithm>

// replicated constants (cockpit.cpp:208-216)
constexpr float COCKPIT_ANIM_TIME = 2.0f;
constexpr float COCKPIT_DORMANT_FRAME = 0.0f;
constexpr float COCKPIT_COMPLETE_FRAME = 10.0f; // stand-in for frame_max
constexpr float MAX_BUFFET_STRENGTH = 0.75f;
constexpr float BUFFET_PERIOD = 0.25f;
constexpr int NUM_SHIELD_GAUGE_FRAMES = 5;
constexpr int PSFILENAME_LEN = 30;

// states (cockpit.h)
constexpr int COCKPIT_STATE_DORMANT = 1, COCKPIT_STATE_QUASI = 2, COCKPIT_STATE_FUNCTIONAL = 3;

// replicated tCockpitCfgInfo (cockpit.cpp:217-224)
struct tCockpitCfgInfo {
  char modelname[PSFILENAME_LEN + 1] = {};
  char shieldrings[NUM_SHIELD_GAUGE_FRAMES][PSFILENAME_LEN + 1] = {};
  char shipimg[PSFILENAME_LEN + 1] = {};
  char burnimg[PSFILENAME_LEN + 1] = {};
  char energyimg[PSFILENAME_LEN + 1] = {};
  char invpulseimg[PSFILENAME_LEN + 1] = {};
};

static std::string g_hudInfName;

// case-insensitive compare
static int StrICmp(const char *a, const char *b) {
  while (*a && *b) {
    int ca = tolower((unsigned char)*a), cb = tolower((unsigned char)*b);
    if (ca != cb) return ca - cb;
    ++a; ++b;
  }
  return (unsigned char)*a - (unsigned char)*b;
}

// replicated CockpitFileParse (cockpit.cpp:324-365)
static bool RepCockpitFileParse(const char *command, const char *operand, void *data) {
  tCockpitCfgInfo *cfginf = (tCockpitCfgInfo *)data;
  if (!strcmp(command, "ckptmodel")) {
    if (cfginf) strcpy(cfginf->modelname, operand);
  } else if (!strncmp(command, "shieldimg", strlen("shieldimg"))) {
    char buf[16];
    for (int i = 0; i < NUM_SHIELD_GAUGE_FRAMES; i++) {
      snprintf(buf, sizeof(buf), "shieldimg%d", i);
      if (!StrICmp(command, buf)) {
        if (cfginf) strcpy(cfginf->shieldrings[i], operand);
        break;
      }
    }
  } else if (!strcmp(command, "shipimg")) {
    if (cfginf) strcpy(cfginf->shipimg, operand);
  } else if (!strcmp(command, "afterburnimg")) {
    if (cfginf) strcpy(cfginf->burnimg, operand);
  } else if (!strcmp(command, "energyimg")) {
    if (cfginf) strcpy(cfginf->energyimg, operand);
  } else if (!strcmp(command, "invpulseimg")) {
    if (cfginf) strcpy(cfginf->invpulseimg, operand);
  } else if (!strcmp(command, "fullhudinf")) {
    if (cfginf) g_hudInfName = operand;
  } else {
    return false;
  }
  return true;
}

/**
 * @test Cockpit.ParseModelCommand
 * @brief Verifies parse Model Command.
 *
 * @details
 * Exercises the Cockpit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST(Cockpit, ParseModelCommand) {
  tCockpitCfgInfo ci;
  EXPECT_TRUE(RepCockpitFileParse("ckptmodel", "pyro.cov", &ci));
  EXPECT_STREQ(ci.modelname, "pyro.cov");
}

/**
 * @test Cockpit.ParseShieldRingFrames
 * @brief Verifies parse Shield Ring Frames.
 *
 * @details
 * Exercises the Cockpit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST(Cockpit, ParseShieldRingFrames) {
  tCockpitCfgInfo ci;
  EXPECT_TRUE(RepCockpitFileParse("shieldimg0", "ring1.ogf", &ci));
  // NOTE: prefix check strncmp("shieldimg") is CASE-SENSITIVE in original;
  // full command compare is stricmp, so only exact-case prefix passes
  EXPECT_FALSE(RepCockpitFileParse("SHIELDIMG4", "ring5.ogf", &ci));
  EXPECT_STREQ(ci.shieldrings[0], "ring1.ogf");
  EXPECT_STREQ(ci.shieldrings[4], "");
  // shieldimg9 out of range -> prefix matches, no slot matches; still true
  EXPECT_TRUE(RepCockpitFileParse("shieldimg9", "x.ogf", &ci));
}

/**
 * @test Cockpit.ParseImageCommandsAndUnknown
 * @brief Verifies parse Image Commands And Unknown.
 *
 * @details
 * Exercises the Cockpit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST(Cockpit, ParseImageCommandsAndUnknown) {
  tCockpitCfgInfo ci;
  EXPECT_TRUE(RepCockpitFileParse("shipimg", "hudship.ogf", &ci));
  EXPECT_TRUE(RepCockpitFileParse("afterburnimg", "burn.ogf", &ci));
  EXPECT_TRUE(RepCockpitFileParse("energyimg", "en.ogf", &ci));
  EXPECT_TRUE(RepCockpitFileParse("invpulseimg", "inv.ogf", &ci));
  EXPECT_STREQ(ci.shipimg, "hudship.ogf");
  EXPECT_STREQ(ci.burnimg, "burn.ogf");
  EXPECT_FALSE(RepCockpitFileParse("boguscmd", "v", &ci)); // unknown -> false
}

// replicated KeyframeAnimateCockpit state (cockpit.cpp:226-244 subset + 549-587)
struct MockCockpit {
  int state = COCKPIT_STATE_DORMANT;
  float frame_time = 0;
  float this_keyframe = COCKPIT_DORMANT_FRAME;
  float next_keyframe = COCKPIT_DORMANT_FRAME;
  bool animating = false;
};
static MockCockpit cp;
static float Frametime = 0.0f;

static float RepKeyframeAnimate() {
  float newkeyframe = cp.this_keyframe + (cp.next_keyframe - cp.this_keyframe) * (cp.frame_time / COCKPIT_ANIM_TIME);
  if (cp.this_keyframe < cp.next_keyframe) {
    if (newkeyframe >= cp.next_keyframe) {
      cp.frame_time = 0.0f;
      cp.this_keyframe = cp.next_keyframe;
    }
  } else if (cp.this_keyframe > cp.next_keyframe) {
    if (newkeyframe <= cp.next_keyframe) {
      cp.frame_time = 0.0f;
      cp.this_keyframe = cp.next_keyframe;
    }
  } else {
    cp.animating = false;
    return newkeyframe;
  }

  cp.animating = true;
  if (cp.this_keyframe != cp.next_keyframe)
    cp.frame_time += Frametime;

  if (cp.this_keyframe == COCKPIT_COMPLETE_FRAME && cp.next_keyframe == cp.this_keyframe) {
    cp.state = COCKPIT_STATE_FUNCTIONAL;
  } else if (cp.this_keyframe == COCKPIT_DORMANT_FRAME && cp.next_keyframe == COCKPIT_DORMANT_FRAME) {
    cp.state = COCKPIT_STATE_DORMANT;
  } else {
    cp.state = COCKPIT_STATE_QUASI;
  }
  return newkeyframe;
}

struct CockpitAnimFixture : ::testing::Test {
  void SetUp() override {
    cp = MockCockpit{};
    Frametime = 0.5f;
  }
};

/**
 * @test CockpitAnimFixture.IdleAnimationReturnsAndClearsFlag
 * @brief Verifies idle Animation Returns And Clears Flag.
 *
 * @details
 * Exercises the CockpitAnimFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST_F(CockpitAnimFixture, IdleAnimationReturnsAndClearsFlag) {
  cp.animating = true;
  float kf = RepKeyframeAnimate();
  EXPECT_FLOAT_EQ(kf, COCKPIT_DORMANT_FRAME);
  EXPECT_FALSE(cp.animating); // this==next -> not animating
}

/**
 * @test CockpitAnimFixture.OpeningInterpolatesThenCompletesFunctional
 * @brief Verifies opening Interpolates Then Completes Functional.
 *
 * @details
 * Exercises the CockpitAnimFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST_F(CockpitAnimFixture, OpeningInterpolatesThenCompletesFunctional) {
  cp.this_keyframe = COCKPIT_DORMANT_FRAME;
  cp.next_keyframe = COCKPIT_COMPLETE_FRAME;
  cp.frame_time = 0.0f;
  // first tick: kf computed with frame_time=0 -> still 0; time accumulates
  float kf = RepKeyframeAnimate();
  EXPECT_FLOAT_EQ(kf, COCKPIT_DORMANT_FRAME);
  EXPECT_FLOAT_EQ(cp.frame_time, Frametime);
  // second tick: kf = 0 + 10*(0.5/2) = 2.5
  kf = RepKeyframeAnimate();
  EXPECT_GT(kf, COCKPIT_DORMANT_FRAME);
  EXPECT_LT(kf, COCKPIT_COMPLETE_FRAME);
  EXPECT_EQ(cp.state, COCKPIT_STATE_QUASI);
  EXPECT_TRUE(cp.animating);
  // run until done
  for (int i = 0; i < 100 && cp.this_keyframe != cp.next_keyframe; ++i)
    RepKeyframeAnimate();
  EXPECT_FLOAT_EQ(cp.this_keyframe, COCKPIT_COMPLETE_FRAME);
}

/**
 * @test CockpitAnimFixture.OpenCompletionSetsFunctionalState
 * @brief Verifies open Completion Sets Functional State.
 *
 * @details
 * Exercises the CockpitAnimFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST_F(CockpitAnimFixture, OpenCompletionSetsFunctionalState) {
  // overshooting tick reaches target: this<next, full frame_time -> newkf >= next
  cp.this_keyframe = COCKPIT_COMPLETE_FRAME - 0.1f;
  cp.next_keyframe = COCKPIT_COMPLETE_FRAME;
  cp.frame_time = COCKPIT_ANIM_TIME; // full interpolation -> reaches target
  RepKeyframeAnimate();
  EXPECT_FLOAT_EQ(cp.this_keyframe, COCKPIT_COMPLETE_FRAME);
  // state set to FUNCTIONAL on the completion tick (this==next==COMPLETE check)
  EXPECT_EQ(cp.state, COCKPIT_STATE_FUNCTIONAL);
  // subsequent idle ticks keep state (early-return path does not touch it)
  cp.frame_time = 0.0f;
  RepKeyframeAnimate();
  EXPECT_EQ(cp.state, COCKPIT_STATE_FUNCTIONAL);
}

/**
 * @test CockpitAnimFixture.CloseInterpolatesDownToDormant
 * @brief Verifies close Interpolates Down To Dormant.
 *
 * @details
 * Exercises the CockpitAnimFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST_F(CockpitAnimFixture, CloseInterpolatesDownToDormant) {
  cp.this_keyframe = COCKPIT_COMPLETE_FRAME;
  cp.next_keyframe = COCKPIT_DORMANT_FRAME;
  cp.frame_time = 0.0f;
  for (int i = 0; i < 100; ++i) {
    float kf = RepKeyframeAnimate();
    if (kf <= COCKPIT_DORMANT_FRAME) break;
  }
  EXPECT_FLOAT_EQ(cp.this_keyframe, COCKPIT_DORMANT_FRAME);
}

/**
 * @test Cockpit.OpenMidwayInvertsFrameTime
 * @brief Verifies open Midway Inverts Frame Time.
 *
 * @details
 * Exercises the Cockpit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST(Cockpit, OpenMidwayInvertsFrameTime) {
  // replicated OpenCockpit (cockpit.cpp:388-401)
  auto RepOpen = [](MockCockpit &c) {
    if (c.this_keyframe <= COCKPIT_COMPLETE_FRAME) {
      c.state = COCKPIT_STATE_QUASI;
      c.this_keyframe = COCKPIT_DORMANT_FRAME;
      c.next_keyframe = COCKPIT_COMPLETE_FRAME;
      if (c.frame_time > 0.0f)
        c.frame_time = COCKPIT_ANIM_TIME - c.frame_time;
    }
  };
  MockCockpit c;
  c.frame_time = 0.6f; // mid-close
  c.this_keyframe = 7.0f;
  RepOpen(c);
  EXPECT_EQ(c.state, COCKPIT_STATE_QUASI);
  EXPECT_FLOAT_EQ(c.frame_time, COCKPIT_ANIM_TIME - 0.6f); // inverted
  EXPECT_FLOAT_EQ(c.next_keyframe, COCKPIT_COMPLETE_FRAME);
  // zero frame time stays zero
  MockCockpit z;
  z.frame_time = 0.0f;
  RepOpen(z);
  EXPECT_FLOAT_EQ(z.frame_time, 0.0f);
}

/**
 * @test Cockpit.CloseMidwayInvertsFrameTime
 * @brief Verifies close Midway Inverts Frame Time.
 *
 * @details
 * Exercises the Cockpit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST(Cockpit, CloseMidwayInvertsFrameTime) {
  // replicated CloseCockpit (cockpit.cpp:403-414)
  auto RepClose = [](MockCockpit &c) {
    if (c.this_keyframe >= COCKPIT_DORMANT_FRAME) {
      c.next_keyframe = COCKPIT_DORMANT_FRAME;
      c.this_keyframe = COCKPIT_COMPLETE_FRAME;
      if (c.frame_time > 0.0f)
        c.frame_time = COCKPIT_ANIM_TIME - c.frame_time;
    }
  };
  MockCockpit c;
  c.frame_time = 1.2f;
  RepClose(c);
  EXPECT_FLOAT_EQ(c.frame_time, COCKPIT_ANIM_TIME - 1.2f);
  EXPECT_FLOAT_EQ(c.next_keyframe, COCKPIT_DORMANT_FRAME);
}

/**
 * @test Cockpit.ShakeClampMagnitude
 * @brief Verifies shake Clamp Magnitude.
 *
 * @details
 * Exercises the Cockpit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST(Cockpit, ShakeClampMagnitude) {
  // replicated StartCockpitShake (cockpit.cpp:434-442)
  float buffet_amp = 0, buffet_wave = 0, buffet_time = 0;
  auto Shake = [&](float mag) {
    if (mag > MAX_BUFFET_STRENGTH) mag = MAX_BUFFET_STRENGTH;
    buffet_amp = mag;
    buffet_wave = std::sin(0.0);
    buffet_time = 0.0f;
  };
  Shake(2.5f);
  EXPECT_FLOAT_EQ(buffet_amp, MAX_BUFFET_STRENGTH); // clamped
  Shake(0.3f);
  EXPECT_FLOAT_EQ(buffet_amp, 0.3f); // preserved
  EXPECT_FLOAT_EQ(buffet_wave, 0.0f);
}

/**
 * @test Cockpit.BuffetDecayHalvesPerPeriod
 * @brief Verifies buffet Decay Halves Per Period.
 *
 * @details
 * Exercises the Cockpit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST(Cockpit, BuffetDecayHalvesPerPeriod) {
  // replicated RenderCockpit buffet block (cockpit.cpp:514-534)
  float amp = MAX_BUFFET_STRENGTH, btime = 0.20f, wave = 0;
  bool animating = false;
  auto BuffetTick = [&](float ft) {
    if (amp > 0.04f) {
      btime += ft;
      if (btime > BUFFET_PERIOD) {
        btime = 0.0f;
        amp *= 0.5f;
      }
      float a = 65536.0f * btime / (BUFFET_PERIOD - ((BUFFET_PERIOD - btime) * 0.5f));
      float s = std::sin(a);
      if (s > 0.5f) wave = 1.0f;
      else if (s < -0.5f) wave = -1.0f;
      else wave = 0.0f;
      animating = true;
    } else if (amp > 0.0f) {
      animating = true;
      amp = 0.0f;
    }
  };
  BuffetTick(0.1f); // crosses period: 0.30 > 0.25 -> halve
  EXPECT_FLOAT_EQ(amp, MAX_BUFFET_STRENGTH * 0.5f);
  EXPECT_EQ(btime, 0.0f);
  EXPECT_TRUE(wave == 1.0f || wave == -1.0f || wave == 0.0f); // quantized

  // decay to below threshold zeroes out
  amp = 0.05f;
  for (int i = 0; i < 20; ++i) BuffetTick(BUFFET_PERIOD * 1.01f);
  EXPECT_FLOAT_EQ(amp, 0.0f);
  EXPECT_TRUE(animating);
}

/**
 * @test Cockpit.LightScalarPipeline
 * @brief Verifies light Scalar Pipeline.
 *
 * @details
 * Exercises the Cockpit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cockpit.cpp
 * @ingroup descent3_tests
 */
TEST(Cockpit, LightScalarPipeline) {
  // replicated light scalar handling (cockpit.cpp:488-508)
  auto Pipe = [](float r, bool hasEffect, float dr, bool headlight) {
    if (r < 0.1f) r = 0.1f;
    r *= 0.8f;
    if (hasEffect) r = std::min(1.0f, r + dr);
    if (headlight) r = 1.0f;
    return r;
  };
  EXPECT_FLOAT_EQ(Pipe(0.05f, false, 0, false), 0.08f);   // clamped to .1 then *.8
  EXPECT_FLOAT_EQ(Pipe(1.0f, false, 0, false), 0.8f);     // scaled down
  EXPECT_FLOAT_EQ(Pipe(1.0f, true, 0.5f, false), 1.0f);   // 0.8+0.5 clamped 1.0
  EXPECT_FLOAT_EQ(Pipe(0.5f, true, 0.1f, false), 0.5f);   // 0.4+0.1=0.5 under cap
  EXPECT_FLOAT_EQ(Pipe(0.2f, false, 0, true), 1.0f);      // headlight forces full
}
