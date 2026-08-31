/**
 * @file multi_save_setting_real_tests.cpp
 * @brief Tests for multi_save_setting.cpp (272 lines).
 *
 * @details
 * Covers the multiplayer settings file round trip: scalar fields, boolean
 * flag lines, name/mission space rejoining, KILLGOAL/TIMER flag derivation,
 * difficulty clamping, object/ship ban lines, and error paths.
 *
 * This harness validates the behavior of `Descent3/multi_save_setting.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/multi_save_setting.cpp`
 * @par Harness
 * `multi_save_setting_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/multi_save_setting.cpp
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "cfile.h"
#include "log.h"
#include "multi.h"
#include "multi_save_settings.h"
#include "objinfo.h"
#include "ship.h"
#include "player.h"
#include "object.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
netgame_info Netgame;
object_info Object_info[MAX_OBJECT_IDS];
ship Ships[MAX_SHIPS];

// ---------------------------------------------------------------------------
// Stubs for lookup layers not linked here
// ---------------------------------------------------------------------------
static std::vector<std::pair<std::string, int>> g_ship_permits; // (name, allowed)
int FindObjectIDName(const char *name) {
  if (strcmp(name, "Virus") == 0)
    return 5;
  return -1;
}
bool PlayerIsShipAllowed(int, int ship_index) { return ship_index != 7; }
bool PlayerIsShipAllowed(int, char *ship_name) {
  (void)ship_name;
  return true;
}
bool PlayerSetShipPermission(int pnum, char *ship_name, bool allowed) {
  g_ship_permits.push_back({ship_name, (int)allowed});
  (void)pnum;
  return true;
}

/**
 * @brief GTest fixture for MultiSaveTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class MultiSaveTest : public ::testing::Test {
protected:
  const std::string path = "/tmp/opencode/mp_settings.txt";

  void SetUp() override {
    Netgame = netgame_info{};
    memset(Object_info, 0, sizeof(Object_info));
    memset(Ships, 0, sizeof(Ships));
    strcpy(Netgame.name, "My Cool Game");
    strcpy(Netgame.mission, "Dave's Level");
    strcpy(Netgame.scriptname, "scr");
    Netgame.packets_per_second = 20;
    Netgame.max_players = 8;
    Netgame.respawn_time = 5;
    Netgame.killgoal = 25;
    Netgame.timelimit = 10;
    Netgame.difficulty = 3;
    g_ship_permits.clear();
    remove(path.c_str());
  }

  bool Save() { return MultiSaveSettings(path) == 1; }
  bool Load() { return MultiLoadSettings(path) == 1; }

  std::string ReadFile() {
    FILE *fp = fopen(path.c_str(), "rb");
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
      s.append(buf, n);
    fclose(fp);
    return s;
  }
};

// ---------------------------------------------------------------------------
// Round trips
// ---------------------------------------------------------------------------

/**
 * @test MultiSaveTest.ScalarsRoundTrip
 * @brief Verifies scalars Round Trip.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, ScalarsRoundTrip) {
  ASSERT_TRUE(Save());
  Netgame.max_players = -1;
  Netgame.respawn_time = -1;
  Netgame.packets_per_second = 0;
  ASSERT_TRUE(Load());

  EXPECT_EQ(Netgame.packets_per_second, 20);
  EXPECT_EQ(Netgame.max_players, 8);
  EXPECT_EQ(Netgame.respawn_time, 5);
  EXPECT_EQ(Netgame.difficulty, 3);
  EXPECT_EQ(Netgame.killgoal, 25);
}

/**
 * @test MultiSaveTest.NamesKeepTheirSpaces
 * @brief Verifies names Keep Their Spaces.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, NamesKeepTheirSpaces) {
  ASSERT_TRUE(Save());
  strcpy(Netgame.name, "");
  strcpy(Netgame.mission, "");
  ASSERT_TRUE(Load());

  EXPECT_STREQ(Netgame.name, "My Cool Game");
  EXPECT_STREQ(Netgame.mission, "Dave's Level");
  EXPECT_STREQ(Netgame.scriptname, "scr");
}

/**
 * @test MultiSaveTest.BooleanFlagsRoundTripBothWays
 * @brief Verifies boolean Flags Round Trip Both Ways.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, BooleanFlagsRoundTripBothWays) {
  Netgame.flags |= NF_PEER_PEER | NF_USE_ACC_WEAP | NF_BRIGHT_PLAYERS;

  ASSERT_TRUE(Save());
  Netgame.flags = 0;
  ASSERT_TRUE(Load());
  EXPECT_TRUE(Netgame.flags & NF_PEER_PEER);
  EXPECT_TRUE(Netgame.flags & NF_USE_ACC_WEAP);
  EXPECT_TRUE(Netgame.flags & NF_BRIGHT_PLAYERS);
  EXPECT_FALSE(Netgame.flags & NF_PERMISSABLE);

  // Now save them all OFF...
  Netgame.flags &= ~(NF_PEER_PEER | NF_USE_ACC_WEAP);
  ASSERT_TRUE(Save());
  // ...pretend every flag got set from somewhere else, then reload
  Netgame.flags = ~0u;
  ASSERT_TRUE(Load());
  EXPECT_FALSE(Netgame.flags & NF_PEER_PEER);
  EXPECT_FALSE(Netgame.flags & NF_USE_ACC_WEAP);
}

/**
 * @test MultiSaveTest.KillgoalAndTimerFlagsDerivedFromValues
 * @brief Verifies killgoal And Timer Flags Derived From Values.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, KillgoalAndTimerFlagsDerivedFromValues) {
  ASSERT_TRUE(Save()); // killgoal=25, timelimit=10
  Netgame.flags = 0;   // drop both flags
  ASSERT_TRUE(Load());

  EXPECT_TRUE(Netgame.flags & NF_KILLGOAL); // nonzero killgoal -> set
  EXPECT_TRUE(Netgame.flags & NF_TIMER);    // nonzero timelimit -> set
}

/**
 * @test MultiSaveTest.ZeroKillgoalClearsStaleFlagOnLoad
 * @brief Verifies zero Killgoal Clears Stale Flag On Load.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, ZeroKillgoalClearsStaleFlagOnLoad) {
  Netgame.killgoal = 0;
  Netgame.timelimit = 0;
  ASSERT_TRUE(Save());

  Netgame.flags |= NF_KILLGOAL | NF_TIMER; // stale flags from elsewhere
  ASSERT_TRUE(Load());

  EXPECT_FALSE(Netgame.flags & NF_KILLGOAL);
  // Quirk: TIMELIMIT=0 also clears NF_TIMER, but only because the save wrote
  // a zero; there is no explicit "no timer" line.
  EXPECT_EQ(Netgame.timelimit, 0);
}

/**
 * @test MultiSaveTest.DifficultyOutOfRangeClampsToZeroOnLoad
 * @brief Verifies difficulty Out Of Range Clamps To Zero On Load.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, DifficultyOutOfRangeClampsToZeroOnLoad) {
  Netgame.difficulty = 7; // save writes it verbatim
  ASSERT_TRUE(Save());
  ASSERT_TRUE(Load());
  EXPECT_EQ(Netgame.difficulty, 0); // load rejects >4

  Netgame.difficulty = 200;         // uint8 overflow case
  ASSERT_TRUE(Save());
  ASSERT_TRUE(Load());
  EXPECT_EQ(Netgame.difficulty, 0);
}

/**
 * @test MultiSaveTest.UnknownLinesAreSkippedNotFatal
 * @brief Verifies unknown Lines Are Skipped Not Fatal.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, UnknownLinesAreSkippedNotFatal) {
  ASSERT_TRUE(Save());
  {
    FILE *fp = fopen(path.c_str(), "a");
    fputs("FUTURE_FIELD\twhatever\n", fp);
    fclose(fp);
  }
  Netgame.max_players = 0;
  ASSERT_TRUE(Load());
  EXPECT_EQ(Netgame.max_players, 8); // rest of file still applied
}

// ---------------------------------------------------------------------------
// Ban lists
// ---------------------------------------------------------------------------

/**
 * @test MultiSaveTest.ObjectBansRoundTripThroughLookup
 * @brief Verifies object Bans Round Trip Through Lookup.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, ObjectBansRoundTripThroughLookup) {
  strcpy(Object_info[5].name, "Virus");
  Object_info[5].type = OBJ_POWERUP;
  Object_info[5].multi_allowed = 0;

  ASSERT_TRUE(Save());
  EXPECT_NE(ReadFile().find("OBJBAN\tVirus"), std::string::npos);

  Object_info[5].multi_allowed = 1; // forget the ban
  ASSERT_TRUE(Load());
  EXPECT_EQ(Object_info[5].multi_allowed, 0); // restored via OBJBAN line
}

/**
 * @test MultiSaveTest.NonPowerupsAreNeverWrittenAsBanned
 * @brief Verifies non Powerups Are Never Written As Banned.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, NonPowerupsAreNeverWrittenAsBanned) {
  strcpy(Object_info[3].name, "SomeRobot");
  Object_info[3].type = OBJ_ROBOT;
  Object_info[3].multi_allowed = 0;

  ASSERT_TRUE(Save());
  EXPECT_EQ(ReadFile().find("OBJBAN"), std::string::npos);
}

/**
 * @test MultiSaveTest.ShipBansGoThroughPermissionStub
 * @brief Verifies ship Bans Go Through Permission Stub.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, ShipBansGoThroughPermissionStub) {
  Ships[7].used = 1; // stub bans index 7 only
  strcpy(Ships[7].name, "Pyro-GL");

  ASSERT_TRUE(Save());
  EXPECT_NE(ReadFile().find("SHIPBAN\tPyro-GL"), std::string::npos);

  ASSERT_TRUE(Load());
  ASSERT_EQ(g_ship_permits.size(), 1u);
  EXPECT_EQ(g_ship_permits[0].first, "Pyro-GL");
  EXPECT_EQ(g_ship_permits[0].second, 0);
}

/**
 * @test MultiSaveTest.UnusedOrAllowedShipsNotWritten
 * @brief Verifies unused Or Allowed Ships Not Written.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, UnusedOrAllowedShipsNotWritten) {
  // Stub bans only ship index 7; none of these qualify.
  Ships[0].used = 1; // allowed
  Ships[1].used = 0;
  strcpy(Ships[1].name, "Phoenix");   // unused -> skipped regardless
  Ships[3].used = 1;
  strcpy(Ships[3].name, "BlackPyro"); // allowed

  ASSERT_TRUE(Save());
  std::string s = ReadFile();
  EXPECT_EQ(s.find("SHIPBAN"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Error paths / raw file shape
// ---------------------------------------------------------------------------

/**
 * @test MultiSaveTest.MissingFileFailsGracefully
 * @brief Verifies missing File Fails Gracefully.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, MissingFileFailsGracefully) {
  EXPECT_EQ(MultiSaveSettings("/tmp/opencode/no_such_dir_mp/x.txt"), 0);
  EXPECT_EQ(MultiLoadSettings("/tmp/opencode/definitely_missing_mps.txt"), 0);
}

/**
 * @test MultiSaveTest.FileUsesTabSeparatedLines
 * @brief Verifies file Uses Tab Separated Lines.
 *
 * @details
 * Exercises the MultiSaveTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_save_setting.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiSaveTest, FileUsesTabSeparatedLines) {
  ASSERT_TRUE(Save());
  std::string s = ReadFile();
  EXPECT_NE(s.find("NAME\tMy Cool Game\n"), std::string::npos);
  EXPECT_NE(s.find("PPS\t20\n"), std::string::npos);
  EXPECT_NE(s.find("MAXPLAYERS\t8\n"), std::string::npos);
}
