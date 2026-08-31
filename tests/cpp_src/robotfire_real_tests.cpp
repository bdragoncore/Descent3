/**
 * @file robotfire_real_tests.cpp
 * @brief Robot/AI weapon-battery system tests (Descent3/robotfire.cpp).
 *
 * @details
 * Links the real robotfire.cpp. World tables (Objects, Weapons, Players,
 * Ships, Object_info, Poly_models, Sounds) are defined here as zeroed
 * fixtures sized by their engine macros; weapon spawning, sound playback,
 * physics forces and network/demo senders are stubbed with captures.
 * Fixture objects live inside Objects[] so OBJNUM() arithmetic stays valid.
 *
 * This harness validates the behavior of `Descent3/robotfire.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/robotfire.cpp`
 * @par Harness
 * `robotfire_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/robotfire.cpp
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "AIMain.h"
#include "SmallViews.h"
#include "config.h"
#include "demofile.h"
#include "game.h"
#include "hlsoundlib.h"
#include "log.h"
#include "multi.h"
#include "objinfo.h"
#include "object.h"
#include "physics.h"
#include "player.h"
#include "polymodel.h"
#include "pserror.h"
#include "psrand.h"
#include "robotfire.h"
#include "ship.h"
#include "sounds.h"
#include "weapon.h"

// ---------------------------------------------------------------------------
// Globals normally defined in other translation units
// ---------------------------------------------------------------------------
object Objects[MAX_OBJECTS];
weapon Weapons[MAX_WEAPONS];
player Players[MAX_PLAYERS];
ship Ships[MAX_SHIPS];
object_info Object_info[MAX_OBJECT_IDS];
poly_model Poly_models[MAX_POLY_MODELS];
sound_info Sounds[MAX_SOUNDS];

float Gametime = 0.0f;
float Frametime = 0.0f;
int Game_mode = 0;
int Player_num = 0;
object *Viewer_object = nullptr;
uint32_t Demo_flags = 0;
int Buddy_handle[MAX_PLAYERS] = {};
int Missile_camera_window = -1;
netgame_info Netgame = {};
tGameToggles Game_toggles = {};

static uint32_t g_ps_next = 1;
void ps_srand(uint32_t seed) { g_ps_next = seed; }
int32_t ps_rand(void) {
  g_ps_next = g_ps_next * 214013u + 2531011u;
  return static_cast<int32_t>((g_ps_next >> 16) & 0x7fff);
}

// ---------------------------------------------------------------------------
// Stubs with capture state
// ---------------------------------------------------------------------------
struct FireRec {
  object *obj;
  int weapon;
  int gun;
  bool fvec;
  bool target;
};
static std::vector<FireRec> g_fires;
static std::vector<int> g_fire_results; // scripted return values, else -1

int FireWeaponFromObject(object *obj, int weapon_num, int gun_num, bool f_force_forward,
                         bool f_fire_target) {
  g_fires.push_back({obj, weapon_num, gun_num, f_force_forward, f_fire_target});
  if (g_fire_results.empty())
    return -1;
  int r = g_fire_results.front();
  g_fire_results.erase(g_fire_results.begin());
  return r;
}

static std::vector<vector> g_forces;
void phys_apply_force(object *, vector *force_vec, int16_t) { g_forces.push_back(*force_vec); }

bool AINotify(object *, uint8_t, void *) { return true; }

static std::vector<std::pair<int, int>> g_robot_fire_sounds;
void MultiSendRobotFireSound(int16_t soundidx, uint16_t objnum) {
  g_robot_fire_sounds.push_back({soundidx, objnum});
}

void MultiSendFirePlayerWB(int, uint8_t, uint8_t, uint8_t, float) {}

static int g_demo_sounds = 0;
void DemoWrite3DSound(int16_t, uint16_t, int, float) { g_demo_sounds++; }

static std::vector<int> g_released_guided;
void ReleaseGuidedMissile(int slot) { g_released_guided.push_back(slot); }

static std::vector<int> g_released_timeout;
void ReleaseUserTimeoutMissile(int slot) { g_released_timeout.push_back(slot); }

struct SmallViewRec {
  int window;
  int objhandle;
  int flags;
};
static std::vector<SmallViewRec> g_smallviews;
int CreateSmallView(int window, int objhandle, int flags, float, float, int, const char *) {
  g_smallviews.push_back({window, objhandle, flags});
  return 1;
}

static int g_objget_calls = 0;
object *ObjGet(int) {
  g_objget_calls++;
  return nullptr;
}

const char *GetStringFromTable(int) { return "guided"; }

// Inventory lives inside the player struct; its real implementation is not
// linked, so provide the ctor/dtor required by the Players[] array.
Inventory::Inventory() {}
Inventory::~Inventory() {}

// Fake hlsSystem pieces referenced by robotfire.cpp
struct Play3dRec {
  int sound;
  int priority;
  vector pos;
  float volume;
  int flags;
};
static std::vector<Play3dRec> g_play3d;

hlsSystem Sound_system;
hlsSystem::hlsSystem() {
  m_ll_sound_ptr = nullptr;
  m_master_volume = 1.0f;
  m_sounds_played = 0;
  m_pause_new = false;
  m_cur_environment = 0;
  n_lls_sounds = 0;
}
void hlsSystem::KillSoundLib(bool) {}
int hlsSystem::Play3dSound(int sound_index, int priority, pos_state *cur_pos, float volume,
                           int flags, float offset) {
  g_play3d.push_back({sound_index, priority, *cur_pos->position, volume, flags});
  return 1;
}

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for RobotFireTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class RobotFireTest : public ::testing::Test {
protected:
  static constexpr int ROBOT_ID = 5;
  static constexpr int SLOT_A = 100; // Objects[] slot for the robot/player under test
  static constexpr int WB = 0;

  dynamic_wb_info dwb[MAX_WBS_PER_OBJ];
  poly_wb_info pwb[2];
  ai_frame aif{};
  otype_wb_info swb_table[MAX_WBS_PER_OBJ];

  void SetUp() override {
    memset(Objects, 0, sizeof(object) * 16);
    memset(&Players[0], 0, sizeof(player));
    memset(&Ships[0], 0, sizeof(ship));
    memset(&Netgame, 0, sizeof(Netgame));
    memset(&Weapons[0], 0, sizeof(weapon) * 40);

    g_fires.clear();
    g_fire_results.clear();
    g_forces.clear();
    g_robot_fire_sounds.clear();
    g_demo_sounds = 0;
    g_released_guided.clear();
    g_released_timeout.clear();
    g_smallviews.clear();
    g_play3d.clear();
    g_ps_next = 1;

    Gametime = 10.0f;
    Frametime = 0.0f;
    Game_mode = 0;
    Player_num = 0;
    Viewer_object = nullptr;
    Demo_flags = 0;
    Missile_camera_window = -1;

    // Robot object in Objects[]
    object &robj = Objects[SLOT_A];
    robj = object{};
    robj.type = OBJ_ROBOT;
    robj.id = ROBOT_ID;
    robj.roomnum = 0;
    robj.control_type = CT_AI;
    robj.rtype.pobj_info.model_num = 0;
    robj.dynamic_wb = dwb;
    aif = ai_frame{};
    robj.ai_info = &aif;
    WBClearInfo(&robj);

    // Polymodel with two banks of 8 gunpoints
    Poly_models[0] = poly_model{};
    Poly_models[0].num_wbs = 2;
    Poly_models[0].poly_wb = pwb;
    for (auto &w : pwb) {
      w.num_gps = 8;
      w.num_turrets = 0;
      for (int g = 0; g < MAX_WB_GUNPOINTS; g++)
        w.gp_index[g] = g;
    }

    // Static bank table for this robot type
    Object_info[ROBOT_ID].static_wb = swb_table;
    WBClearInfo(swb_table);
    auto &swb = Object_info[ROBOT_ID].static_wb[WB];
    swb.num_masks = 2;
    swb.gp_fire_masks[0] = 0b00000001;
    swb.gp_fire_masks[1] = 0b00000010;
    swb.gp_weapon_index[0] = 3;
    swb.gp_weapon_index[1] = 3;
    swb.fm_fire_sound_index[0] = 9;
    swb.fm_fire_sound_index[1] = 9;
    swb.gp_fire_wait[0] = 1.0f;
    swb.gp_fire_wait[1] = 1.0f;

    // Player table entry
    Players[0].ship_index = 0;
    Players[0].weapon[PW_PRIMARY].index = WB;
    Players[0].weapon_recharge_scalar = 1.0f;
    Players[0].guided_obj = nullptr;
    Players[0].user_timeout_obj = nullptr;
    WBClearInfo(Ships[0].static_wb);
  }

  object &Rob() { return Objects[SLOT_A]; }
  otype_wb_info &Swb() { return Object_info[ROBOT_ID].static_wb[WB]; }
};

// ---------------------------------------------------------------------------
// WBClearInfo
// ---------------------------------------------------------------------------
/**
 * @test RobotFireTest.StaticWBClearDefaults
 * @brief Verifies static WBClear Defaults.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, StaticWBClearDefaults) {
  WBClearInfo(static_cast<otype_wb_info *>(nullptr)); // NULL-safe no-op

  otype_wb_info table[MAX_WBS_PER_OBJ];
  memset(table, 0xAB, sizeof(table));
  WBClearInfo(table);

  for (int i = 0; i < MAX_WBS_PER_OBJ; i++) {
    EXPECT_EQ(table[i].num_masks, 1);
    EXPECT_EQ(table[i].gp_fire_masks[0], 1); // artist-proof default: first gun enabled
    for (int j = 1; j < MAX_WB_GUNPOINTS; j++)
      EXPECT_EQ(table[i].gp_fire_masks[j], 0);
    for (int j = 0; j < MAX_WB_GUNPOINTS; j++)
      EXPECT_EQ(table[i].gp_weapon_index[j], LASER_INDEX);
    EXPECT_EQ(table[i].fm_fire_sound_index[0], SOUND_NONE_INDEX);
    EXPECT_FLOAT_EQ(table[i].gp_fire_wait[0], 1.0f);
    EXPECT_FLOAT_EQ(table[i].aiming_3d_dot, 0.9f);
    EXPECT_FLOAT_EQ(table[i].aiming_3d_dist, 1000.0f);
    EXPECT_EQ(table[i].flags, 0);
  }
}

/**
 * @test RobotFireTest.ObjectWBClearResetsBanks
 * @brief Verifies object WBClear Resets Banks.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, ObjectWBClearResetsBanks) {
  Rob().dynamic_wb[WB].last_fire_time = 5.0f;
  Rob().dynamic_wb[WB].cur_firing_mask = 1;
  Rob().dynamic_wb[WB].flags = 0;

  WBClearInfo(&Rob());

  EXPECT_FLOAT_EQ(Rob().dynamic_wb[WB].last_fire_time, -1.0f);
  EXPECT_EQ(Rob().dynamic_wb[WB].cur_firing_mask, 0u);
  EXPECT_EQ(Rob().dynamic_wb[WB].flags, DWBF_ENABLED | DWBF_AUTOMATIC);
  EXPECT_EQ(Rob().dynamic_wb[WB].turret_direction[0], WB_MOVE_STILL);

  // Player variant clears all MAX_WBS_PER_OBJ banks regardless of model
  object pobj = object{};
  pobj.type = OBJ_PLAYER;
  dynamic_wb_info pdwb[MAX_WBS_PER_OBJ];
  pobj.dynamic_wb = pdwb;
  pdwb[MAX_WBS_PER_OBJ - 1].last_fire_time = 42.0f;
  WBClearInfo(&pobj);
  EXPECT_FLOAT_EQ(pdwb[MAX_WBS_PER_OBJ - 1].last_fire_time, -1.0f);
}

// ---------------------------------------------------------------------------
// WBEnable
// ---------------------------------------------------------------------------
/**
 * @test RobotFireTest.WBEnableTogglesAndBounds
 * @brief Verifies wBEnable Toggles And Bounds.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, WBEnableTogglesAndBounds) {
  WBEnable(&Rob(), WB, false);
  EXPECT_EQ(Rob().dynamic_wb[WB].flags & DWBF_ENABLED, 0);
  WBEnable(&Rob(), WB, true);
  EXPECT_NE(Rob().dynamic_wb[WB].flags & DWBF_ENABLED, 0);

  // Bank beyond the model's num_wbs is untouched for robots
  Rob().dynamic_wb[15].flags |= DWBF_ENABLED;
  WBEnable(&Rob(), 15, false);
  EXPECT_NE(Rob().dynamic_wb[15].flags & DWBF_ENABLED, 0);

  // Players can toggle any bank up to MAX_WBS_PER_OBJ
  object pobj = object{};
  pobj.type = OBJ_PLAYER;
  dynamic_wb_info pdwb[MAX_WBS_PER_OBJ];
  pobj.dynamic_wb = pdwb;
  WBClearInfo(&pobj);
  WBEnable(&pobj, 20, false);
  EXPECT_EQ(pdwb[20].flags & DWBF_ENABLED, 0);

  // Negative index fans out over every model bank
  WBEnable(&Rob(), -1, false);
  EXPECT_EQ(Rob().dynamic_wb[0].flags & DWBF_ENABLED, 0);
  EXPECT_EQ(Rob().dynamic_wb[1].flags & DWBF_ENABLED, 0);
}

// ---------------------------------------------------------------------------
// WBIsBatteryReady
// ---------------------------------------------------------------------------
/**
 * @test RobotFireTest.BatteryReadyTimingBoundary
 * @brief Verifies battery Ready Timing Boundary.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, BatteryReadyTimingBoundary) {
  // Cleared state fires immediately
  Gametime = 10.0f;
  EXPECT_TRUE(WBIsBatteryReady(&Rob(), &Swb(), WB));

  Rob().dynamic_wb[WB].last_fire_time = 9.0f;
  Swb().gp_fire_wait[0] = 2.0f;

  Gametime = 10.999f; // 9 + 2 = 11 not reached
  EXPECT_FALSE(WBIsBatteryReady(&Rob(), &Swb(), WB));
  Gametime = 11.0f; // strict '<': exactly at recharge time still blocked
  EXPECT_FALSE(WBIsBatteryReady(&Rob(), &Swb(), WB));
  Gametime = 11.001f;
  EXPECT_TRUE(WBIsBatteryReady(&Rob(), &Swb(), WB));
}

/**
 * @test RobotFireTest.BatteryReadyScalarAndQuadPenalty
 * @brief Verifies battery Ready Scalar And Quad Penalty.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, BatteryReadyScalarAndQuadPenalty) {
  Swb().gp_fire_wait[0] = 2.0f;

  // Player recharge scalar halves the wait: 9 + 2*0.5 = 10
  Rob().type = OBJ_PLAYER;
  Rob().id = 0;
  Players[0].weapon_recharge_scalar = 0.5f;
  Rob().dynamic_wb[WB].last_fire_time = 9.0f;

  Gametime = 10.0f;
  EXPECT_FALSE(WBIsBatteryReady(&Rob(), &Swb(), WB));
  Gametime = 10.01f;
  EXPECT_TRUE(WBIsBatteryReady(&Rob(), &Swb(), WB));

  // Quad adds 25% of the current-mask wait: 9 + 1 + 0.5 = 10.5
  Rob().dynamic_wb[WB].flags |= DWBF_QUAD;
  Gametime = 10.4f;
  EXPECT_FALSE(WBIsBatteryReady(&Rob(), &Swb(), WB));
  Gametime = 10.6f;
  EXPECT_TRUE(WBIsBatteryReady(&Rob(), &Swb(), WB));
}

/**
 * @test RobotFireTest.BatteryReadyAnimatingBlocks
 * @brief Verifies battery Ready Animating Blocks.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, BatteryReadyAnimatingBlocks) {
  Rob().dynamic_wb[WB].last_fire_time = -1.0f;
  Rob().dynamic_wb[WB].flags |= DWBF_ANIMATING;
  EXPECT_FALSE(WBIsBatteryReady(&Rob(), &Swb(), WB));
}

// ---------------------------------------------------------------------------
// WBFireBattery
// ---------------------------------------------------------------------------
/**
 * @test RobotFireTest.FailedFiresAdvanceMaskWithoutSound
 * @brief Verifies failed Fires Advance Mask Without Sound.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, FailedFiresAdvanceMaskWithoutSound) {
  // mask0 = gun0 only -> attempt gun0; failed spawn (-1) => num_fired == 0
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);

  ASSERT_EQ(g_fires.size(), 1u);
  EXPECT_EQ(g_fires[0].gun, 0);
  EXPECT_EQ(g_fires[0].weapon, 3);
  EXPECT_TRUE(g_play3d.empty());

  // last_fire_time stamped even on failure; mask advanced 0 -> 1
  EXPECT_FLOAT_EQ(Rob().dynamic_wb[WB].last_fire_time, 10.0f);
  EXPECT_EQ(Rob().dynamic_wb[WB].cur_firing_mask, 1u);

  // Next call uses mask1 (gun1), then wraps back to mask0
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  ASSERT_EQ(g_fires.size(), 2u);
  EXPECT_EQ(g_fires[1].gun, 1);
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  // Cycle is 0 -> 1 -> 0 -> 1, so after three calls the next mask is 1
  EXPECT_EQ(Rob().dynamic_wb[WB].cur_firing_mask, 1u);
  EXPECT_TRUE(g_play3d.empty());
}

/**
 * @test RobotFireTest.SuccessfulFireAveragesPositionsAndPlaysSound
 * @brief Verifies successful Fire Averages Positions And Plays Sound.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, SuccessfulFireAveragesPositionsAndPlaysSound) {
  Swb().gp_fire_masks[0] = 0b00000101; // guns 0 and 2
  g_fire_results = {5, 6};
  Objects[5].pos = {1, 0, 0};
  Objects[6].pos = {3, 0, 4};

  WBFireBattery(&Rob(), &Swb(), WB, WB, 2.5f);

  ASSERT_EQ(g_fires.size(), 2u);
  EXPECT_EQ(g_fires[0].gun, 0);
  EXPECT_EQ(g_fires[1].gun, 2);
  EXPECT_EQ(Objects[5].ctype.laser_info.multiplier, 2.5f);
  EXPECT_EQ(Objects[6].ctype.laser_info.multiplier, 2.5f);

  ASSERT_EQ(g_play3d.size(), 1u);
  EXPECT_EQ(g_play3d[0].sound, 9);
  EXPECT_EQ(g_play3d[0].priority, SND_PRIORITY_HIGHEST);
  EXPECT_NEAR(g_play3d[0].pos.x(), 2.0f, 0.001f); // average of spawn positions
  EXPECT_NEAR(g_play3d[0].pos.z(), 2.0f, 0.001f);
}

/**
 * @test RobotFireTest.QuadFlagUsesQuadMask
 * @brief Verifies quad Flag Uses Quad Mask.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, QuadFlagUsesQuadMask) {
  Rob().dynamic_wb[WB].flags |= DWBF_QUAD;
  Swb().gp_quad_fire_mask = 0b00000100; // gun 2 only
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);

  ASSERT_EQ(g_fires.size(), 1u);
  EXPECT_EQ(g_fires[0].gun, 2);
}

/**
 * @test RobotFireTest.RandomFireOrderPicksMaskInBounds
 * @brief Verifies random Fire Order Picks Mask In Bounds.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, RandomFireOrderPicksMaskInBounds) {
  Swb().flags |= WBF_RANDOM_FIRE_ORDER;
  Swb().num_masks = 3;

  ps_srand(12345);
  uint32_t roll = static_cast<uint32_t>(ps_rand()); // probe the first roll
  int expected = static_cast<int>((static_cast<float>(roll) / static_cast<float>(D3_RAND_MAX)) *
                                  Swb().num_masks);
  if (expected >= static_cast<int>(Swb().num_masks))
    expected = 0;

  ps_srand(12345); // rewind so the battery consumes that same roll
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  EXPECT_EQ(Rob().dynamic_wb[WB].cur_firing_mask, expected);
}

/**
 * @test RobotFireTest.AIWeaponSpeedDerivedFromWeaponPhysics
 * @brief Verifies aIWeapon Speed Derived From Weapon Physics.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, AIWeaponSpeedDerivedFromWeaponPhysics) {
  // After firing, the new mask's first-bit weapon sets ai weapon_speed.
  Swb().gp_fire_masks[0] = 0b00000001;
  Swb().gp_fire_masks[1] = 0b00000001;

  // FIXED_VELOCITY: speed = velocity.z
  Weapons[3].phys_info.flags = PF_FIXED_VELOCITY;
  Weapons[3].phys_info.velocity.z() = 42.0f;
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  EXPECT_FLOAT_EQ(Rob().ai_info->weapon_speed, 42.0f);

  // USES_THRUST: speed = full_thrust / drag
  WBClearInfo(&Rob());
  Weapons[3].phys_info.flags = PF_USES_THRUST;
  Weapons[3].phys_info.full_thrust = 100.0f;
  Weapons[3].phys_info.drag = 4.0f;
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  EXPECT_FLOAT_EQ(Rob().ai_info->weapon_speed, 25.0f);

  // Neither flag: speed forced to zero
  WBClearInfo(&Rob());
  Weapons[3].phys_info.flags = 0;
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  EXPECT_FLOAT_EQ(Rob().ai_info->weapon_speed, 0.0f);
}

/**
 * @test RobotFireTest.GuidedMissileLifecycle
 * @brief Verifies guided Missile Lifecycle.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, GuidedMissileLifecycle) {
  Rob().type = OBJ_PLAYER;
  Rob().id = 0;
  Swb().flags |= WBF_GUIDED;
  g_fire_results = {7};

  // Local player without guided mainview gets the missile camera popup
  Game_toggles.guided_mainview = false;
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);

  ASSERT_EQ(g_fires.size(), 1u);
  EXPECT_EQ(Players[0].guided_obj, &Objects[7]);
  EXPECT_NE(Objects[7].mtype.phys_info.flags & (PF_HOMING | PF_GUIDED), 0u);
  ASSERT_EQ(g_smallviews.size(), 1u);
  EXPECT_EQ(g_smallviews[0].window, SVW_LEFT); // Missile_camera_window == -1
  EXPECT_EQ(g_smallviews[0].flags, SVF_POPUP | SVF_BIGGER | SVF_CROSSHAIRS);

  // Firing again releases the previous guided missile first
  g_fire_results = {8};
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  ASSERT_EQ(g_released_guided.size(), 1u);
  EXPECT_EQ(g_released_guided[0], 0); // player slot
  EXPECT_EQ(Players[0].guided_obj, &Objects[8]);
}

/**
 * @test RobotFireTest.GuidedMainviewSuppressedWhenToggleSet
 * @brief Verifies guided Mainview Suppressed When Toggle Set.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, GuidedMainviewSuppressedWhenToggleSet) {
  Rob().type = OBJ_PLAYER;
  Rob().id = 0;
  Swb().flags |= WBF_GUIDED;
  Game_toggles.guided_mainview = true;
  g_fire_results = {7};

  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  EXPECT_EQ(Players[0].guided_obj, &Objects[7]);
  EXPECT_TRUE(g_smallviews.empty());
}

/**
 * @test RobotFireTest.UserTimeoutMissileStored
 * @brief Verifies user Timeout Missile Stored.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, UserTimeoutMissileStored) {
  Rob().type = OBJ_PLAYER;
  Rob().id = 0;
  Swb().flags |= WBF_USER_TIMEOUT;
  g_fire_results = {7};

  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  EXPECT_EQ(Players[0].user_timeout_obj, &Objects[7]);
}

/**
 * @test RobotFireTest.RecoilAppliedOnceOnFirstGun
 * @brief Verifies recoil Applied Once On First Gun.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, RecoilAppliedOnceOnFirstGun) {
  Rob().type = OBJ_PLAYER;
  Rob().id = Player_num;
  Swb().gp_fire_masks[0] = 0b00000101; // two guns
  g_fire_results = {5, 6};
  Weapons[3].recoil_force = 3.0f;
  Rob().orient.fvec = {0, 0, 1};

  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);

  ASSERT_EQ(g_forces.size(), 1u); // only the first gun kicks
  EXPECT_NEAR(g_forces[0].z(), -3.0f, 0.001f);
  EXPECT_NEAR(g_forces[0].x(), 0.0f, 0.001f);
}

/**
 * @test RobotFireTest.ViewerHearsNoEffectVariant
 * @brief Verifies viewer Hears No Effect Variant.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, ViewerHearsNoEffectVariant) {
  Viewer_object = &Rob();
  g_fire_results = {5, 6}; // one successful spawn per fire call

  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  ASSERT_EQ(g_play3d.size(), 1u);
  EXPECT_EQ(g_play3d[0].flags, SIF_NO_3D_EFFECTS);
  EXPECT_FLOAT_EQ(g_play3d[0].volume, MAX_GAME_VOLUME);

  // Non-viewer objects use the plain positional variant (flags == 0)
  g_play3d.clear();
  Viewer_object = nullptr;
  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  ASSERT_EQ(g_play3d.size(), 1u);
  EXPECT_EQ(g_play3d[0].flags, 0);
}

/**
 * @test RobotFireTest.DemoRecordingWritesSound
 * @brief Verifies demo Recording Writes Sound.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, DemoRecordingWritesSound) {
  Demo_flags = DF_RECORDING;
  g_fire_results = {5};

  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  EXPECT_EQ(g_demo_sounds, 1);
}

/**
 * @test RobotFireTest.MultiServerRelaysRobotFireSound
 * @brief Verifies multi Server Relays Robot Fire Sound.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, MultiServerRelaysRobotFireSound) {
  Game_mode = GM_MULTI;
  Netgame.local_role = LR_SERVER;
  g_fire_results = {5};

  WBFireBattery(&Rob(), &Swb(), WB, WB, 1.0f);
  ASSERT_EQ(g_robot_fire_sounds.size(), 1u);
  EXPECT_EQ(g_robot_fire_sounds[0].first, 9);      // fm_fire_sound_index
  EXPECT_EQ(g_robot_fire_sounds[0].second, SLOT_A); // OBJNUM
}

// ---------------------------------------------------------------------------
// FireOnOffWeapon
// ---------------------------------------------------------------------------
/**
 * @test RobotFireTest.FireOnOffWeaponRejectsBadWbIndex
 * @brief Verifies fire On Off Weapon Rejects Bad Wb Index.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, FireOnOffWeaponRejectsBadWbIndex) {
  Rob().ai_info->last_special_wb_firing = 50; // > MAX_WBS_PER_OBJ
  FireOnOffWeapon(&Rob());
  EXPECT_TRUE(g_fires.empty());
  EXPECT_EQ(Rob().dynamic_wb[WB].cur_firing_mask, 0u);
}

/**
 * @test RobotFireTest.FireOnOffWeaponFiresSpecialBankWhenReady
 * @brief Verifies fire On Off Weapon Fires Special Bank When Ready.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, FireOnOffWeaponFiresSpecialBankWhenReady) {
  // Configure bank 1 (the special-firing bank): two masks, weapon 3.
  auto &swb1 = Object_info[ROBOT_ID].static_wb[1];
  swb1.num_masks = 2;
  swb1.gp_fire_masks[0] = 0b00000001;
  swb1.gp_fire_masks[1] = 0b00000010;
  swb1.gp_weapon_index[0] = 3;
  swb1.gp_weapon_index[1] = 3;

  Rob().ai_info->last_special_wb_firing = 1;
  FireOnOffWeapon(&Rob());

  ASSERT_EQ(g_fires.size(), 1u);
  EXPECT_EQ(g_fires[0].weapon, 3);
  // Bank 1's mask advanced (started 0 -> 1)
  EXPECT_EQ(Rob().dynamic_wb[1].cur_firing_mask, 1u);
  EXPECT_FLOAT_EQ(Rob().dynamic_wb[1].last_fire_time, 10.0f);
}

/**
 * @test RobotFireTest.FireOnOffWeaponSkipsWhenNotReady
 * @brief Verifies fire On Off Weapon Skips When Not Ready.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, FireOnOffWeaponSkipsWhenNotReady) {
  Rob().ai_info->last_special_wb_firing = 1;
  Rob().dynamic_wb[1].last_fire_time = Gametime; // just fired
  FireOnOffWeapon(&Rob());
  EXPECT_TRUE(g_fires.empty());
}

/**
 * @test RobotFireTest.FireOnOffWeaponPlayerUsesShipBank
 * @brief Verifies fire On Off Weapon Player Uses Ship Bank.
 *
 * @details
 * Exercises the RobotFireTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/robotfire.cpp
 * @ingroup descent3_tests
 */
TEST_F(RobotFireTest, FireOnOffWeaponPlayerUsesShipBank) {
  Rob().type = OBJ_PLAYER;
  Rob().id = 0;

  // The SHIP bank fires gun 1 while the Object_info table would fire gun 0,
  // so the fired gun proves which table was consulted.
  Ships[0].static_wb[WB].gp_fire_masks[0] = 0b00000010; // gun 1
  Players[0].weapon[PW_PRIMARY].index = WB;

  FireOnOffWeapon(&Rob());

  ASSERT_EQ(g_fires.size(), 1u);
  EXPECT_EQ(g_fires[0].gun, 1); // ship bank consulted
  // The ship bank has a single mask (cleared default), so the wrap clamp
  // returns cur_firing_mask to 0; last_fire_time is still stamped.
  EXPECT_EQ(Rob().dynamic_wb[WB].cur_firing_mask, 0u);
  EXPECT_FLOAT_EQ(Rob().dynamic_wb[WB].last_fire_time, 10.0f);
}
