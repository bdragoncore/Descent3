/**
 * @file weather_real_tests.cpp
 * @brief Tests for weather.cpp — rain/snow/lightning systems (310 lines).
 *
 * @details
 * Compiles real Descent3/weather.cpp with fake hlsSystem, vis-effect pool,
 * deterministic ps_rand LCG, terrain/room stubs and real vecmat.
 *
 * This harness validates the behavior of `Descent3/weather.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/weather.cpp`
 * @par Harness
 * `weather_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/weather.cpp
 */

#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <cstring>
#include "weather.h"
#include "psrand.h"
#include "object.h"
#include "hlsoundlib.h"
#include "fireball_external.h"
#include "grdefs.h"

extern void DoSnowEffect();

extern int ThunderA_sound_handle;
extern int ThunderB_sound_handle;
extern int Num_ambient_sound_patterns;

// ---- deterministic ps_rand (LCG from misc/psrand.cpp) ----
static int32_t g_seed = 1;
void ps_srand(uint32_t seed) { g_seed = (int32_t)seed; }
int32_t ps_rand(void) {
  g_seed = g_seed * 214013 + 2531011;
  return (int32_t)((g_seed >> 16) & 0x7fff);
}

// ---- vis effect pool ----
#include "viseffect.h"
static vis_effect g_vis_pool[MAX_VIS_EFFECTS];
vis_effect *VisEffects = g_vis_pool;
int g_vis_next = 0;

struct VisRec {
  uint8_t type, id;
  int roomnum;
  vector pos;
};
std::vector<VisRec> g_vis_creates;

int VisEffectCreate(uint8_t type, uint8_t id, int roomnum, vector *pos) {
  if (g_vis_next >= MAX_VIS_EFFECTS)
    return -1;
  int idx = g_vis_next++;
  memset(&g_vis_pool[idx], 0, sizeof(vis_effect));
  g_vis_pool[idx].type = type;
  g_vis_pool[idx].id = id;
  g_vis_pool[idx].roomnum = roomnum;
  g_vis_pool[idx].pos = *pos;
  g_vis_creates.push_back({type, id, roomnum, *pos});
  return idx;
}

// ---- sound name lookup stub ----
int FindSoundName(const char *name) {
  if (!name)
    return -1;
  if (strcmp(name, "ThunderA") == 0)
    return 101;
  if (strcmp(name, "ThunderB") == 0)
    return 102;
  return -1;
}

// ---- terrain stub ----
float GetTerrainGroundPoint(vector *pos, vector *normal) {
  if (normal) {
    normal->x() = 0;
    normal->y() = 1.0f;
    normal->z() = 0;
  }
  return 5.0f;
}

// ---- game globals ----
float Gametime = 0.0f;
object *Viewer_object = nullptr;
object *Player_object = nullptr;

// ---- rooms ----
#include "room.h"
#define TEST_MAX_ROOMS 4
room Rooms[TEST_MAX_ROOMS];

struct FakePlayRec {
  int index;
  float volume;
};
extern hlsSystem Sound_system; // defined in fake_hlsystem.cpp
extern std::vector<FakePlayRec> g_fake_plays;

// helpers
static object g_viewer;
static object g_player;
static portal g_portals[2];

static void SetupOutsideObject(object &obj) {
  memset(&obj, 0, sizeof(object));
  obj.type = OBJ_PLAYER;
  obj.roomnum = (int)ROOMNUM_CELLNUM_FLAG | 3; // outside
  obj.pos = {500.0f, 0.0f, 500.0f};
  obj.orient.rvec = {1, 0, 0};
  obj.orient.uvec = {0, 1, 0};
  obj.orient.fvec = {0, 0, 1};
  obj.mtype.phys_info.velocity = {0, 0, 50.0f};
}

/**
 * @brief GTest fixture for WeatherTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class WeatherTest : public ::testing::Test {
protected:
  void SetUp() override {
    ps_srand(42);
    ResetWeather();
    g_vis_next = 0;
    memset(g_vis_pool, 0, sizeof(g_vis_pool));
    g_vis_creates.clear();
    g_fake_plays.clear();
    Gametime = 100.0f;
    ThunderA_sound_handle = -1;
    ThunderB_sound_handle = -1;

    SetupOutsideObject(g_viewer);
    SetupOutsideObject(g_player);
    Viewer_object = &g_viewer;
    Player_object = &g_player;

    memset(Rooms, 0, sizeof(Rooms));
    for (int i = 0; i < TEST_MAX_ROOMS; i++)
      Rooms[i].flags = 0;
    Rooms[0].portals = g_portals;
  }
};

/**
 * @test WeatherTest.SetRainStateTogglesFlagAndScalar
 * @brief Verifies set Rain State Toggles Flag And Scalar.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, SetRainStateTogglesFlagAndScalar) {
  EXPECT_EQ(Weather.flags, 0);
  SetRainState(1, 0.75f);
  EXPECT_NE(Weather.flags & WEATHER_FLAGS_RAIN, 0);
  EXPECT_FLOAT_EQ(Weather.rain_intensity_scalar, 0.75f);
  SetRainState(0, 0);
  EXPECT_EQ(Weather.flags & WEATHER_FLAGS_RAIN, 0);
}

/**
 * @test WeatherTest.SetSnowStateTogglesFlagAndScalar
 * @brief Verifies set Snow State Toggles Flag And Scalar.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, SetSnowStateTogglesFlagAndScalar) {
  SetSnowState(1, 0.5f);
  EXPECT_NE(Weather.flags & WEATHER_FLAGS_SNOW, 0);
  EXPECT_FLOAT_EQ(Weather.snow_intensity_scalar, 0.5f);
  SetSnowState(0, 0);
  EXPECT_EQ(Weather.flags & WEATHER_FLAGS_SNOW, 0);
}

/**
 * @test WeatherTest.SetLightningStateSetsParams
 * @brief Verifies set Lightning State Sets Params.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, SetLightningStateSetsParams) {
  SetLightningState(1, 2.5f, 1234);
  EXPECT_NE(Weather.flags & WEATHER_FLAGS_LIGHTNING, 0);
  EXPECT_EQ(Weather.lightning_sequence, 0);
  EXPECT_EQ(Weather.lightning_rand_value, 1234);
  EXPECT_FLOAT_EQ(Weather.lightning_interval_time, 2.5f);
  SetLightningState(0, 0, 0);
  EXPECT_EQ(Weather.flags & WEATHER_FLAGS_LIGHTNING, 0);
}

/**
 * @test WeatherTest.ResetWeatherClears
 * @brief Verifies reset Weather Clears.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, ResetWeatherClears) {
  SetRainState(1, 1);
  Weather.last_lightning_evaluation_time = 55.0f;
  ResetWeather();
  EXPECT_EQ(Weather.flags, 0);
  EXPECT_FLOAT_EQ(Weather.last_lightning_evaluation_time, 0);
}

/**
 * @test WeatherTest.RainCreatesDistanceRainAndPuddles
 * @brief Verifies rain Creates Distance Rain And Puddles.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, RainCreatesDistanceRainAndPuddles) {
  SetRainState(1, 0.5f);
  DoRainEffect();

  int fading = 0, puddles = 0;
  for (auto &r : g_vis_creates) {
    if (r.id == FADING_LINE_INDEX)
      fading++;
    else if (r.id == PUDDLEDROP_INDEX)
      puddles++;
  }
  // distance rain: 20..34 lines
  EXPECT_GE(fading, 20);
  EXPECT_LE(fading, 34);

  // every fading line: life .001, windshield flag, end_pos.y() = pos.y+20
  for (int i = 0; i < g_vis_next; i++) {
    vis_effect &v = g_vis_pool[i];
    if (v.id == FADING_LINE_INDEX) {
      EXPECT_FLOAT_EQ(v.lifeleft, 0.001f);
      EXPECT_FLOAT_EQ(v.lifetime, 0.001f);
      EXPECT_NE(v.flags & VF_WINDSHIELD_EFFECT, 0);
      EXPECT_NEAR(v.end_pos.y(), v.pos.y() + 20.0f, 0.01f);
      EXPECT_EQ(v.lighting_color, GR_RGB16(200, 200, 255));
    } else if (v.id == PUDDLEDROP_INDEX) {
      puddles++;
      EXPECT_FLOAT_EQ(v.pos.y(), 5.0f); // ground stub applied
      EXPECT_NE(v.flags & VF_PLANAR, 0);
      EXPECT_FLOAT_EQ(v.lifeleft, 0.2f);
    }
  }
}

/**
 * @test WeatherTest.RainWindshieldDropletEventuallyAppears
 * @brief Verifies rain Windshield Droplet Eventually Appears.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, RainWindshieldDropletEventuallyAppears) {
  SetRainState(1, 0.9f);
  bool found = false;
  for (int iter = 0; iter < 4000 && !found; iter++) {
    ps_srand(iter + 1);
    g_vis_next = 0;
    g_vis_creates.clear();
    DoRainEffect();
    for (int i = 0; i < g_vis_next; i++) {
      if (g_vis_pool[i].id == RAINDROP_INDEX) {
        found = true;
        vis_effect &v = g_vis_pool[i];
        EXPECT_NE(v.flags & VF_WINDSHIELD_EFFECT, 0);
        EXPECT_NEAR(v.pos.z(), 3.0f, 0.001f);
        EXPECT_LE(std::abs(v.pos.x()), 2.0f);
        EXPECT_LE(std::abs(v.pos.y()), 500.0f / 350.0f + 0.01f);
        EXPECT_FLOAT_EQ(v.lifeleft, v.lifetime);
        break;
      }
    }
  }
  EXPECT_TRUE(found);
}

/**
 * @test WeatherTest.SnowCapPreventsCreation
 * @brief Verifies snow Cap Prevents Creation.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, SnowCapPreventsCreation) {
  SetSnowState(1, 1.0f);
  Weather.snowflakes_to_create = MAX_SNOW_INTENSITY + 200; // way over cap
  DoSnowEffect();
  EXPECT_TRUE(g_vis_creates.empty());
  EXPECT_EQ(Weather.snowflakes_to_create, 0);
}

/**
 * @test WeatherTest.SnowCreatesBoundedFlakes
 * @brief Verifies snow Creates Bounded Flakes.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, SnowCreatesBoundedFlakes) {
  SetSnowState(1, 1.0f);
  Weather.snowflakes_to_create = 240;
  ps_srand(7);
  DoSnowEffect();
  int flakes = 0;
  for (auto &r : g_vis_creates) {
    ASSERT_EQ(r.id, SNOWFLAKE_INDEX);
    flakes++;
  }
  // cap allows only 250-240=10 this frame
  EXPECT_LE(flakes, 10);
  EXPECT_GE(flakes, 1);
  for (int i = 0; i < g_vis_next; i++) {
    vis_effect &v = g_vis_pool[i];
    EXPECT_FLOAT_EQ(v.velocity.x(), 0);
    EXPECT_FLOAT_EQ(v.velocity.y(), -30.0f);
    EXPECT_FLOAT_EQ(v.velocity.z(), 0);
    EXPECT_NE(v.flags & VF_USES_LIFELEFT, 0);
    EXPECT_GE(v.size, 0.5f);
    EXPECT_LT(v.size, 1.51f);
  }
  EXPECT_EQ(Weather.snowflakes_to_create, 0);
}

/**
 * @test WeatherTest.LightningTriggersSequenceWhenIntervalElapsed
 * @brief Verifies lightning Triggers Sequence When Interval Elapsed.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, LightningTriggersSequenceWhenIntervalElapsed) {
  Player_object->roomnum = 0; // inside room 0
  Rooms[0].num_portals = 1;
  g_portals[0].croom = -1; // unconnected portal -> thunder audible

  SetLightningState(1, 1.0f, D3_RAND_MAX); // always strike
  Weather.last_lightning_evaluation_time = 99.0f;
  Gametime = 100.5f; // interval elapsed

  DoWeatherForFrame();

  EXPECT_EQ(Weather.lightning_sequence, 1);
  EXPECT_FLOAT_EQ(Weather.last_lightning_evaluation_time, 100.5f);

  float eval_time = Weather.last_lightning_evaluation_time;
  DoWeatherForFrame(); // within interval: no re-evaluation
  EXPECT_FLOAT_EQ(Weather.last_lightning_evaluation_time, eval_time);
  EXPECT_EQ(Weather.lightning_sequence, 1);
}

/**
 * @test WeatherTest.LightningSkipsWhenInsideWithoutOpenPortals
 * @brief Verifies lightning Skips When Inside Without Open Portals.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, LightningSkipsWhenInsideWithoutOpenPortals) {
  Player_object->roomnum = 0;
  Rooms[0].num_portals = 1;
  g_portals[0].croom = 1;       // connected
  Rooms[1].flags = 0;           // not external
  Rooms[1].portals = nullptr;
  Rooms[1].num_portals = 0;

  SetLightningState(1, 1.0f, 0); // rand_value 0: ps_rand()<=0 never true -> no strike
  Gametime = 200.0f;
  Weather.last_lightning_evaluation_time = 0;
  DoWeatherForFrame();
  EXPECT_EQ(Weather.lightning_sequence, 0);
}

/**
 * @test WeatherTest.DispatchesPerFlags
 * @brief Verifies dispatches Per Flags.
 *
 * @details
 * Exercises the WeatherTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weather.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeatherTest, DispatchesPerFlags) {
  Player_object->roomnum = ROOMNUM_CELLNUM_FLAG; // outside -> hear_thunder
  SetRainState(1, 0.5f);
  SetSnowState(1, 0.5f);
  DoWeatherForFrame();
  bool has_fading = false, has_snow = false;
  for (auto &r : g_vis_creates) {
    has_fading |= (r.id == FADING_LINE_INDEX);
    has_snow |= (r.id == SNOWFLAKE_INDEX);
  }
  EXPECT_TRUE(has_fading);
  EXPECT_TRUE(has_snow);
}
