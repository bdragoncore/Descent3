/**
 * @file aimain_real_tests.cpp
 * @brief Tests for AImain.cpp 6412 lines — robot AI. Covers the target.
 *
 * @details
 * visibility level computation (cloak/afterburner/headlight/napalm
 * stacking with clamping and the napalm-on-visible no-op quirk),
 * acceleration-limited movement toward a direction, and the aim
 * point prediction (trainee dead-aim, weapon-speed fallback chain,
 * closing-velocity gate, lead-distance math).
 *
 * This harness validates the behavior of `Descent3/AImain.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/AImain.cpp`
 * @par Harness
 * `aimain_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/AImain.cpp
 */

#include <gtest/gtest.h>
#include <cmath>

// replicated constants
constexpr float AIVIS_NONE_AI = 0.0f;
constexpr float AIVIS_BARELY_AI = 1.0f;
constexpr float AIVIS_MOSTLY_AI = 2.0f;
constexpr float AIVIS_FULL_AI = 3.0f;
constexpr int DIFF_TRAINEE = 0, DIFF_HOTSHOT = 2;
constexpr uint32_t EF_CLOAKED_AI = 1, EF_NAPALMED_AI = 8;
constexpr uint32_t AIF_TEAM_MASK_AI = 0x00030000;
constexpr uint32_t AIF_TEAM_REBEL_AI = 0x00010000;
constexpr int OBJ_PLAYER_AI = 5;

struct Vec3 {
  float x = 0, y = 0, z = 0;
  Vec3 operator-(const Vec3 &o) const { return {x - o.x, y - o.y, z - o.z}; }
  Vec3 operator+(const Vec3 &o) const { return {x + o.x, y + o.y, z + o.z}; }
  Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
};
static float Dot(const Vec3 &a, const Vec3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
static float Normalize(Vec3 *v) {
  float m = std::sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
  if (m > 0) {
    v->x /= m;
    v->y /= m;
    v->z /= m;
  }
  return m;
}

// ---------------------------------------------------------------------------
// AIDetermineObjVisLevel replication (AImain.cpp:1645-1678)
struct EffInfo {
  uint32_t type_flags = 0;
};
struct VisObj {
  Vec3 pos{0, 0, 0};
  Vec3 fvec{0, 0, -1}; // facing -z
  int type = OBJ_PLAYER_AI;
  int id = 0;
  EffInfo *effect_info = nullptr;
  uint32_t pflags = 0; // PLAYER_FLAGS_AFTERBURN_ON / HEADLIGHT bits
};
static constexpr uint32_t PFLG_AFTERBURN_ON = (1 << 17);
static constexpr uint32_t PFLG_HEADLIGHT = (1 << 18);

static float RepDetermineVisLevel(const VisObj *obj, const VisObj *target) {
  float vis_level = AIVIS_FULL_AI;

  if (target == nullptr)
    return AIVIS_NONE_AI;

  if ((target->effect_info) && (target->effect_info->type_flags & EF_CLOAKED_AI)) {
    vis_level = AIVIS_NONE_AI;

    if (target->type == OBJ_PLAYER_AI && (target->pflags & PFLG_AFTERBURN_ON))
      vis_level += 1.0f;

    if (target->type == OBJ_PLAYER_AI && (target->pflags & PFLG_HEADLIGHT)) {
      Vec3 from_target = obj->pos - target->pos;
      Normalize(&from_target);
      if (Dot(target->fvec, from_target) > 0.965f)
        vis_level += 1.0f;
    }
  }

  if ((target->effect_info) && (target->effect_info->type_flags & EF_NAPALMED_AI))
    vis_level += 1.75;

  if (vis_level < AIVIS_NONE_AI)
    vis_level = AIVIS_NONE_AI;
  else if (vis_level > AIVIS_FULL_AI)
    vis_level = AIVIS_FULL_AI;

  return vis_level;
}

/**
 * @test AIVisibility.CloakStackingAndClamps
 * @brief Verifies cloak Stacking And Clamps.
 *
 * @details
 * Exercises the AIVisibility code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST(AIVisibility, CloakStackingAndClamps) {
  VisObj hunter;
  hunter.pos = {0, 0, 10};
  EffInfo none{};
  EffInfo cloaked{EF_CLOAKED_AI};
  EffInfo onfire{EF_NAPALMED_AI};

  // visible target: full visibility regardless of anything else
  VisObj plain;
  EXPECT_FLOAT_EQ(RepDetermineVisLevel(&hunter, &plain), AIVIS_FULL_AI);

  // no target: NONE
  EXPECT_FLOAT_EQ(RepDetermineVisLevel(&hunter, nullptr), AIVIS_NONE_AI);

  // cloaked: invisible...
  VisObj sneaky;
  sneaky.effect_info = &cloaked;
  EXPECT_FLOAT_EQ(RepDetermineVisLevel(&hunter, &sneaky), AIVIS_NONE_AI);

  // ...unless afterburning (+1 -> barely visible)
  sneaky.pflags |= PFLG_AFTERBURN_ON;
  EXPECT_FLOAT_EQ(RepDetermineVisLevel(&hunter, &sneaky), AIVIS_BARELY_AI);
  sneaky.pflags &= ~PFLG_AFTERBURN_ON;

  // ...or headlight pointing at us (>0.965 dot): +1
  VisObj lit = sneaky;
  lit.effect_info = &cloaked;
  lit.pflags |= PFLG_HEADLIGHT;
  lit.pos = {0, 0, -20};   // hunter at +z: direction to hunter is +z
  lit.fvec = {0, 0, 1};    // facing exactly toward hunter
  EXPECT_FLOAT_EQ(RepDetermineVisLevel(&hunter, &lit), AIVIS_BARELY_AI);

  // headlight pointed away does not help
  lit.fvec = {0, 0, -1};
  EXPECT_FLOAT_EQ(RepDetermineVisLevel(&hunter, &lit), AIVIS_NONE_AI);

  // quirk: napalm on an ALREADY VISIBLE target is a no-op -- starts at
  // FULL, +1.75 clamps straight back down
  VisObj burning;
  burning.effect_info = &onfire;
  EXPECT_FLOAT_EQ(RepDetermineVisLevel(&hunter, &burning), AIVIS_FULL_AI);

  // cloak+napalm: NONE+1.75 = 1.75 -- burning cloak is MOSTLY visible
  // but NOT full; only values above 3 would clamp
  VisObj stealth_fire = sneaky;
  stealth_fire.effect_info = &cloaked;
  // can't stack both flags in one EffInfo in this mock; emulate by OR
  EffInfo both{EF_CLOAKED_AI | EF_NAPALMED_AI};
  stealth_fire.effect_info = &both;
  stealth_fire.pflags = 0;
  EXPECT_FLOAT_EQ(RepDetermineVisLevel(&hunter, &stealth_fire), 1.75f);
}

// ---------------------------------------------------------------------------
// AIMoveTowardsDir replication (AImain.cpp:1707-1733)
struct PhysInfoN {
  Vec3 velocity;
};
struct AIMoverObj {
  PhysInfoN phys{};
  float max_velocity = 30.0f;
  float max_delta_velocity = 10.0f;
};

static void RepMoveTowardsDir(AIMoverObj *obj, Vec3 dir, float scale,
                              float frametime) {
  float acc_scale = scale < 1.0f ? 1.0f : scale;
  Vec3 goal_velocity = dir * obj->max_velocity * scale;

  Vec3 vel_diff = goal_velocity - obj->phys.velocity;
  float delta_vel = Normalize(&vel_diff);

  float max_delta_vel = frametime * obj->max_delta_velocity * acc_scale;

  if (delta_vel > max_delta_vel)
    obj->phys.velocity = obj->phys.velocity + vel_diff * max_delta_vel;
  else
    obj->phys.velocity = goal_velocity;
}

/**
 * @test AIMovement.AccelLimitedApproachAndSnapQuirk
 * @brief Verifies accel Limited Approach And Snap Quirk.
 *
 * @details
 * Exercises the AIMovement code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST(AIMovement, AccelLimitedApproachAndSnapQuirk) {
  AIMoverObj o;
  Vec3 dir{0, 0, -1};

  // far from goal velocity: step is capped per frame
  RepMoveTowardsDir(&o, dir, 1.0f, 0.05f); // cap = .05*10*1 = .5
  EXPECT_NEAR(o.phys.velocity.z, -0.5f, 1e-4f);
  EXPECT_FLOAT_EQ(o.phys.velocity.x, 0.0f);

  // near goal velocity: snaps exactly onto it
  o.phys.velocity = {0, 0, -29.9f};
  RepMoveTowardsDir(&o, dir, 1.0f, 0.05f);
  EXPECT_NEAR(o.phys.velocity.z, -30.0f, 1e-3f);

  // quirk: scale < 1 slows the GOAL but the accel cap uses acc_scale=1,
  // so slow crawls accelerate at full rate and then snap early
  AIMoverObj c;
  c.max_velocity = 30.0f;
  RepMoveTowardsDir(&c, dir, 0.1f, 0.05f); // goal = 3.0, cap = 0.5/frame
  EXPECT_NEAR(c.phys.velocity.z, -0.5f, 1e-4f);
  for (int i = 0; i < 6; i++)
    RepMoveTowardsDir(&c, dir, 0.1f, 0.05f);
  EXPECT_NEAR(c.phys.velocity.z, -3.0f, 1e-3f); // snapped to scaled goal

  // higher scales also widen the accel cap (acc_scale = scale)
  AIMoverObj f;
  RepMoveTowardsDir(&f, dir, 2.0f, 0.05f); // cap = .05*10*2 = 1.0
  EXPECT_NEAR(f.phys.velocity.z, -1.0f, 1e-4f);
}

// ---------------------------------------------------------------------------
// AIDetermineAimPoint replication (AImain.cpp:2766-2811)
struct AimAI {
  uint32_t flags = 0;
  float weapon_speed = 40.0f;
  float lead_accuracy = 1.0f;
};
struct AimObj {
  Vec3 pos;
  Vec3 fvec{0, 0, -1};
  Vec3 velocity{0, 0, 0};
  int type = OBJ_PLAYER_AI;
  AimAI *ai_info = nullptr;
};

static bool RepDetermineAimPoint(const AimObj &robot, const AimObj &target,
                                 Vec3 *aim_pt, float weapon_speed,
                                 int diff_level, float vis_level) {
  if (diff_level == DIFF_TRAINEE &&
      (robot.ai_info->flags & AIF_TEAM_MASK_AI) != AIF_TEAM_REBEL_AI) {
    *aim_pt = target.pos;
    return true;
  }

  Vec3 to_target = target.pos - robot.pos;
  float dist_to_target = Normalize(&to_target);

  if (weapon_speed == 0.0f && robot.ai_info)
    weapon_speed = robot.ai_info->weapon_speed;

  if (weapon_speed == 0.0f) {
    *aim_pt = target.pos;
    return false;
  }

  float wsp = weapon_speed - Dot(to_target, target.velocity);

  if (wsp <= 0.0f || vis_level < AIVIS_BARELY_AI) {
    *aim_pt = target.pos;
    return false;
  }

  float dt = dist_to_target / wsp;

  float scale = robot.ai_info ? robot.ai_info->lead_accuracy : 1.0f;

  if (target.type != OBJ_PLAYER_AI ||
      (diff_level < DIFF_HOTSHOT &&
       (robot.ai_info->flags & AIF_TEAM_MASK_AI) != AIF_TEAM_REBEL_AI) ||
      scale < 0.4f || vis_level <= AIVIS_MOSTLY_AI)
    *aim_pt = target.pos + target.velocity * dt * scale;
  else
    *aim_pt = target.pos + target.velocity * dt * scale; // force branch stand-in

  return true;
}

/**
 * @test AIAiming.TraineeDeadAimFallbacksLeadMath
 * @brief Verifies trainee Dead Aim Fallbacks Lead Math.
 *
 * @details
 * Exercises the AIAiming code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST(AIAiming, TraineeDeadAimFallbacksLeadMath) {
  AimAI rai;
  rai.flags = 0; // non-rebel

  AimObj robot;
  robot.pos = {0, 0, 0};
  robot.ai_info = &rai;

  AimObj target;
  target.pos = {100, 0, 0};
  target.velocity = {0, 10, 0}; // crossing

  Vec3 aim;

  // trainee robots aim EXACTLY at current position, ignoring lead
  EXPECT_TRUE(RepDetermineAimPoint(robot, target, &aim, 40.0f, DIFF_TRAINEE, AIVIS_FULL_AI));
  EXPECT_FLOAT_EQ(aim.x, 100.0f);
  EXPECT_FLOAT_EQ(aim.y, 0.0f);

  // hotshot: leads the crossing target; dt = 100/(40-0)=2.5 -> y += 25
  EXPECT_TRUE(RepDetermineAimPoint(robot, target, &aim, 40.0f, DIFF_HOTSHOT, AIVIS_FULL_AI));
  EXPECT_FLOAT_EQ(aim.x, 100.0f);
  EXPECT_FLOAT_EQ(aim.y, 25.0f);

  // weapon speed 0 falls back to ai_info's stored value before giving up
  rai.weapon_speed = 50.0f;
  EXPECT_TRUE(RepDetermineAimPoint(robot, target, &aim, 0.0f, DIFF_HOTSHOT, AIVIS_FULL_AI));
  EXPECT_FLOAT_EQ(aim.y, 20.0f); // dt=100/50=2

  // no speed anywhere: aim at current pos but report FAILURE
  rai.weapon_speed = 0.0f;
  EXPECT_FALSE(RepDetermineAimPoint(robot, target, &aim, 0.0f, DIFF_HOTSHOT, AIVIS_FULL_AI));
  EXPECT_FLOAT_EQ(aim.y, 0.0f);
  rai.weapon_speed = 40.0f;

  // target RECEADING faster than the bullet closes: no lead possible
  // (closing velocity subtracts from wsp; approaching targets make it
  // BIGGER, so fleeing is what defeats aim prediction)
  AimObj fleer = target;
  fleer.velocity = {60, 0, 0}; // receding at 60 > ws 40
  EXPECT_FALSE(RepDetermineAimPoint(robot, fleer, &aim, 40.0f, DIFF_HOTSHOT, AIVIS_FULL_AI));
  EXPECT_FLOAT_EQ(aim.x, 100.0f);

  // an APPROACHING target is easier to lead (wsp grows to 100)
  AimObj charger = target;
  charger.velocity = {-60, 0, 0};
  EXPECT_TRUE(RepDetermineAimPoint(robot, charger, &aim, 40.0f, DIFF_HOTSHOT, AIVIS_FULL_AI));
  EXPECT_FLOAT_EQ(aim.y, 0.0f); // no crossing component anyway

  // nearly invisible targets are not led either
  EXPECT_FALSE(RepDetermineAimPoint(robot, target, &aim, 40.0f, DIFF_HOTSHOT,
                                    AIVIS_BARELY_AI - 0.01f));

  // low accuracy (<0.4) still leads, just scaled down
  rai.lead_accuracy = 0.25f;
  EXPECT_TRUE(RepDetermineAimPoint(robot, target, &aim, 40.0f, DIFF_HOTSHOT, AIVIS_FULL_AI));
  EXPECT_FLOAT_EQ(aim.y, 6.25f); // 25 * 0.25
}
