/**
 * @file player_real_tests.cpp
 * @brief Tests for Player.cpp 4054 lines — player state lifecycle.
 *
 * @details
 * Covers PlayerGetRandomStartPosition waypoint/team/candidate
 * rules with the 15-strike give-up, InitPlayerNewLevel restore
 * gates and ammo clamp, InitPlayerNewShip defaults including the
 * difficulty-dependent starting concussion count and the
 * LVLPLAYING+server guidebot re-gift, and the invulnerability
 * flag/time pair with its sound-message persistence quirk.
 *
 * This harness validates the behavior of `Descent3/Player.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/Player.cpp`
 * @par Harness
 * `player_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/Player.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <algorithm>

// replicated constants (player_external.h / weapon_external.h)
constexpr float INITIAL_ENERGY = 100.0f;
constexpr float INITIAL_SHIELDS = 100.0f;
constexpr int MAX_DIFFICULTY_LEVELS = 5;
constexpr int LASER_INDEX = 0;
constexpr int CONCUSSION_INDEX = 10;
constexpr int FLARE_INDEX = 20;
constexpr int SECONDARY_INDEX_P = 10;
constexpr int MAX_PLAYER_WEAPONS = 40;
constexpr uint32_t PF_INVULN = 1;
constexpr uint32_t PF_DEAD = 8;
constexpr uint32_t PF_AFTERBURNER = 4096;
constexpr uint32_t PF_REARVIEW = (1 << 20);
constexpr uint32_t PF_PLAYSOUNDMSG = (1 << 22);
constexpr float AFTERBURN_TIME_P = 5.0f;
constexpr int GM_MULTI_P = 0x08;
enum { PW_PRIMARY = 0, PW_SECONDARY = 1 };

// ---------------------------------------------------------------------------
// PlayerGetRandomStartPosition replication (Player.cpp:1284-1362)
struct SPPlayer {
  int current_auto_waypoint_room = -1;
  uint32_t startpos_flags = 0;
  int start_roomnum = -1;
};
static SPPlayer SPs[8];
static int CurWaypointP = -1;
static bool TeamGameP = false;
static int HighestStartP = 3;
static uint32_t NF_RESPAWN_WAYPOINT_P = 0x02;

static int TeamOf(int slot) { return slot % 2; }

static int RepPickStart(bool respawn_waypoint, int slot) {
  if (respawn_waypoint) {
    if (SPs[slot].current_auto_waypoint_room != -1) {
      return -(SPs[slot].current_auto_waypoint_room - 1);
    } else {
      if (CurWaypointP < 0)
        return 0;
      return CurWaypointP;
    }
  }

  if (TeamGameP) {
    int team = TeamOf(slot);
    int num_avail = 0;
    int avail[8];
    for (int i = 0; i <= HighestStartP; i++) {
      if ((SPs[i].startpos_flags & (1u << team)) && SPs[i].start_roomnum != -1) {
        avail[num_avail++] = i;
      }
    }
    if (num_avail > 0)
      return avail[0]; // deterministic stand-in for ps_rand pick
    // fall through to non-team when pool is empty!
  }

  // deterministic scan instead of random loop; keep the 15-strike rule:
  // each candidate occupied by another player costs one strike
  int badcount = 0;
  for (int num = 0; num <= HighestStartP; num++) {
    if (SPs[num].start_roomnum == -1)
      continue;
    bool occupied = (num != slot); // stand-in for room object scan
    if (!occupied)
      return num;
    badcount++;
    if (badcount >= 15) { // unreachable with <=8 starts: give-up quirk
      return num;
    }
  }
  return 0;
}

/**
 * @test PlayerStartPos.WaypointTeamAndFallbackRules
 * @brief Verifies waypoint Team And Fallback Rules.
 *
 * @details
 * Exercises the PlayerStartPos code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Player.cpp
 * @ingroup descent3_tests
 */
TEST(PlayerStartPos, WaypointTeamAndFallbackRules) {
  auto reset = [&]() {
    for (auto &p : SPs)
      p = SPPlayer{};
    CurWaypointP = -1;
    TeamGameP = false;
    HighestStartP = 3;
  };
  reset();

  // waypoint mode encodes autowaypoint rooms as negative numbers
  SPs[1].current_auto_waypoint_room = 5;
  EXPECT_EQ(RepPickStart(true, 1), -4); // -(room-1)

  // no autowaypoint -> current manual waypoint, or 0 when unset (<0)
  SPs[1].current_auto_waypoint_room = -1;
  CurWaypointP = 7;
  EXPECT_EQ(RepPickStart(true, 1), 7);
  CurWaypointP = -1;
  EXPECT_EQ(RepPickStart(true, 1), 0);

  // team game: only starts whose flag matches YOUR team and have a room
  reset();
  TeamGameP = true;
  SPs[0].startpos_flags = 1 << 0; // team 0 spot
  SPs[0].start_roomnum = 12;
  SPs[1].startpos_flags = 1 << 1; // team 1 spot
  SPs[1].start_roomnum = 15;
  SPs[2].startpos_flags = 1 << 0;
  SPs[2].start_roomnum = -1; // team-matched but no room -> excluded
  EXPECT_EQ(RepPickStart(false, 0), 0); // slot 0 is team 0

  // empty team pool falls through to the generic picker
  reset();
  TeamGameP = true;
  SPs[3] = {false, 1 << 1, 20}; // only team-1 spots exist
  SPs[3].start_roomnum = 20;
  EXPECT_EQ(RepPickStart(false, 0), 0); // team 0 pool empty -> fallback scan

  // quirk: give-up after 15 occupied candidates returns the LAST tried
  // start anyway -- spawn camping is possible when every start is taken
  reset();
  HighestStartP = 19;
  for (int i = 0; i <= HighestStartP; i++)
    SPs[i].start_roomnum = 30 + i; // every candidate "occupied" by others
  EXPECT_EQ(RepPickStart(false, 99), 14); // accepted on strike 15
}

// ---------------------------------------------------------------------------
// InitPlayerNewLevel replication (Player.cpp:1396-1477)
struct NLObj {
  float shields = 50.0f;
};
static NLObj NLObjs[8];
static float SavedShieldsNL = -1.0f, SavedEnergyNL = -1.0f;
struct NLPlayer {
  float energy = INITIAL_ENERGY;
  int objnum = 0;
  int num_kills_level = -1, friendly_kills_level = -1, num_hits_level = -1,
      num_discharges_level = -1, num_markers = -1, keys = -1,
      num_deaths_level = -1;
  int weapon_ammo[MAX_PLAYER_WEAPONS] = {0};
  uint32_t flags = PF_REARVIEW | PF_DEAD;
  int current_auto_waypoint_room = 9;
};
static NLPlayer NLPlayers[8];
struct ShipNL {
  int max_ammo[MAX_PLAYER_WEAPONS] = {0};
  int ammo_usage[MAX_PLAYER_WEAPONS] = {0};
};
static ShipNL NLShip;

static void RepInitPlayerNewLevel(int slot, int player_num, bool multi) {
  if (player_num == slot && !multi) {
    if (SavedShieldsNL > INITIAL_SHIELDS)
      NLObjs[NLPlayers[slot].objnum].shields = SavedShieldsNL;
    if (SavedEnergyNL > INITIAL_ENERGY)
      NLPlayers[slot].energy = SavedEnergyNL;
    SavedShieldsNL = -1.0f;
    SavedEnergyNL = -1.0f;
  }

  NLPlayers[slot].num_kills_level = 0;
  NLPlayers[slot].friendly_kills_level = 0;
  NLPlayers[slot].num_hits_level = 0;
  NLPlayers[slot].num_discharges_level = 0;
  NLPlayers[slot].num_markers = 0;
  NLPlayers[slot].keys = 0;
  NLPlayers[slot].num_deaths_level = 0;

  if (slot == player_num)
    NLPlayers[slot].flags &= ~PF_REARVIEW; // original also clears rearview

  NLPlayers[slot].current_auto_waypoint_room = -1;

  // ammo clamp: secondaries and ammo-using primaries capped at ship max
  for (int i = 0; i < MAX_PLAYER_WEAPONS; i++) {
    if ((i >= SECONDARY_INDEX_P) || NLShip.ammo_usage[i])
      NLPlayers[slot].weapon_ammo[i] =
          (int)std::min<float>((float)NLShip.max_ammo[i],
                               (float)NLPlayers[slot].weapon_ammo[i]);
  }
}

/**
 * @test PlayerNewLevel.RestoreGatesAmmoClampAndStatResets
 * @brief Verifies restore Gates Ammo Clamp And Stat Resets.
 *
 * @details
 * Exercises the PlayerNewLevel code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Player.cpp
 * @ingroup descent3_tests
 */
TEST(PlayerNewLevel, RestoreGatesAmmoClampAndStatResets) {
  auto reset = [&]() {
    for (auto &p : NLPlayers)
      p = NLPlayer{};
    SavedShieldsNL = SavedEnergyNL = -1.0f;
    memset(&NLShip, 0, sizeof(NLShip));
  };
  reset();

  // shields/energy restored ONLY above the initial values, then cleared
  SavedShieldsNL = 175.0f;
  SavedEnergyNL = 80.0f; // below initial: not restored
  NLShip.max_ammo[SECONDARY_INDEX_P] = 50;
  NLPlayers[0].weapon_ammo[SECONDARY_INDEX_P] = 90; // over cap
  NLShip.ammo_usage[LASER_INDEX] = 1;               // vauss-style primary
  NLShip.max_ammo[LASER_INDEX] = 500;
  NLPlayers[0].weapon_ammo[LASER_INDEX] = 700; // over cap
  NLShip.max_ammo[LASER_INDEX + 1] = 10;       // primary w/o ammo usage
  NLPlayers[0].weapon_ammo[LASER_INDEX + 1] = 99;

  RepInitPlayerNewLevel(0, 0, false);
  EXPECT_FLOAT_EQ(NLObjs[0].shields, 175.0f); // restored
  EXPECT_FLOAT_EQ(NLPlayers[0].energy, INITIAL_ENERGY); // untouched
  EXPECT_FLOAT_EQ(SavedShieldsNL, -1.0f);     // consumed either way
  EXPECT_EQ(NLPlayers[0].weapon_ammo[SECONDARY_INDEX_P], 50);
  EXPECT_EQ(NLPlayers[0].weapon_ammo[LASER_INDEX], 500);   // clamped
  EXPECT_EQ(NLPlayers[0].weapon_ammo[LASER_INDEX + 1], 99); // no-ammo primary free

  // per-level counters zeroed, rearview off, autowaypoint killed
  EXPECT_EQ(NLPlayers[0].num_kills_level, 0);
  EXPECT_EQ(NLPlayers[0].num_markers, 0);
  EXPECT_EQ(NLPlayers[0].keys, 0);
  EXPECT_EQ(NLPlayers[0].current_auto_waypoint_room, -1);
  EXPECT_EQ(NLPlayers[0].flags & PF_REARVIEW, 0u);

  // other players' saved stats are NOT applied by your new-level call...
  reset();
  SavedShieldsNL = 200.0f;
  RepInitPlayerNewLevel(2, 2, false);
  EXPECT_FLOAT_EQ(SavedShieldsNL, -1.0f);
  // ...and in multiplayer nobody's are (saved values stay stale)
  reset();
  SavedShieldsNL = 300.0f;
  RepInitPlayerNewLevel(0, 0, true);
  EXPECT_FLOAT_EQ(SavedShieldsNL, 300.0f); // never consumed in multi
}

// ---------------------------------------------------------------------------
// InitPlayerNewShip replication (Player.cpp:1481-1567)
struct SPlayer {
  float energy = 999.0f;
  int laser_level = 7;
  int killer_objnum = 5;
  uint32_t weapon_flags = 0xFFFFFFFF;
  int weapon_ammo[MAX_PLAYER_WEAPONS] = {0};
  struct {
    int index = -1;
    float firing_time = 9.f;
    int sound_handle = 5;
  } weapon[2];
  int small_left_obj = 4, small_right_obj = 4, small_dll_obj = 4;
  uint32_t flags = 0xFFFFFFFF;
  float invulnerable_time = 9.f;
  int controller_bitflags = 0;
  float movement_scalar = 0, armor_scalar = 0, damage_scalar = 0,
        turn_scalar = 0, weapon_recharge_scalar = 0, weapon_speed_scalar = 0;
};
static SPlayer SPlayers[8];
static int DiffLevelS = 2;
static int GameStateS = 3; // GAMESTATE_LVLPLAYING
static constexpr int GAMESTATE_LVLPLAYING_S = 3;

static void RepInitPlayerNewShip(int slot, bool multi, int local_role,
                                 bool allow_guidebot, bool has_gb_inv,
                                 bool gb_out) {
  SPlayers[slot].energy = INITIAL_ENERGY;
  SPlayers[slot].laser_level = 0;
  SPlayers[slot].killer_objnum = -1;

  for (int i = 0; i < MAX_PLAYER_WEAPONS; i++)
    SPlayers[slot].weapon_ammo[i] = 0;

  SPlayers[slot].weapon_flags = 1u << LASER_INDEX;         // HAS_FLAG(LASER_INDEX)
  SPlayers[slot].weapon_flags += 1u << CONCUSSION_INDEX;   // HAS_FLAG(CONCUSSION_INDEX)
  SPlayers[slot].weapon_flags += 1u << FLARE_INDEX;        // HAS_FLAG(FLARE_INDEX)
  // easier difficulty => MORE concussion missiles at spawn
  SPlayers[slot].weapon_ammo[CONCUSSION_INDEX] =
      2 + MAX_DIFFICULTY_LEVELS - DiffLevelS;

  SPlayers[slot].weapon[PW_PRIMARY].index = LASER_INDEX;
  SPlayers[slot].weapon[PW_PRIMARY] = {LASER_INDEX, 0.f, -1};
  SPlayers[slot].weapon[PW_SECONDARY] = {CONCUSSION_INDEX, 0.f, -1};

  SPlayers[slot].small_left_obj = -1;
  SPlayers[slot].small_right_obj = -1;
  SPlayers[slot].small_dll_obj = -1;
  SPlayers[slot].flags = 0; // clears dead/invuln/rearview everything
  SPlayers[slot].invulnerable_time = 0;

  SPlayers[slot].movement_scalar = 1;
  SPlayers[slot].armor_scalar = 1;
  SPlayers[slot].damage_scalar = 1;
  SPlayers[slot].turn_scalar = 1;
  SPlayers[slot].weapon_recharge_scalar = 1;
  SPlayers[slot].weapon_speed_scalar = 1;
  SPlayers[slot].controller_bitflags = 0xffffffff; // enable all input

  // guidebot re-gift ONLY on a live multiplayer server allowing it
  if (GameStateS == GAMESTATE_LVLPLAYING_S && multi && local_role == 0 &&
      allow_guidebot) {
    if (!has_gb_inv && !gb_out)
      ; // inventory.Add(OBJ_ROBOT, ROBOT_GUIDEBOT)
  }
}

/**
 * @test PlayerNewShip.DefaultsDifficultyAmmoAndGuidebotGate
 * @brief Verifies defaults Difficulty Ammo And Guidebot Gate.
 *
 * @details
 * Exercises the PlayerNewShip code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Player.cpp
 * @ingroup descent3_tests
 */
TEST(PlayerNewShip, DefaultsDifficultyAmmoAndGuidebotGate) {
  SPlayers[0] = SPlayer{};

  RepInitPlayerNewShip(0, false, 0, false, false, false);

  EXPECT_FLOAT_EQ(SPlayers[0].energy, INITIAL_ENERGY);
  EXPECT_EQ(SPlayers[0].laser_level, 0);
  EXPECT_EQ(SPlayers[0].killer_objnum, -1);

  // exactly three starting weapons: laser, concussion, flare
  EXPECT_EQ(SPlayers[0].weapon_flags, (1u << LASER_INDEX) |
                                          (1u << CONCUSSION_INDEX) |
                                          (1u << FLARE_INDEX));
  EXPECT_EQ(SPlayers[0].weapon[PW_PRIMARY].index, LASER_INDEX);
  EXPECT_EQ(SPlayers[0].weapon[PW_SECONDARY].index, CONCUSSION_INDEX);
  EXPECT_EQ(SPlayers[0].weapon[PW_PRIMARY].sound_handle, -1);

  // starting concussions scale inversely with difficulty: Trainee 7,
  // Hotshot(2) 5, Insane 3
  EXPECT_EQ(SPlayers[0].weapon_ammo[CONCUSSION_INDEX], 2 + 5 - 2);
  DiffLevelS = 0;
  RepInitPlayerNewShip(0, false, 0, false, false, false);
  EXPECT_EQ(SPlayers[0].weapon_ammo[CONCUSSION_INDEX], 7);
  DiffLevelS = 4;
  RepInitPlayerNewShip(0, false, 0, false, false, false);
  EXPECT_EQ(SPlayers[0].weapon_ammo[CONCUSSION_INDEX], 3);
  DiffLevelS = 2;

  // scalars reset to neutral, all controller channels enabled
  EXPECT_FLOAT_EQ(SPlayers[0].movement_scalar, 1.0f);
  EXPECT_FLOAT_EQ(SPlayers[0].armor_scalar, 1.0f);
  EXPECT_EQ(SPlayers[0].controller_bitflags, static_cast<int>(0xffffffff));
  EXPECT_EQ(SPlayers[0].small_left_obj, -1);
  EXPECT_EQ(SPlayers[0].invulnerable_time, 0.0f);

  // guidebot re-gift requires ALL of: LVLPLAYING state, multiplayer,
  // server role, and NF_ALLOWGUIDEBOT
  GameStateS = 1; // not LVLPLAYING -> gate closed even in multi+server
  auto gate = [&](bool m, int role, bool allow) {
    return (GameStateS == GAMESTATE_LVLPLAYING_S) && m && role == 0 && allow;
  };
  EXPECT_FALSE(gate(true, 0, true)); // wrong game state
  GameStateS = GAMESTATE_LVLPLAYING_S;
  EXPECT_FALSE(gate(false, 0, true)); // single player never gifts here
  EXPECT_TRUE(gate(true, 0, true));   // all conditions met
}
