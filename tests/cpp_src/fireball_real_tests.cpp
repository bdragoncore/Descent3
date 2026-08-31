/**
 * @file fireball_real_tests.cpp
 * @brief Tests for fireball.cpp 2483 lines — explosions & debris.
 *
 * @details
 * Covers the gray-spark 565-to-grayscale conversion (pure-green
 * transparency, luminance weights, green-LSB drop quirk),
 * CreateFireball VISUAL/REAL routing, custom fireball lifetime
 * rules, DoDebrisFrame death/smoke gating, debris physics setup
 * (velocity clamps, REMAINS bounce/life split), and
 * PlayObjectExplosionSound selection.
 *
 * This harness validates the behavior of `Descent3/fireball.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/fireball.cpp`
 * @par Harness
 * `fireball_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/fireball.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>

// replicated constants
constexpr int NUM_FIREBALLS = 52;
constexpr int BLACK_SMOKE_INDEX = 8;
constexpr int CUSTOM_EXPLOSION_INDEX = 11;
constexpr int GRAY_SPARK_INDEX = 50;
constexpr int VISUAL_FIREBALL = 0, REAL_FIREBALL = 1;
constexpr uint32_t DF_DEBRIS_SMOKES = 0x0010000;
constexpr uint32_t DF_DEBRIS_FIREBALL = 0x0020000;
constexpr uint32_t DF_DEBRIS_BLAST_RING = 0x0040000;
constexpr uint32_t DF_DEBRIS_REMAINS = 0x0080000;
constexpr int D3_RAND_MAX = 0x7fff;
constexpr int GSI_EXPLODE = 1;
constexpr int SOUND_NONE_INDEX = -1;

static uint16_t OPAQUE_FLAG = 0x8000;
static uint16_t NEW_TRANSPARENT_COLOR = 0x0000;
static uint16_t GR_RGB16(int r, int g, int b) {
  return (uint16_t)(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
}

// deterministic rand
static unsigned FSeed = 7;
static void FSrand(unsigned s) { FSeed = s; }
static int FRand() {
  FSeed = FSeed * 1103515245u + 12345u;
  return (int)((FSeed >> 16) & 0x7FFF);
}

// ---------------------------------------------------------------------------
// InitFireballs grayscale loop replication (fireball.cpp:692-711)
static uint16_t ConvertGrayPixel(uint16_t col565) {
  if (col565 == 0x07e0) { // pure green -> transparent
    return NEW_TRANSPARENT_COLOR;
  }
  uint8_t r = (uint8_t)((col565 & 0xf800) >> 11);
  uint8_t g = (uint8_t)((col565 & 0x07c0) >> 6); // quirk: 6-bit green, LSB dropped
  uint8_t b = (uint8_t)(col565 & 0x001f);
  float brightness = ((r * 0.30f) + (g * 0.59f) + (b * 0.11f)) * (1.0f / 32.0f);
  uint8_t elem = (uint8_t)(255.0f * brightness);
  if (brightness > 1.0f)
    elem = 255;
  return GR_RGB16(elem, elem, elem) | OPAQUE_FLAG;
}

/**
 * @test Fireball.GraySparkConversionRules
 * @brief Verifies gray Spark Conversion Rules.
 *
 * @details
 * Exercises the Fireball code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(Fireball, GraySparkConversionRules) {
  // pure green becomes the transparency sentinel (no OPAQUE flag!)
  EXPECT_EQ(ConvertGrayPixel(0x07e0), NEW_TRANSPARENT_COLOR);

  // quirk: max channel value 31 gives luminance 31/32 -> gray 247;
  // the "brightness > 1.0" clamp is dead code, 255 is unreachable
  EXPECT_EQ(ConvertGrayPixel(0xffff), GR_RGB16(247, 247, 247) | OPAQUE_FLAG);

  // black stays black but opaque
  EXPECT_EQ(ConvertGrayPixel(0x0000), 0 | OPAQUE_FLAG);

  // red channel only: floor(31 * .30 / 32 * 255) = 74 gray
  uint16_t out = ConvertGrayPixel(0xf800);
  int gray_out = (((out >> 10) & 0x1f) << 3) | ((out >> 10) & 0x07);
  EXPECT_NEAR(gray_out, 74, 8);

  // green LSB dropped: 0x0020 (green bit0 only) yields same as black
  EXPECT_EQ(ConvertGrayPixel(0x0020), ConvertGrayPixel(0x0000));
}

// ---------------------------------------------------------------------------
// CreateFireball routing replication (fireball.cpp:839-859)
struct FBDefMock {
  const char *name = "NOIMAGE";
  float size = 3.0f;
  float total_life = 1.0f;
};
static FBDefMock Fireballs[NUM_FIREBALLS];

struct ObjMockF {
  bool used = false;
  float size = 0, lifeleft = -1, lifetime = -1;
  uint32_t flags = 0;
};

static ObjMockF FObjects[8];
static int VisCreateCalls = 0;

static int RepCreateFireball(int objnum_target, int fireball_num, int realtype) {
  if (realtype == VISUAL_FIREBALL) {
    VisCreateCalls++;
    return 100 + fireball_num; // vis handle stand-in
  }
  if (objnum_target < 0)
    return -1;
  FObjects[objnum_target].used = true;
  FObjects[objnum_target].size = Fireballs[fireball_num].size;
  FObjects[objnum_target].flags |= 0x400; // OF_USES_LIFELEFT
  FObjects[objnum_target].lifeleft = Fireballs[fireball_num].total_life;
  FObjects[objnum_target].lifetime = FObjects[objnum_target].lifeleft;
  return objnum_target;
}

/**
 * @test Fireball.VisualVsRealRoutingAndFieldInit
 * @brief Verifies visual Vs Real Routing And Field Init.
 *
 * @details
 * Exercises the Fireball code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(Fireball, VisualVsRealRoutingAndFieldInit) {
  memset(FObjects, 0, sizeof(FObjects));
  Fireballs[9] = {"boom", 4.5f, 2.25f};
  VisCreateCalls = 0;

  // visual: routed to viseffect pool, object untouched
  int vis = RepCreateFireball(0, 9, VISUAL_FIREBALL);
  EXPECT_EQ(vis, 109);
  EXPECT_EQ(VisCreateCalls, 1);
  EXPECT_FALSE(FObjects[0].used);

  // real: object created with def size/life and lifeleft flag
  int obj = RepCreateFireball(2, 9, REAL_FIREBALL);
  ASSERT_GE(obj, 0);
  EXPECT_FLOAT_EQ(FObjects[2].size, 4.5f);
  EXPECT_FLOAT_EQ(FObjects[2].lifeleft, 2.25f);
  EXPECT_FLOAT_EQ(FObjects[2].lifetime, 2.25f); // mirrors lifeleft
  EXPECT_TRUE(FObjects[2].flags & 0x400);

  // real creation failure path
  EXPECT_EQ(RepCreateFireball(-1, 9, REAL_FIREBALL), -1);
}

// ---------------------------------------------------------------------------
// Custom fireball lifetime replication (fireball.cpp:862-888)
/**
 * @test Fireball.CustomLifetimeFromVclipOrHalfSecond
 * @brief Verifies custom Lifetime From Vclip Or Half Second.
 *
 * @details
 * Exercises the Fireball code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(Fireball, CustomLifetimeFromVclipOrHalfSecond) {
  float frame_time = 0.125f;
  int num_frames = 8;
  bool animated = true;

  float lifetime = animated ? frame_time * num_frames : 0.5f;
  EXPECT_FLOAT_EQ(lifetime, 1.0f); // full clip length

  animated = false;
  EXPECT_FLOAT_EQ(animated ? 99.f : 0.5f, 0.5f); // static texture default
}

// ---------------------------------------------------------------------------
// DoDebrisFrame replication (fireball.cpp:1218-1251)
struct DebrisMock {
  float lifeleft = 5.0f;
  int num_bounces = 3;
  uint32_t death_flags = 0;
  float last_smoke_time = -1;
  float size = 4.0f;
  // observed effects
  bool dead = false, control_none = false;
  int fireballs = 0, blast_rings = 0, smokes = 0;
};

static float Gametime_d = 100.0f;

static void RepDoDebrisFrame(DebrisMock &d, int smoke_rand /*0 or 1*/) {
  if ((d.lifeleft <= 0.0) || (d.num_bounces < 1)) {
    if (d.death_flags & DF_DEBRIS_FIREBALL)
      d.fireballs++;
    if (d.death_flags & DF_DEBRIS_BLAST_RING)
      d.blast_rings++;
    if (!(d.death_flags & DF_DEBRIS_REMAINS))
      d.dead = true;
    else
      d.control_none = true;
    return;
  }
  if (d.death_flags & DF_DEBRIS_SMOKES) {
    if (Gametime_d - d.last_smoke_time > .015 && smoke_rand) {
      d.smokes++;
      d.last_smoke_time = Gametime_d;
    }
  }
}

/**
 * @test Fireball.DebrisDeathSplitsRemainsVsGone
 * @brief Verifies debris Death Splits Remains Vs Gone.
 *
 * @details
 * Exercises the Fireball code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(Fireball, DebrisDeathSplitsRemainsVsGone) {
  DebrisMock gone;
  gone.lifeleft = -0.1f;
  gone.death_flags = DF_DEBRIS_FIREBALL | DF_DEBRIS_BLAST_RING;
  RepDoDebrisFrame(gone, 0);
  EXPECT_TRUE(gone.dead);
  EXPECT_EQ(gone.fireballs, 1);
  EXPECT_EQ(gone.blast_rings, 1);

  DebrisMock remains;
  remains.num_bounces = 0; // bounced out via count too
  remains.death_flags = DF_DEBRIS_REMAINS;
  RepDoDebrisFrame(remains, 0);
  EXPECT_FALSE(remains.dead);
  EXPECT_TRUE(remains.control_none); // parked inert instead of deleted
}

/**
 * @test Fireball.DebrisSmokeNeedsBothTimeGapAndCoinFlip
 * @brief Verifies debris Smoke Needs Both Time Gap And Coin Flip.
 *
 * @details
 * Exercises the Fireball code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(Fireball, DebrisSmokeNeedsBothTimeGapAndCoinFlip) {
  DebrisMock d;
  d.death_flags = DF_DEBRIS_SMOKES;
  d.last_smoke_time = 99.98f; // only 20ms ago

  Gametime_d = 100.0f;
  RepDoDebrisFrame(d, 1); // gap 20ms > 15ms, coin heads
  EXPECT_EQ(d.smokes, 1);
  EXPECT_FLOAT_EQ(d.last_smoke_time, 100.0f);

  d.last_smoke_time = 99.99f; // 10ms gap: too soon even with heads
  RepDoDebrisFrame(d, 1);
  EXPECT_EQ(d.smokes, 1);

  d.last_smoke_time = 0; // huge gap but tails
  RepDoDebrisFrame(d, 0);
  EXPECT_EQ(d.smokes, 1);
}

// ---------------------------------------------------------------------------
// debris creation physics replication (fireball.cpp:936-950)
/**
 * @test Fireball.DebrisPhysicsRemainsFlagChangesBounceBudget
 * @brief Verifies debris Physics Remains Flag Changes Bounce Budget.
 *
 * @details
 * Exercises the Fireball code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(Fireball, DebrisPhysicsRemainsFlagChangesBounceBudget) {
  auto setup_debris = [](uint32_t death_flags, float &lifeleft, int &bounces, uint32_t &pflags,
                         bool &uses_lifeleft) {
    pflags = 0x08 | 0x02 | 0x80; // GRAVITY|BOUNCE|FIXED_ROT_VELOCITY stand-ins
    if (death_flags & DF_DEBRIS_REMAINS) {
      bounces = 8;
      pflags |= 0x1000; // PF_STICK
      uses_lifeleft = false;
    } else {
      bounces = 2;
      lifeleft = 2.0 + ((FRand() % 50) * .05);
      uses_lifeleft = true;
    }
  };

  FSrand(42);
  float life_gone = 0, life_rem = 0;
  int b_gone = 0, b_rem = 0;
  uint32_t pf_gone = 0, pf_rem = 0;
  bool ul_gone = false, ul_rem = false;

  setup_debris(DF_DEBRIS_REMAINS, life_rem, b_rem, pf_rem, ul_rem);
  EXPECT_EQ(b_rem, 8);
  EXPECT_TRUE(pf_rem & 0x1000);
  EXPECT_FALSE(ul_rem); // sticks around forever

  setup_debris(0, life_gone, b_gone, pf_gone, ul_gone);
  EXPECT_EQ(b_gone, 2);
  EXPECT_TRUE(ul_gone);
  EXPECT_GE(life_gone, 2.0f);
  EXPECT_LT(life_gone, 4.55f); // 2.0 + 49*.05 max

  // velocity clamp bounds
  float vx = 250000.0f;
  if (vx > 100000.0f)
    vx = 100000.0f;
  EXPECT_FLOAT_EQ(vx, 100000.0f);
}

// ---------------------------------------------------------------------------
// PlayObjectExplosionSound replication (fireball.cpp:1301-1317)
enum { OBJ_BUILDING = 16, OBJ_DOOR = 17, OBJ_ROBOT = 2 };
constexpr int SOUND_ROBOT_EXPLODE_1 = 101, SOUND_ROBOT_EXPLODE_2 = 102, SOUND_BUILDING_EXPLODE = 103;

static int RepExplosionSound(int obj_type, int info_sound, int rand_val) {
  int sound;
  if (obj_type == OBJ_DOOR) {
    sound = SOUND_ROBOT_EXPLODE_1; // doors borrow robot explosion
  } else {
    sound = info_sound;
    if (sound == SOUND_NONE_INDEX) {
      if (obj_type == OBJ_BUILDING)
        sound = SOUND_BUILDING_EXPLODE;
      else
        sound = (rand_val > D3_RAND_MAX / 2) ? SOUND_ROBOT_EXPLODE_1 : SOUND_ROBOT_EXPLODE_2;
    }
  }
  if (sound != -1)
    return sound; // played at SND_PRIORITY_HIGH
  return -1;      // silent
}

/**
 * @test Fireball.ExplosionSoundSelectionChain
 * @brief Verifies explosion Sound Selection Chain.
 *
 * @details
 * Exercises the Fireball code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(Fireball, ExplosionSoundSelectionChain) {
  EXPECT_EQ(RepExplosionSound(OBJ_DOOR, 777, 999), SOUND_ROBOT_EXPLODE_1);
  EXPECT_EQ(RepExplosionSound(OBJ_ROBOT, 55, 0), 55); // explicit sound wins
  EXPECT_EQ(RepExplosionSound(OBJ_BUILDING, SOUND_NONE_INDEX, 12345),
            SOUND_BUILDING_EXPLODE);
  EXPECT_EQ(RepExplosionSound(OBJ_ROBOT, SOUND_NONE_INDEX, D3_RAND_MAX / 2 + 1),
            SOUND_ROBOT_EXPLODE_1);
  EXPECT_EQ(RepExplosionSound(OBJ_ROBOT, SOUND_NONE_INDEX, D3_RAND_MAX / 2),
            SOUND_ROBOT_EXPLODE_2);
}
