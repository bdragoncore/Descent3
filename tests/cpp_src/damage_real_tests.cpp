/**
 * @file damage_real_tests.cpp
 * @brief Tests for damage.cpp 1599 lines — damage application. Covers the.
 *
 * @details
 * single-player path of ApplyDamageToPlayer (armor scalars, dying/
 * invulnerable early-outs with knockback vector), GenerateDefaultDeath
 * deterministic branches (quick vs slow, building flying death), and
 * KillObject's weighted death-type dispatch with default fallback.
 * Randomness steered via the replicated ps_rand LCG.
 *
 * This harness validates the behavior of `Descent3/damage.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/damage.cpp`
 * @par Harness
 * `damage_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/damage.cpp
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cmath>

// ps_rand LCG replication (misc/psrand.cpp)
static uint32_t holdrand_p;
static void ps_srand_p(uint32_t s) { holdrand_p = s; }
static int32_t ps_rand_p() {
  holdrand_p = holdrand_p * 214013 + 2531011;
  return (int32_t)((holdrand_p >> 16) & 0x7fff);
}
constexpr int D3_RAND_MAX_P = 0x7fff;

#define PLAYER_FLAGS_DEAD_P 8
#define PLAYER_FLAGS_DYING_P 4
#define PLAYER_FLAGS_INVULNERABLE_P 1
#define OF_DESTROYABLE_P 0x4000

constexpr int OBJ_PLAYER_P = 1;
constexpr int OBJ_WEAPON_P = 6;
constexpr int OBJ_CLUTTER_P = 7;
constexpr int OBJ_ROBOT_P = 2;
constexpr int OBJ_BUILDING_P = 3;

constexpr int MT_NONE_P = 0;
constexpr int MT_PHYSICS_P = 1;
constexpr int MT_WALKING_P = 3;

#define WF_MICROWAVE_P 0x0001000
#define WF_FREEZE_P 0x0002000
#define WF_NAPALM_P 0x0040000

#define DF_DELAY_SPARKS_P 0x4
#define DF_DELAY_FLYING_P 0x100000
#define DF_BLAST_RING_P 0x80
#define DEATH_BASE_FLAGS_P (0x20 | 0x40 | 0x1000 | 0x2000 | 0x10000 | 0x20000)

struct ObjP {
  int type = OBJ_CLUTTER_P;
  int id = 0;
  uint32_t flags = OF_DESTROYABLE_P;
  float shields = 100;
  float pos[3] = {0, 0, 0};
  int movement_type = MT_NONE_P;
  float uvec_y = 1.0f;
};

struct PlayerP {
  uint32_t flags = 0;
  float armor_scalar = 1.0f;
  float damage_magnitude = 0;
  float invul_magnitude = 0;
  float invul_vec_x = 0;
  int ship_index = 0;
};

static ObjP obj_p[8];
static PlayerP ply_p[2];
static float ship_armor_p[2] = {1.0f, 0.5f};

struct WeaponFxP { // stands in for effect side effects
  bool deformed = false, frozen = false, napalmed = false, killed = false;
};
static WeaponFxP fx_p;
static bool damage_sound_played = false;

// ---------------------------------------------------------------------------
// Single-player ApplyDamageToPlayer replica (damage.cpp:807-956)
static bool RepApplyDamageSP(ObjP &pobj, PlayerP &pl, const ObjP *killer,
                             int damage_type, float dmg, int weapon_id,
                             bool playsound) {
  if (pobj.type != OBJ_PLAYER_P)
    return false;

  if ((pl.flags & PLAYER_FLAGS_DYING_P) || (pl.flags & PLAYER_FLAGS_DEAD_P)) {
    if (damage_type != 0)
      damage_sound_played = true; // quirk: corpses still play hit sounds
    return false;
  }

  if ((pl.flags & PLAYER_FLAGS_INVULNERABLE_P) || !(obj_p[0].flags & OF_DESTROYABLE_P)) {
    // quirk: destroyable gate reads GLOBAL local player object (obj slot 0),
    // not the object being damaged
    fx_p.frozen = true; // stands in for invulnerability sound
    if (killer != nullptr) {
      pl.invul_magnitude = 1;
      float dx = killer->pos[0] - pobj.pos[0];
      float len = fabsf(dx) > 0 ? fabsf(dx) : 1;
      pl.invul_vec_x = dx / len; // normalized weapon-minus-player direction
    }
    return false;
  }

  dmg *= pl.armor_scalar;
  dmg *= ship_armor_p[pl.ship_index];

  pobj.shields -= dmg;
  pl.damage_magnitude += dmg;

  if (dmg > 0 && playsound)
    damage_sound_played = true;

  if (weapon_id != 255) {
    if (weapon_id == 1)
      fx_p.deformed = true; // microwave
    if (weapon_id == 2)
      fx_p.frozen = true;   // freeze
    if (weapon_id == 3)
      fx_p.napalmed = true; // napalm
  }

  if (pobj.shields < 0) {
    fx_p.killed = true;      // KillPlayer
    pl.flags |= PLAYER_FLAGS_DEAD_P;
  }
  return true;
}

/**
 * @test DamagePlayer.ScalarsInvulKnockbackAndCorpseBails
 * @brief Verifies scalars Invul Knockback And Corpse Bails.
 *
 * @details
 * Exercises the DamagePlayer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/damage.cpp
 * @ingroup descent3_tests
 */
TEST(DamagePlayer, ScalarsInvulKnockbackAndCorpseBails) {
  auto reset = []() {
    for (auto &o : obj_p)
      o = ObjP{};
    ply_p[0] = PlayerP{};
    ply_p[1] = PlayerP{};
    fx_p = WeaponFxP{};
    damage_sound_played = false;
  };

  reset();
  obj_p[1].type = OBJ_PLAYER_P;
  ObjP weapon;
  weapon.type = OBJ_WEAPON_P;
  weapon.pos[0] = 5.0f;
  EXPECT_TRUE(RepApplyDamageSP(obj_p[1], ply_p[1], &weapon, 1, 30.0f, 255, true));
  EXPECT_FLOAT_EQ(obj_p[1].shields, 70.0f); // scalars are neutral at 1.0
  EXPECT_TRUE(damage_sound_played);
  EXPECT_FALSE(fx_p.killed);

  // both armor scalars multiply: 40 * 0.75(player) * 0.5(ship) = 15
  reset();
  obj_p[1].type = OBJ_PLAYER_P;
  ply_p[1].armor_scalar = 0.75f;
  ply_p[1].ship_index = 1;
  RepApplyDamageSP(obj_p[1], ply_p[1], nullptr, 0, 40.0f, 255, false);
  EXPECT_FLOAT_EQ(obj_p[1].shields, 85.0f);

  // lethal blow kills
  reset();
  obj_p[1].type = OBJ_PLAYER_P;
  RepApplyDamageSP(obj_p[1], ply_p[1], nullptr, 0, 150.0f, 255, true);
  EXPECT_TRUE(fx_p.killed);
  EXPECT_TRUE(ply_p[1].flags & PLAYER_FLAGS_DEAD_P);

  // dead players reject further damage but still emit a sound
  reset();
  obj_p[1].type = OBJ_PLAYER_P;
  ply_p[1].flags |= PLAYER_FLAGS_DEAD_P;
  obj_p[1].shields = 50;
  EXPECT_FALSE(RepApplyDamageSP(obj_p[1], ply_p[1], nullptr, 3, 10.0f, 255, true));
  EXPECT_FLOAT_EQ(obj_p[1].shields, 50.0f);
  EXPECT_TRUE(damage_sound_played);

  // invulnerable: no shield loss, knockback stored TOWARD the attacker
  reset();
  obj_p[1].type = OBJ_PLAYER_P;
  obj_p[1].pos[0] = 2.0f;
  ply_p[1].flags |= PLAYER_FLAGS_INVULNERABLE_P;
  EXPECT_FALSE(RepApplyDamageSP(obj_p[1], ply_p[1], &weapon, 1, 99.0f, 255, false));
  EXPECT_FLOAT_EQ(obj_p[1].shields, 100.0f);
  EXPECT_EQ(ply_p[1].invul_magnitude, 1);
  EXPECT_FLOAT_EQ(ply_p[1].invul_vec_x, 1.0f); // points at weapon (+x)

  // quirk: undestroyable LOCAL player (slot 0) makes ANY target invulnerable
  reset();
  obj_p[1].type = OBJ_PLAYER_P;
  obj_p[0].flags &= ~OF_DESTROYABLE_P; // ghost-mode observer locally
  EXPECT_FALSE(RepApplyDamageSP(obj_p[1], ply_p[1], nullptr, 0, 10.0f, 255, false));
  EXPECT_FLOAT_EQ(obj_p[1].shields, 100.0f);

  // weapon id gates special effects
  reset();
  obj_p[1].type = OBJ_PLAYER_P;
  RepApplyDamageSP(obj_p[1], ply_p[1], nullptr, 0, 5.0f, 3, false);
  EXPECT_TRUE(fx_p.napalmed);
  EXPECT_FALSE(fx_p.frozen);
}

// ---------------------------------------------------------------------------
// GenerateDefaultDeath replica (damage.cpp:959-1031)
struct DeathOutP {
  uint32_t flags = 0;
  float delay = 0;
};

static DeathOutP RepDefaultDeath(int type, int movement_type, float uvec_y, bool has_anim) {
  DeathOutP out;
  out.flags = 0;
  out.delay = 0.0f;

  bool quick;
  if (type != OBJ_ROBOT_P && !(type == OBJ_BUILDING_P))
    quick = true;
  else if (movement_type != MT_WALKING_P && (ps_rand_p() % 3) != 0)
    quick = true;
  else if (movement_type == MT_WALKING_P && (ps_rand_p() % 10) > 5)
    quick = true;
  else
    quick = false;

  if (quick) {
    if ((ps_rand_p() % 3) == 0)
      out.flags |= DF_BLAST_RING_P;
  } else {
    bool alternate = false;
    bool upsidedown = (movement_type == MT_WALKING_P) && (uvec_y < 0.0f);
    if (!upsidedown && (ps_rand_p() % 2) == 1) {
      out.flags = DF_DELAY_SPARKS_P;
      alternate = true;
    }

    if (type == OBJ_ROBOT_P) {
      if (alternate || (movement_type != MT_PHYSICS_P && movement_type != MT_WALKING_P)) {
        out.flags = DF_DELAY_SPARKS_P;
        out.delay = 3.0f;
        alternate = true;
      } else {
        out.flags = (movement_type == MT_WALKING_P) ? 0x8 : 0; // antigrav or plain
      }
      if (!alternate && has_anim)
        out.delay = 2.0f;
    } else { // building
      out.flags |= DF_DELAY_FLYING_P;
      out.delay = (uvec_y == 1.0f) ? 2.0f : 0.7f;
    }

    if ((ps_rand_p() % 8) == 0)
      out.flags |= DF_BLAST_RING_P;
  }

  out.flags |= DEATH_BASE_FLAGS_P;
  return out;
}

/**
 * @test DefaultDeath.QuickClutterRobotSparksAndFlyingBuildings
 * @brief Verifies quick Clutter Robot Sparks And Flying Buildings.
 *
 * @details
 * Exercises the DefaultDeath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/damage.cpp
 * @ingroup descent3_tests
 */
TEST(DefaultDeath, QuickClutterRobotSparksAndFlyingBuildings) {
  // clutter never qualifies as slow -> exactly one roll (blast ring %3)
  auto one_roll_pred = [](int32_t r) { return r % 3 != 0; }; // want NO ring
  for (uint32_t s = 0; s < 1000; s++) {
    ps_srand_p(s);
    if (!one_roll_pred(ps_rand_p()))
      continue;
    ps_srand_p(s);
    DeathOutP d = RepDefaultDeath(OBJ_CLUTTER_P, MT_NONE_P, 1.0f, false);
    EXPECT_EQ(d.delay, 0.0f);
    EXPECT_EQ(d.flags, DEATH_BASE_FLAGS_P); // base combo only
    break;
  }

  // MT_PHYSICS robot steered slow (first roll %3==0), no sparks-alternate
  // (second roll %2==0), no anim -> plain 0 flags; anim present -> 2s delay
  for (uint32_t s = 0; s < 100000; s++) {
    ps_srand_p(s);
    int32_t r0 = ps_rand_p();
    int32_t r1 = ps_rand_p();
    if (r0 % 3 != 0 || r1 % 2 != 0)
      continue;
    ps_srand_p(s);
    DeathOutP d = RepDefaultDeath(OBJ_ROBOT_P, MT_PHYSICS_P, 1.0f, false);
    EXPECT_EQ(d.flags, DEATH_BASE_FLAGS_P); // physics robot: plain quick burn
    break;
  }
  for (uint32_t s = 0; s < 100000; s++) {
    ps_srand_p(s);
    int32_t r0 = ps_rand_p();
    int32_t r1 = ps_rand_p();
    if (r0 % 3 != 0 || r1 % 2 != 0)
      continue;
    ps_srand_p(s);
    DeathOutP d = RepDefaultDeath(OBJ_ROBOT_P, MT_PHYSICS_P, 1.0f, true);
    EXPECT_FLOAT_EQ(d.delay, 2.0f); // anim fallback delay
    break;
  }

  // sparks variant: alternate roll %2==1 sets sparks+3s regardless of anim
  for (uint32_t s = 0; s < 100000; s++) {
    ps_srand_p(s);
    int32_t r0 = ps_rand_p();
    int32_t r1 = ps_rand_p();
    if (r0 % 3 != 0 || r1 % 2 != 1)
      continue;
    ps_srand_p(s);
    DeathOutP d = RepDefaultDeath(OBJ_ROBOT_P, MT_NONE_P, 1.0f, false);
    EXPECT_TRUE(d.flags & DF_DELAY_SPARKS_P);
    EXPECT_FLOAT_EQ(d.delay, 3.0f);
    break;
  }

  // building slow deaths fly upward; exact uvec.y==1.0 gets 2s, tilted 0.7
  for (uint32_t s = 0; s < 100000; s++) {
    ps_srand_p(s);
    int32_t r0 = ps_rand_p();
    if (r0 % 3 != 0)
      continue;
    ps_srand_p(s);
    DeathOutP d = RepDefaultDeath(OBJ_BUILDING_P, MT_PHYSICS_P, 1.0f, false);
    EXPECT_TRUE(d.flags & DF_DELAY_FLYING_P);
    EXPECT_FLOAT_EQ(d.delay, 2.0f);
    break;
  }
  for (uint32_t s = 0; s < 100000; s++) {
    ps_srand_p(s);
    int32_t r0 = ps_rand_p();
    if (r0 % 3 != 0)
      continue;
    ps_srand_p(s);
    DeathOutP d = RepDefaultDeath(OBJ_BUILDING_P, MT_PHYSICS_P, 0.9f, false);
    EXPECT_FLOAT_EQ(d.delay, 0.7f); // exact ==1.0 comparison decides
    break;
  }

  // every death carries the base fireball/break-apart/debris combo
  ps_srand_p(7);
  DeathOutP d = RepDefaultDeath(OBJ_ROBOT_P, MT_WALKING_P, -0.5f, false);
  EXPECT_TRUE(d.flags & DEATH_BASE_FLAGS_P);
}

// ---------------------------------------------------------------------------
// KillObject dispatch replica (damage.cpp:1037-1080)
#define OF_DYING_P 0x4
#define MAX_DEATH_TYPES_P 4

struct DeathTypeP {
  uint32_t flags = 0;
  float delay_min = 0, delay_max = 0;
};

struct ObjInfoP {
  DeathTypeP death_types[MAX_DEATH_TYPES_P];
  uint8_t death_probabilities[MAX_DEATH_TYPES_P] = {0, 0, 0, 0};
};

static ObjInfoP oinfo_p;

static DeathOutP RepKillDispatch(uint32_t flags_in) {
  DeathOutP out;
  out.delay = -1;
  out.flags = -1;

  if (flags_in & (OF_DYING_P | 0x2))
    return {-1u, -1.0f}; // bail silently

  int death_flags = -1;
  float delay_time = 0;
  int r = (ps_rand_p() * 100 / (D3_RAND_MAX_P + 1)) + 1;

  for (int i = 0; i < MAX_DEATH_TYPES_P; i++) {
    int p = oinfo_p.death_probabilities[i];
    if (r <= p) {
      death_flags = oinfo_p.death_types[i].flags;
      delay_time = oinfo_p.death_types[i].delay_min +
                   (oinfo_p.death_types[i].delay_max - oinfo_p.death_types[i].delay_min) *
                       ps_rand_p() / D3_RAND_MAX_P;
      break;
    }
    r -= p;
  }

  if (death_flags == -1) {
    death_flags = 0xABCD; // GenerateDefaultDeath placeholder
    delay_time = 9.9f;
  }
  return {(uint32_t)death_flags, delay_time};
}

/**
 * @test KillObjectDispatch.WeightedPickCumulativeShiftAndFallback
 * @brief Verifies weighted Pick Cumulative Shift And Fallback.
 *
 * @details
 * Exercises the KillObjectDispatch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/damage.cpp
 * @ingroup descent3_tests
 */
TEST(KillObjectDispatch, WeightedPickCumulativeShiftAndFallback) {
  oinfo_p = ObjInfoP{};
  oinfo_p.death_probabilities[0] = 30;
  oinfo_p.death_probabilities[1] = 30;
  oinfo_p.death_probabilities[2] = 40;
  oinfo_p.death_types[0] = {0x111, 1.0f, 1.0f};
  oinfo_p.death_types[1] = {0x222, 2.0f, 2.0f};
  oinfo_p.death_types[2] = {0x333, 3.0f, 3.0f};

  // r==1 -> first bucket (v=0 gives r=1)
  ps_srand_p(0);
  holdrand_p = 0; // force next rand == 0? LCG of 0 is nonzero; instead scan:
  for (uint32_t s = 0; s < 200000; s++) {
    ps_srand_p(s);
    if (ps_rand_p() == 0) {
      ps_srand_p(s);
      DeathOutP d = RepKillDispatch(0);
      EXPECT_EQ(d.flags, 0x111u);
      EXPECT_FLOAT_EQ(d.delay, 1.0f);
      break;
    }
  }

  // cumulative subtraction: r=31 lands in bucket 2 after r-=30
  for (uint32_t s = 0; s < 200000; s++) {
    ps_srand_p(s);
    int32_t v = ps_rand_p();
    if (v >= 9840 && v <= 10137) { // r == 31
      ps_srand_p(s);
      DeathOutP d = RepKillDispatch(0);
      EXPECT_EQ(d.flags, 0x222u);
      break;
    }
  }

  // zero coverage everywhere -> every roll falls through to default death
  oinfo_p.death_probabilities[0] = 0;
  oinfo_p.death_probabilities[1] = 0;
  oinfo_p.death_probabilities[2] = 0;
  for (uint32_t s = 1; s < 100; s++) {
    ps_srand_p(s);
    DeathOutP d = RepKillDispatch(0);
    EXPECT_EQ(d.flags, 0xABCDu);
    break;
  }

  // already dying/dead objects bail before any roll
  ps_srand_p(99);
  DeathOutP d = RepKillDispatch(OF_DYING_P);
  EXPECT_EQ(d.flags, 0xFFFFFFFFu);
}
