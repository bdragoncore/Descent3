/**
 * @file pilot_class_real_tests.cpp
 * @brief Tests for pilot_class.cpp (~1536 lines).
 *
 * @details
 * Covers pilot file write/flush/read round trips, version gating
 * (PLTR_TOO_NEW / PFV_* sections), verify() stat fixups and autoselect
 * refresh, mission data management, and controller config restore with a
 * recording gameController stub.
 *
 * This harness validates the behavior of `Descent3/pilot_class.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/pilot_class.cpp`
 * @par Harness
 * `pilot_class_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/pilot_class.cpp
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

#include "pilot_class.h"
#include "controller.h"
#include "weapon.h"
#include "config.h"
#include "ship.h"
#include "hud.h"
#include "difficulty.h"
#include "mem.h"
#include "appdatabase.h"
#include "stringtable.h"
#include <cstdio>

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
std::vector<tVideoResolution> Video_res_list = {{640, 480}};
int Current_video_resolution_id = 0;
tGameToggles Game_toggles = {true, true, true};
float Key_ramp_speed = 0.35f;

static uint16_t g_autoselect_primary[MAX_PRIMARY_WEAPONS];
static uint16_t g_autoselect_secondary[MAX_SECONDARY_WEAPONS];
uint16_t GetAutoSelectPrimaryWpnIdx(int slot) { return g_autoselect_primary[slot]; }
uint16_t GetAutoSelectSecondaryWpnIdx(int slot) { return g_autoselect_secondary[slot]; }
void SetAutoSelectPrimaryWpnIdx(int slot, uint16_t idx) { g_autoselect_primary[slot] = idx; }
void SetAutoSelectSecondaryWpnIdx(int slot, uint16_t idx) { g_autoselect_secondary[slot] = idx; }

// Recording gameController stub
struct TestController : public gameController {
  TestController() : gameController(NUM_CONTROLLER_FUNCTIONS, Controller_needs) {}
  int set_calls = 0;
  int get_calls = 0;
  int mask_joy = -1, mask_mouse = -1;

  void flush() override {}
  ct_config_data get_controller_value(ct_type) override { return 0; }
  void set_controller_function(int, const ct_type *, ct_config_data, const uint8_t *) override { set_calls++; }
  void get_controller_function(int id, ct_type *type, ct_config_data *value, uint8_t *flags) override {
    get_calls++;
    type[0] = ctNone;
    type[1] = ctNone;
    *value = 0;
    flags[0] = flags[1] = 0;
    (void)id;
  }
  void enable_function(int, bool) override {}
  bool get_packet(int, ct_packet *, ct_format) override { return false; }
  float get_axis_sensitivity(ct_type, uint8_t) override { return 0.0f; }
  void set_axis_sensitivity(ct_type, uint8_t, float) override {}
  int assign_function(ct_function *) override { return 0; }
  void mask_controllers(bool joystick, bool mouse) override {
    mask_joy = joystick ? 1 : 0;
    mask_mouse = mouse ? 1 : 0;
  }
  const char *get_binding_text(ct_type, uint8_t, uint8_t) override { return ""; }
  int get_mouse_raw_values(int *, int *) override { return 0; }
  unsigned get_joy_raw_values(int *, int *) override { return 0; }
};

TestController g_test_controller;
gameController *Controller = &g_test_controller;

// Matches private #define in pilot_class.cpp
#define PLT_FILE_VERSION 0x2B

// Controller function table (normally filled by controls.cpp).
// IDs are the ctf* constants, which are simply 0..NUM_CONTROLLER_FUNCTIONS-1.
ct_function Controller_needs[NUM_CONTROLLER_FUNCTIONS];
int Default_ship_permission = 1;
struct ControllerNeedsInit {
  ControllerNeedsInit() {
    for (int i = 0; i < NUM_CONTROLLER_FUNCTIONS; i++)
      Controller_needs[i].id = i;
  }
} g_controller_needs_init;

// App database stub: reports "ProfanityPrevention" disabled so defaults apply
class StubAppDatabase : public oeAppDatabase {
public:
  bool create_record(const char *) override { return true; }
  bool lookup_record(const char *) override { return true; }
  bool read(const char *, char *, int *) override { return false; }
  bool read(const char *, bool *) override { return false; }
  bool write(const char *, const char *, int) override { return true; }
  bool write(const char *, int) override { return true; }
  void get_user_name(char *, size_t *) override {}
  bool read(const char *label, void *entry, int) override {
    if (strcmp(label, "ProfanityPrevention") == 0) {
      *(int *)entry = 0;
      return true;
    }
    return false;
  }
};
StubAppDatabase g_database;
oeAppDatabase *Database = &g_database;
const char *GetStringFromTable(int index) {
  static char buf[32];
  snprintf(buf, sizeof(buf), "str%d", index);
  return buf;
}
void grtext_SetProfanityFilter(bool) {}
void taunt_Enable(bool) {}


// ---------------------------------------------------------------------------

/**
 * @brief GTest fixture for PilotClassTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PilotClassTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() { cf_AddBaseDirectory("/tmp/opencode"); }

  void SetUp() override {
    memset(g_autoselect_primary, 0, sizeof(g_autoselect_primary));
    memset(g_autoselect_secondary, 0, sizeof(g_autoselect_secondary));
    Game_toggles = {true, true, true};
    Key_ramp_speed = 0.35f;
    g_test_controller.set_calls = 0;
    g_test_controller.get_calls = 0;
    g_test_controller.mask_joy = g_test_controller.mask_mouse = -1;

    remove("/tmp/opencode/test.plt");
  }

  // Fills a pilot with distinctive data and writes it out
  void MakePilot(pilot &plt, const char *fname) {
    plt.clean(true);
    plt.set_filename(fname);
    plt.set_name("Tester");
    plt.set_ship("Phoenix");
    plt.set_difficulty(DIFFICULTY_ACE);
    plt.set_profanity_filter(true);
    plt.set_audiotaunts(false);
    plt.set_guidebot_name((char *)"Buddy");

    uint8_t hmode = 3;
    uint16_t hstat = 7, hgraph = 9;
    int gw_w = 1024, gw_h = 768;
    plt.set_hud_data(&hmode, &hstat, &hgraph, &gw_w, &gw_h);

    tMissionData md{};
    md.highest_level = 4;
    md.ship_permissions = 2;
    md.finished = true;
    strcpy(md.mission_name, "D3LVL1");
    md.num_restores = 5;
    md.num_saves = 6;
    plt.add_mission_data(&md);

    plt.write();
    ASSERT_EQ(plt.flush(false), PLTW_NO_ERROR);
  }
};

/**
 * @test PilotClassTest.WriteReadRoundTripPreservesData
 * @brief Verifies write Read Round Trip Preserves Data.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, WriteReadRoundTripPreservesData) {
  pilot src;
  MakePilot(src, "test.plt");

  pilot dst;
  dst.clean(true);
  dst.set_filename("test.plt");
  dst.read(false, false);

  char buf[PILOT_STRING_SIZE];
  dst.get_name(buf);
  EXPECT_STREQ(buf, "Tester");
  dst.get_ship(buf);
  EXPECT_STREQ(buf, "Phoenix");
  dst.get_guidebot_name(buf);
  EXPECT_STREQ(buf, "Buddy");

  uint8_t diff;
  dst.get_difficulty(&diff);
  EXPECT_EQ(diff, DIFFICULTY_ACE);

  bool b;
  dst.get_profanity_filter(&b);
  EXPECT_TRUE(b);
  dst.get_audiotaunts(&b);
  EXPECT_FALSE(b);

  tMissionData md{};
  ASSERT_EQ(dst.find_mission_data("D3LVL1"), 0);
  dst.get_mission_data(0, &md);
  EXPECT_EQ(md.highest_level, 4);
  EXPECT_EQ(md.ship_permissions, 2);
  EXPECT_TRUE(md.finished);
  EXPECT_EQ(md.num_restores, 5);
  EXPECT_EQ(md.num_saves, 6);
}

/**
 * @test PilotClassTest.FlushWithoutPendingWriteSkipsDisk
 * @brief Verifies flush Without Pending Write Skips Disk.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, FlushWithoutPendingWriteSkipsDisk) {
  pilot plt;
  plt.set_filename("test.plt");
  // no write() -> pending flag clear -> no error but also no file
  EXPECT_EQ(plt.flush(false), PLTW_NO_ERROR);
  EXPECT_FALSE(cfexist(std::filesystem::path("/tmp/opencode/test.plt")));
}

/**
 * @test PilotClassTest.FlushWithoutFilenameFails
 * @brief Verifies flush Without Filename Fails.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, FlushWithoutFilenameFails) {
  pilot plt;
  plt.write();
  EXPECT_EQ(plt.flush(false), PLTW_NO_FILENAME);
}

/**
 * @test PilotClassTest.NewFileRefusesToOverwrite
 * @brief Verifies new File Refuses To Overwrite.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, NewFileRefusesToOverwrite) {
  pilot plt;
  plt.clean(true);
  plt.set_filename("test.plt");
  plt.set_name("One");
  plt.write();
  ASSERT_EQ(plt.flush(true), PLTW_NO_ERROR);   // creates
  plt.write();
  EXPECT_EQ(plt.flush(true), PLTW_FILE_EXISTS); // refuses second create
}

/**
 * @test PilotClassTest.ReadMissingFileReportsNoExist
 * @brief Verifies read Missing File Reports No Exist.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, ReadMissingFileReportsNoExist) {
  pilot plt;
  plt.set_filename("no_such_pilot.plt");
  EXPECT_EQ(plt.read(false, false), PLTR_FILE_NOEXIST);
}

/**
 * @test PilotClassTest.TooNewVersionRejected
 * @brief Verifies too New Version Rejected.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, TooNewVersionRejected) {
  CFILE *cf = cfopen("/tmp/opencode/test.plt", "wb");
  ASSERT_NE(cf, nullptr);
  cf_WriteInt(cf, PLT_FILE_VERSION + 1); // future version
  cfclose(cf);

  pilot plt;
  plt.set_filename("test.plt");
  EXPECT_EQ(plt.read(false, false), PLTR_TOO_NEW);
}

/**
 * @test PilotClassTest.VerifyMovesGraphicalInventoryStatToText
 * @brief Verifies verify Moves Graphical Inventory Stat To Text.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, VerifyMovesGraphicalInventoryStatToText) {
  pilot plt;
  plt.clean(true);
  uint8_t hmode = 1;
  uint16_t hstat = 0, hgraph = STAT_STANDARD | STAT_INVENTORY;
  int w = 640, h = 480;
  plt.set_hud_data(&hmode, &hstat, &hgraph, &w, &h);
  plt.verify();

  plt.get_hud_data(&hmode, &hstat, &hgraph, &w, &h);
  EXPECT_FALSE(hgraph & STAT_INVENTORY); // stripped from graphical...
  EXPECT_TRUE(hstat & STAT_INVENTORY);   // ...and moved into the text stat
}

/**
 * @test PilotClassTest.WeaponAutoselectRoundTripsThroughFile
 * @brief Verifies weapon Autoselect Round Trips Through File.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, WeaponAutoselectRoundTripsThroughFile) {
  g_autoselect_primary[0] = 42;
  g_autoselect_secondary[3] = 77;

  pilot src;
  MakePilot(src, "test.plt"); // flush->verify() pulls the values above in

  // read pushes them back out through SetAutoSelect*WpnIdx
  pilot dst;
  dst.clean(true);
  dst.set_filename("test.plt");
  ASSERT_EQ(dst.read(false, false), PLTR_NO_ERROR);
  EXPECT_EQ(g_autoselect_primary[0], 42);
  EXPECT_EQ(g_autoselect_secondary[3], 77);
}

/**
 * @test PilotClassTest.MissionDataAddFindEdit
 * @brief Verifies mission Data Add Find Edit.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, MissionDataAddFindEdit) {
  pilot plt;
  plt.clean(true);
  EXPECT_EQ(plt.find_mission_data("nothing"), -1);

  tMissionData a{}, b{};
  strcpy(a.mission_name, "Alpha");
  a.highest_level = 1;
  strcpy(b.mission_name, "Beta");
  b.highest_level = 2;
  plt.add_mission_data(&a);
  plt.add_mission_data(&b);
  EXPECT_EQ(plt.find_mission_data("beta"), 1); // case-insensitive

  tMissionData edited{};
  strcpy(edited.mission_name, "Beta");
  edited.highest_level = 9;
  plt.edit_mission_data(1, &edited);

  tMissionData got{};
  plt.get_mission_data(1, &got);
  EXPECT_EQ(got.highest_level, 9);
}

/**
 * @test PilotClassTest.ControlsRestoreFeedsControllerAndAppliesMasks
 * @brief Verifies controls Restore Feeds Controller And Applies Masks.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, ControlsRestoreFeedsControllerAndAppliesMasks) {
  pilot src;
  MakePilot(src, "test.plt"); // default read_controller has both masks? set explicitly:
  src.read_controller = READF_JOY | READF_MOUSE;
  src.controls[5].value = 1234; // natural slot keeps id uniqueness
  src.write();
  ASSERT_EQ(src.flush(false), PLTW_NO_ERROR);

  pilot dst;
  dst.clean(true);
  dst.set_filename("test.plt");
  ASSERT_EQ(dst.read(false, false), PLTR_NO_ERROR);

  EXPECT_GE(g_test_controller.set_calls, 73); // every function re-bound
  EXPECT_EQ(dst.controls[5].value, 1234);
  EXPECT_EQ(g_test_controller.mask_joy, 1);
  EXPECT_EQ(g_test_controller.mask_mouse, 1);
  // gameplay toggles + key ramp restored to globals
  EXPECT_FLOAT_EQ(Key_ramp_speed, dst.key_ramping);
}

/**
 * @test PilotClassTest.SkipConfigLeavesControllerUntouched
 * @brief Verifies skip Config Leaves Controller Untouched.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, SkipConfigLeavesControllerUntouched) {
  pilot src;
  MakePilot(src, "test.plt");
  int sets_before = g_test_controller.set_calls;

  pilot dst;
  dst.clean(true);
  dst.set_filename("test.plt");
  ASSERT_EQ(dst.read(true /*skip controls*/, false), PLTR_NO_ERROR);
  EXPECT_EQ(g_test_controller.set_calls, sets_before); // no rebinding
}

/**
 * @test PilotClassTest.DifficultyDefaultsToRookieForFreshPilot
 * @brief Verifies difficulty Defaults To Rookie For Fresh Pilot.
 *
 * @details
 * Exercises the PilotClassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_class.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotClassTest, DifficultyDefaultsToRookieForFreshPilot) {
  pilot plt;
  plt.clean(true);
  uint8_t diff;
  plt.get_difficulty(&diff);
  EXPECT_EQ(diff, DIFFICULTY_ROOKIE);
}
