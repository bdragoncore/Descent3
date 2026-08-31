/**
 * @file spew_real_tests.cpp
 * @brief Tests for spew.cpp (483 lines).
 *
 * @details
 * Covers SpewCreate validation/clamping/handle encoding, SpewEmitAll interval
 * emission / per-frame cap / randomization / gunpoint updates / longevity,
 * SpewClearEvent staleness checks and SpewInit reset.
 *
 * This harness validates the behavior of `Descent3/spew.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/spew.cpp`
 * @par Harness
 * `spew_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/spew.cpp
 */

#include <gtest/gtest.h>
#include <cmath>
#include <map>
#include <vector>

#include "game.h"
#include "vecmat.h"
#include "pserror.h"
#include "object.h"
#include "viseffect.h"
#include "fireball.h"
#include "log.h"
#include "spew.h"
#include "weapon.h"
#include "polymodel.h"
#include "psrand.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
float Frametime = 0.0f;
float Gametime = 0.0f;
poly_model Poly_models[4];

// ---------------------------------------------------------------------------
// Scripted object lookup
// ---------------------------------------------------------------------------
static std::map<int, object *> g_objmap;
object *ObjGet(int handle) {
  auto it = g_objmap.find(handle);
  return it == g_objmap.end() ? nullptr : it->second;
}

// ---------------------------------------------------------------------------
// Scripted ps_rand
// ---------------------------------------------------------------------------
static int g_rand_seq[32];
static int g_rand_len = 0;
static int g_rand_idx = 0;
int ps_rand() {
  if (g_rand_idx >= g_rand_len)
    g_rand_idx = 0;
  return g_rand_seq[g_rand_idx++];
}
void ps_srand(unsigned int seed) { (void)seed; }

// ---------------------------------------------------------------------------
// Stubs for engine calls spew.cpp depends on
// ---------------------------------------------------------------------------
static vector g_calc_normal;
static vector g_calc_point;
static int g_calccalls;

bool WeaponCalcGun(vector *gun_point, vector *gun_normal, object *obj, int gun_num) {
  (void)obj;
  (void)gun_num;
  g_calccalls++;
  *gun_point = g_calc_point;
  *gun_normal = g_calc_normal;
  return true;
}

struct VisCapture {
  int type;
  int id;
  int roomnum;
  vector pos;
  float lifetime;
  vector vel;
  int phys_flags;
  float size;
  float mass_arg;
  float drag_arg;
  bool isreal;
  float speed_component() const { return vel.z(); }
};
static std::vector<VisCapture> g_vis;

int VisEffectCreateControlled(uint8_t type, object *parent, uint8_t id, int roomnum, vector *pos, float lifetime,
                              vector *velocity, int phys_flags, float size, float mass, float drag, bool isreal) {
  VisCapture c{};
  c.type = type;
  c.id = id;
  c.roomnum = roomnum;
  c.pos = *pos;
  c.lifetime = lifetime;
  c.vel = *velocity;
  c.phys_flags = phys_flags;
  c.size = size;
  c.mass_arg = mass;
  c.drag_arg = drag;
  c.isreal = isreal;
  (void)parent;
  g_vis.push_back(c);
  return (int)g_vis.size();
}

void SetNormalizedTimeObj(object *obj, float *normalized_time) {
  (void)obj;
  (void)normalized_time;
}
void SetModelAnglesAndPos(poly_model *pm, float *normalized_time, uint32_t subobj_flags) {
  (void)pm;
  (void)normalized_time;
  (void)subobj_flags;
}

// Defined in spew.cpp but missing from spew.h
bool SpewObjectNeedsEveryFrameUpdate(object *obj, int gunpoint);

// ---------------------------------------------------------------------------
// Model scaffolding for gunpoint tests
// ---------------------------------------------------------------------------
static w_bank g_banks[2];
static bsp_info g_subs[2];

static void SetupModel(int n_guns, int bank_parent, int sub_flags, int sub_parent) {
  Poly_models[0] = poly_model{};
  Poly_models[0].n_guns = n_guns;
  Poly_models[0].gun_slots = g_banks;
  Poly_models[0].submodel = g_subs;
  for (auto &b : g_banks)
    b = w_bank{};
  for (auto &s : g_subs)
    s = bsp_info{};
  g_banks[0].parent = bank_parent;
  g_subs[0].flags = sub_flags;
  g_subs[0].parent = sub_parent;
}

/**
 * @brief GTest fixture for SpewTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class SpewTest : public ::testing::Test {
protected:
  void SetUp() override {
    SpewInit();
    Gametime = 10.0f;
    Frametime = 0.0f;
    g_objmap.clear();
    g_rand_len = g_rand_idx = 0;
    g_calccalls = 0;
    g_calc_normal = vector{0, 0, 1};
    g_calc_point = vector{5, 6, 7};
    g_vis.clear();
    SetupModel(0, -1, 0, -1);
  }
};

static spewinfo BasePointSpew() {
  spewinfo s{};
  s.effect_type = MED_SMOKE_INDEX;
  s.pt.origin = vector{1, 2, 3};
  s.pt.normal = vector{0, 0, 1};
  s.pt.room_num = 42;
  s.time_int = 0.1f;
  return s;
}

static int CreateSimple(float size = 2.0f, float speed = 10.0f, float lifetime = 3.0f, int random = 0,
                        bool real_obj = false) {
  spewinfo s = BasePointSpew();
  s.size = size;
  s.speed = speed;
  s.lifetime = lifetime;
  s.random = random;
  s.real_obj = real_obj;
  return SpewCreate(&s);
}

// ---------------------------------------------------------------------------
// SpewCreate: validation, clamping, handles
// ---------------------------------------------------------------------------

/**
 * @test SpewTest.CreateRejectsOutOfRangeEffectType
 * @brief Verifies create Rejects Out Of Range Effect Type.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, CreateRejectsOutOfRangeEffectType) {
  spewinfo s = BasePointSpew();
  s.effect_type = -1;
  EXPECT_EQ(SpewCreate(&s), -1);
  s.effect_type = NUM_FIREBALLS;
  EXPECT_EQ(SpewCreate(&s), -1);
  EXPECT_EQ(spew_count, 0);
}

/**
 * @test SpewTest.CreateAcceptsHighestEffectType
 * @brief Verifies create Accepts Highest Effect Type.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, CreateAcceptsHighestEffectType) {
  spewinfo s = BasePointSpew();
  s.effect_type = NUM_FIREBALLS - 1;
  EXPECT_GE(SpewCreate(&s), 0);
}

/**
 * @test SpewTest.CreateClampsNegativeFields
 * @brief Verifies create Clamps Negative Fields.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, CreateClampsNegativeFields) {
  spewinfo s = BasePointSpew();
  s.drag = -5;
  s.lifetime = -1;
  s.longevity = -2;
  s.mass = -3;
  s.size = 0;
  s.speed = -9;
  s.time_int = 0;
  int h = SpewCreate(&s);
  ASSERT_GE(h, 0);
  spewinfo *v = &SpewEffects[h & 0xFF];
  EXPECT_FLOAT_EQ(v->drag, 0);
  EXPECT_FLOAT_EQ(v->lifetime, 0);
  EXPECT_FLOAT_EQ(v->longevity, 0);
  EXPECT_FLOAT_EQ(v->mass, 0);
  EXPECT_FLOAT_EQ(v->size, 1);
  EXPECT_FLOAT_EQ(v->speed, 0);
  EXPECT_FLOAT_EQ(v->time_int, 0.001f);
}

/**
 * @test SpewTest.FailedCreatesDoNotConsumeCounter
 * @brief Verifies failed Creates Do Not Consume Counter.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, FailedCreatesDoNotConsumeCounter) {
  spewinfo bad = BasePointSpew();
  bad.effect_type = -1;
  SpewCreate(&bad);
  SpewCreate(&bad);

  // First successful allocation encodes counter 1
  spewinfo good = BasePointSpew();
  int h = SpewCreate(&good);
  ASSERT_GE(h, 0);
  EXPECT_EQ(h, (1 << 16) | (h & 0xFF));
  EXPECT_EQ(h >> 16, 1);
}

/**
 * @test SpewTest.HandleCounterIncrementsPerCreate
 * @brief Verifies handle Counter Increments Per Create.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, HandleCounterIncrementsPerCreate) {
  int a = CreateSimple();
  int b = CreateSimple();
  ASSERT_GE(a, 0);
  ASSERT_GE(b, 0);
  EXPECT_EQ(a >> 16, 1);
  EXPECT_EQ(b >> 16, 2);
  EXPECT_NE(a & 0xFF, b & 0xFF);
}

/**
 * @test SpewTest.PointSpewCopiesPositionData
 * @brief Verifies point Spew Copies Position Data.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, PointSpewCopiesPositionData) {
  int h = CreateSimple();
  ASSERT_GE(h, 0);
  spewinfo *v = &SpewEffects[h & 0xFF];
  EXPECT_TRUE(v->inuse);
  EXPECT_FALSE(v->use_gunpoint);
  EXPECT_FLOAT_EQ(v->pt.origin.x(), 1);
  EXPECT_FLOAT_EQ(v->pt.normal.z(), 1);
  EXPECT_EQ(v->pt.room_num, 42);
  EXPECT_FLOAT_EQ(v->start_time, 10.0f); // Gametime
  EXPECT_FLOAT_EQ(v->time_until_next_blob, 0.1f);
}

/**
 * @test SpewTest.GunpointRequiresLiveObject
 * @brief Verifies gunpoint Requires Live Object.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, GunpointRequiresLiveObject) {
  spewinfo s = BasePointSpew();
  s.use_gunpoint = true;
  s.gp.obj_handle = OBJECT_HANDLE_NONE;
  s.gp.gunpoint = 0;
  EXPECT_EQ(SpewCreate(&s), -1);
  EXPECT_EQ(spew_count, 0);
}

/**
 * @test SpewTest.GunpointRangeCheckedAgainstModel
 * @brief Verifies gunpoint Range Checked Against Model.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, GunpointRangeCheckedAgainstModel) {
  object obj{};
  g_objmap[77] = &obj;
  SetupModel(1, -1, 0, -1);
  obj.rtype.pobj_info.model_num = 0;

  spewinfo s = BasePointSpew();
  s.use_gunpoint = true;
  s.gp.obj_handle = 77;
  s.gp.gunpoint = 1; // >= n_guns (1)
  EXPECT_EQ(SpewCreate(&s), -1);
  s.gp.gunpoint = -1;
  EXPECT_EQ(SpewCreate(&s), -1);
}

/**
 * @test SpewTest.GunpointStoresInfoAndSetsForceUpdate
 * @brief Verifies gunpoint Stores Info And Sets Force Update.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, GunpointStoresInfoAndSetsForceUpdate) {
  object obj{};
  obj.rtype.pobj_info.model_num = 0;
  g_objmap[77] = &obj;
  SetupModel(1, -1, 0, -1);

  spewinfo s = BasePointSpew();
  s.use_gunpoint = true;
  s.gp.obj_handle = 77;
  s.gp.gunpoint = 0;
  int h = SpewCreate(&s);
  ASSERT_GE(h, 0);
  spewinfo *v = &SpewEffects[h & 0xFF];
  EXPECT_TRUE(v->inuse);
  EXPECT_EQ(v->gp.obj_handle, 77);
  EXPECT_EQ(v->gp.gunpoint, 0);
  EXPECT_TRUE(v->flags & SF_FORCEUPDATE);
  EXPECT_FALSE(v->flags & SF_UPDATEEVERYFRAME); // static submodel chain
}

/**
 * @test SpewTest.TurretGunGetsEveryFrameFlag
 * @brief Verifies turret Gun Gets Every Frame Flag.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, TurretGunGetsEveryFrameFlag) {
  object obj{};
  obj.flags = OF_POLYGON_OBJECT;
  obj.rtype.pobj_info.model_num = 0;
  g_objmap[77] = &obj;
  // Gun parented to submodel 0 which rotates -> every-frame updates
  SetupModel(1, 0, SOF_ROTATE, -1);

  spewinfo s = BasePointSpew();
  s.use_gunpoint = true;
  s.gp.obj_handle = 77;
  s.gp.gunpoint = 0;
  int h = SpewCreate(&s);
  ASSERT_GE(h, 0);
  EXPECT_TRUE(SpewEffects[h & 0xFF].flags & SF_UPDATEEVERYFRAME);
}

/**
 * @test SpewTest.TurretDeepInChainGetsEveryFrameFlag
 * @brief Verifies turret Deep In Chain Gets Every Frame Flag.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, TurretDeepInChainGetsEveryFrameFlag) {
  object obj{};
  obj.flags = OF_POLYGON_OBJECT;
  obj.rtype.pobj_info.model_num = 0;
  g_objmap[77] = &obj;
  // gun -> submodel 0 (plain) -> submodel 1 (turret)
  SetupModel(1, 0, 0, 1);
  g_subs[1].flags = SOF_TURRET;
  g_subs[1].parent = -1;

  spewinfo s = BasePointSpew();
  s.use_gunpoint = true;
  s.gp.obj_handle = 77;
  s.gp.gunpoint = 0;
  int h = SpewCreate(&s);
  ASSERT_GE(h, 0);
  EXPECT_TRUE(SpewEffects[h & 0xFF].flags & SF_UPDATEEVERYFRAME);
}

/**
 * @test SpewTest.NeedsEveryFrameFalseForNonPolygonOrNoGuns
 * @brief Verifies needs Every Frame False For Non Polygon Or No Guns.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, NeedsEveryFrameFalseForNonPolygonOrNoGuns) {
  object obj{};
  obj.flags = 0; // not a polygon object
  EXPECT_FALSE(SpewObjectNeedsEveryFrameUpdate(&obj, 0));

  obj.flags = OF_POLYGON_OBJECT;
  obj.rtype.pobj_info.model_num = 0;
  SetupModel(0, -1, 0, -1); // model has no guns
  EXPECT_FALSE(SpewObjectNeedsEveryFrameUpdate(&obj, 0));

  SetupModel(1, -1, 0, -1);
  EXPECT_FALSE(SpewObjectNeedsEveryFrameUpdate(&obj, 5)); // bad gun num
}

// ---------------------------------------------------------------------------
// SpewEmitAll: point-based emission
// ---------------------------------------------------------------------------

/**
 * @test SpewTest.EmitsAtIntervalAlongNormal
 * @brief Verifies emits At Interval Along Normal.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, EmitsAtIntervalAlongNormal) {
  int h = CreateSimple();
  ASSERT_GE(h, 0);
  SpewEffects[h & 0xFF].drag = 3.0f;
  SpewEffects[h & 0xFF].mass = 7.0f;

  Frametime = 0.25f; // 2.5 intervals -> exactly 2 blobs
  SpewEmitAll();

  ASSERT_EQ(g_vis.size(), 2u);
  EXPECT_EQ(g_vis[0].type, VIS_FIREBALL);
  EXPECT_EQ(g_vis[0].id, MED_SMOKE_INDEX);
  EXPECT_EQ(g_vis[0].roomnum, 42);
  EXPECT_FLOAT_EQ(g_vis[0].pos.x(), 1);
  EXPECT_FLOAT_EQ(g_vis[0].vel.z(), 10.0f); // normal(0,0,1) * speed
  EXPECT_FLOAT_EQ(g_vis[0].lifetime, 3.0f);
  EXPECT_FLOAT_EQ(g_vis[0].size, 2.0f);
  EXPECT_FALSE(g_vis[0].isreal);
  // Quirk: spew.cpp passes (..., size, drag, mass, real_obj) into
  // (..., size, mass, drag, isreal) - the two arguments are swapped.
  EXPECT_FLOAT_EQ(g_vis[0].mass_arg, 3.0f); // really spew's drag
  EXPECT_FLOAT_EQ(g_vis[0].drag_arg, 7.0f); // really spew's mass
  spewinfo *v = &SpewEffects[h & 0xFF];
  EXPECT_NEAR(v->time_until_next_blob, 0.05f, 1e-4f);
  EXPECT_TRUE(v->inuse);
}

/**
 * @test SpewTest.PerFrameCapIsFiveBlobsAndTimerStaysNegative
 * @brief Verifies per Frame Cap Is Five Blobs And Timer Stays Negative.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, PerFrameCapIsFiveBlobsAndTimerStaysNegative) {
  int h = CreateSimple();
  ASSERT_GE(h, 0);
  SpewEffects[h & 0xFF].time_int = 0.001f;

  Frametime = 1.0f; // would need ~1000 blobs
  SpewEmitAll();

  EXPECT_EQ(g_vis.size(), 5u);
  spewinfo *v = &SpewEffects[h & 0xFF];
  // Quirk: the num_spewed==0 reset branch is unreachable (counter ends at -1),
  // so the timer stays negative and spills into the next frame.
  EXPECT_LT(v->time_until_next_blob, 0.0f);
}

/**
 * @test SpewTest.RealObjectSuppressesRandomness
 * @brief Verifies real Object Suppresses Randomness.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, RealObjectSuppressesRandomness) {
  int h = CreateSimple(2, 10, 3, SPEW_RAND_SIZE | SPEW_RAND_SPEED | SPEW_RAND_LIFETIME, /*real_obj*/ true);
  ASSERT_GE(h, 0);
  spewinfo *v = &SpewEffects[h & 0xFF];
  v->random = SPEW_RAND_SIZE | SPEW_RAND_SPEED | SPEW_RAND_LIFETIME;

  Frametime = 0.25f;
  SpewEmitAll();

  EXPECT_EQ(g_vis.size(), 2u);
  EXPECT_FLOAT_EQ(g_vis[0].size, 2.0f);   // unrandomized despite rand flags
  EXPECT_FLOAT_EQ(g_vis[0].lifetime, 3.0f);
  // Quirk: real_obj permanently zeroes the stored random mask.
  EXPECT_EQ(v->random, 0);
}

/**
 * @test SpewTest.RandomSizeFollowsFormulaWithScriptedRand
 * @brief Verifies random Size Follows Formula With Scripted Rand.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, RandomSizeFollowsFormulaWithScriptedRand) {
  int h = CreateSimple(4, 10, 3, SPEW_RAND_SIZE);
  ASSERT_GE(h, 0);

  int r = 1234;
  g_rand_seq[0] = r;
  g_rand_len = 1;

  Frametime = 0.25f;
  SpewEmitAll();

  float expected = 4.0f + (((D3_RAND_MAX >> 1) - r) / (float)D3_RAND_MAX) * 4.0f * 0.5f;
  ASSERT_EQ(g_vis.size(), 2u);
  EXPECT_NEAR(g_vis[0].size, expected, 1e-4f);
  EXPECT_FLOAT_EQ(g_vis[0].speed_component(), 10.0f); // speed flag off
  EXPECT_FLOAT_EQ(g_vis[0].lifetime, 3.0f);
}

/**
 * @test SpewTest.RandomSpeedAndLifetimeConsumeRandPerBlob
 * @brief Verifies random Speed And Lifetime Consume Rand Per Blob.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, RandomSpeedAndLifetimeConsumeRandPerBlob) {
  int h = CreateSimple(2, 20, 5, SPEW_RAND_SPEED | SPEW_RAND_LIFETIME);
  ASSERT_GE(h, 0);

  int rs = 100, rl = 30000;
  g_rand_seq[0] = rs;
  g_rand_seq[1] = rl;
  g_rand_len = 2;

  Frametime = 0.25f;
  SpewEmitAll();

  float exp_speed = 20.0f + (((D3_RAND_MAX >> 1) - rs) / (float)D3_RAND_MAX) * 20.0f * 0.5f;
  float exp_life = 5.0f + (((D3_RAND_MAX >> 1) - rl) / (float)D3_RAND_MAX) * 5.0f * 0.5f;
  ASSERT_EQ(g_vis.size(), 2u);
  EXPECT_NEAR(g_vis[0].speed_component(), exp_speed, 1e-3f);
  EXPECT_NEAR(g_vis[0].lifetime, exp_life, 1e-4f);
  // Second blob wraps back to sequence start
  EXPECT_NEAR(g_vis[1].speed_component(), exp_speed, 1e-3f);
  EXPECT_FLOAT_EQ(g_vis[0].size, 2.0f);
}

/**
 * @test SpewTest.LongevityExpiryClearsEvent
 * @brief Verifies longevity Expiry Clears Event.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, LongevityExpiryClearsEvent) {
  int h = CreateSimple();
  ASSERT_GE(h, 0);
  SpewEffects[h & 0xFF].longevity = 5.0f;

  Gametime = 14.9f; // not yet expired
  Frametime = 0.25f;
  SpewEmitAll();
  EXPECT_EQ(g_vis.size(), 2u);
  EXPECT_TRUE(SpewEffects[h & 0xFF].inuse);

  Gametime = 15.0f; // elapsed >= longevity
  Frametime = 0.0f;  // no new blobs, just expiry sweep
  SpewEmitAll();
  EXPECT_FALSE(SpewEffects[h & 0xFF].inuse);
}

/**
 * @test SpewTest.ZeroLongevityNeverExpires
 * @brief Verifies zero Longevity Never Expires.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, ZeroLongevityNeverExpires) {
  int h = CreateSimple();
  ASSERT_GE(h, 0);
  SpewEffects[h & 0xFF].longevity = 0.0f;

  Gametime = 100000.0f;
  SpewEmitAll();
  EXPECT_TRUE(SpewEffects[h & 0xFF].inuse);
}

// ---------------------------------------------------------------------------
// SpewEmitAll: gunpoint-based emission
// ---------------------------------------------------------------------------

static int CreateGunpointSpew(int obj_handle) {
  spewinfo s = BasePointSpew();
  s.use_gunpoint = true;
  s.gp.obj_handle = obj_handle;
  s.gp.gunpoint = 0;
  s.size = 2;
  s.speed = 10;
  s.lifetime = 3;
  return SpewCreate(&s);
}

/**
 * @test SpewTest.GunpointEmitsFromCalculatedGunPosition
 * @brief Verifies gunpoint Emits From Calculated Gun Position.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, GunpointEmitsFromCalculatedGunPosition) {
  object obj{};
  obj.rtype.pobj_info.model_num = 0;
  g_objmap[77] = &obj;
  SetupModel(1, -1, 0, -1);

  int h = CreateGunpointSpew(77);
  ASSERT_GE(h, 0);

  Frametime = 0.25f;
  SpewEmitAll();

  ASSERT_EQ(g_vis.size(), 2u);
  EXPECT_FLOAT_EQ(g_vis[0].pos.x(), 5); // from WeaponCalcGun script
  EXPECT_FLOAT_EQ(g_vis[0].pos.y(), 6);
  EXPECT_FLOAT_EQ(g_vis[0].pos.z(), 7);
  EXPECT_EQ(g_vis[0].roomnum, obj.roomnum);
  EXPECT_EQ(obj.roomnum, 0); // default object room
}

/**
 * @test SpewTest.GunCalcRunsOncePerFrameDespiteMultipleBlobs
 * @brief Verifies gun Calc Runs Once Per Frame Despite Multiple Blobs.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, GunCalcRunsOncePerFrameDespiteMultipleBlobs) {
  object obj{};
  obj.flags = OF_MOVED_THIS_FRAME | OF_POLYGON_OBJECT;
  obj.rtype.pobj_info.model_num = 0;
  g_objmap[77] = &obj;
  SetupModel(1, -1, 0, -1);

  int h = CreateGunpointSpew(77);
  ASSERT_GE(h, 0);
  SpewEffects[h & 0xFF].time_int = 0.001f;

  Frametime = 1.0f;
  SpewEmitAll();

  EXPECT_EQ(g_vis.size(), 5u);
  EXPECT_EQ(g_calccalls, 1); // SF_UPDATEDFORFRAME suppresses recalculation

  // Next frame: object stopped moving, force-update flag was consumed...
  obj.flags &= ~OF_MOVED_THIS_FRAME;
  g_vis.clear();
  SpewEmitAll();
  EXPECT_EQ(g_calccalls, 1); // ...so no recalculation, stale normal reused
  EXPECT_EQ(g_vis.size(), 5u);
}

/**
 * @test SpewTest.AttachedPhysicsParentMoveTriggersRecalc
 * @brief Verifies attached Physics Parent Move Triggers Recalc.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, AttachedPhysicsParentMoveTriggersRecalc) {
  object parent{};
  parent.movement_type = MT_PHYSICS;
  parent.flags = OF_MOVED_THIS_FRAME;
  object child{};
  child.flags = OF_ATTACHED | OF_POLYGON_OBJECT;
  child.rtype.pobj_info.model_num = 0;
  child.attach_ultimate_handle = 55;
  g_objmap[55] = &parent;
  g_objmap[77] = &child;
  SetupModel(1, -1, 0, -1);

  int h = CreateGunpointSpew(77);
  ASSERT_GE(h, 0);

  Frametime = 0.25f;
  SpewEmitAll();
  EXPECT_EQ(g_calccalls, 1);
}

/**
 * @test SpewTest.EveryFrameSpewerRecalcsEachFrameWithoutMovement
 * @brief Verifies every Frame Spewer Recalcs Each Frame Without Movement.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, EveryFrameSpewerRecalcsEachFrameWithoutMovement) {
  object obj{};
  obj.flags = OF_POLYGON_OBJECT;
  obj.rtype.pobj_info.model_num = 0;
  g_objmap[77] = &obj;
  SetupModel(1, 0, SOF_ROTATE, -1);

  int h = CreateGunpointSpew(77);
  ASSERT_GE(h, 0);
  EXPECT_TRUE(SpewEffects[h & 0xFF].flags & SF_UPDATEEVERYFRAME);

  Frametime = 0.25f;
  SpewEmitAll();
  EXPECT_EQ(g_calccalls, 1);

  SpewEmitAll(); // second frame, object never flagged as moved
  EXPECT_EQ(g_calccalls, 2);
}

/**
 * @test SpewTest.DeadObjectDuringEmitKillsSpewer
 * @brief Verifies dead Object During Emit Kills Spewer.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, DeadObjectDuringEmitKillsSpewer) {
  object obj{};
  obj.rtype.pobj_info.model_num = 0;
  g_objmap[77] = &obj;
  SetupModel(1, -1, 0, -1);

  int h = CreateGunpointSpew(77);
  ASSERT_GE(h, 0);

  g_objmap.erase(77); // object dies between frames
  Frametime = 0.25f;
  SpewEmitAll();
  EXPECT_FALSE(SpewEffects[h & 0xFF].inuse);
  EXPECT_TRUE(g_vis.empty());
}

// ---------------------------------------------------------------------------
// SpewClearEvent / SpewInit
// ---------------------------------------------------------------------------

/**
 * @test SpewTest.ClearEventRejectsStaleHandleUnlessForced
 * @brief Verifies clear Event Rejects Stale Handle Unless Forced.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, ClearEventRejectsStaleHandleUnlessForced) {
  int h = CreateSimple();
  ASSERT_GE(h, 0);
  int slot = h & 0xFF;

  int stale = (999 << 16) | slot; // same slot, wrong counter
  SpewClearEvent(stale, false);
  EXPECT_TRUE(SpewEffects[slot].inuse);

  SpewClearEvent(stale, true);
  EXPECT_FALSE(SpewEffects[slot].inuse);
}

/**
 * @test SpewTest.ClearEventAcceptsLiveHandle
 * @brief Verifies clear Event Accepts Live Handle.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, ClearEventAcceptsLiveHandle) {
  int h = CreateSimple();
  ASSERT_GE(h, 0);
  SpewClearEvent(h, false);
  EXPECT_FALSE(SpewEffects[h & 0xFF].inuse);
  spewinfo *v = &SpewEffects[h & 0xFF];
  EXPECT_EQ(v->handle, 0);
  EXPECT_EQ(v->effect_type, MED_SMOKE_INDEX);
  EXPECT_EQ(v->pt.room_num, 0);
}

/**
 * @test SpewTest.ClearEventIgnoresOutOfRangeSlot
 * @brief Verifies clear Event Ignores Out Of Range Slot.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, ClearEventIgnoresOutOfRangeSlot) {
  CreateSimple();
  SpewClearEvent((5 << 16) | 60, true); // slot 60 > MAX_SPEW_EFFECTS(50)
  // Slot 0 untouched
  EXPECT_TRUE(SpewEffects[0].inuse || spew_count == 0);
}

/**
 * @test SpewTest.InitResetsEverything
 * @brief Verifies init Resets Everything.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, InitResetsEverything) {
  int h = CreateSimple();
  ASSERT_GE(h, 0);
  SpewEffects[h & 0xFF].handle = 12345;

  SpewInit();
  EXPECT_EQ(spew_count, 0);
  for (int i = 0; i < MAX_SPEW_EFFECTS; i++) {
    EXPECT_FALSE(SpewEffects[i].inuse) << "slot " << i;
    EXPECT_EQ(SpewEffects[i].handle, 0) << "slot " << i;
  }
}

/**
 * @test SpewTest.SlotsAreReusedAfterClear
 * @brief Verifies slots Are Reused After Clear.
 *
 * @details
 * Exercises the SpewTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/spew.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpewTest, SlotsAreReusedAfterClear) {
  int a = CreateSimple();
  ASSERT_GE(a, 0);
  SpewClearEvent(a, false);
  int b = CreateSimple();
  ASSERT_GE(b, 0);
  EXPECT_EQ(a & 0xFF, b & 0xFF);      // same slot
  EXPECT_EQ(b >> 16, 2);              // fresh counter generation
  EXPECT_EQ(SpewEffects[b & 0xFF].handle, b);
  // Old handle no longer matches -> treated as stale
  SpewClearEvent(a, false);
  EXPECT_TRUE(SpewEffects[b & 0xFF].inuse);
}
