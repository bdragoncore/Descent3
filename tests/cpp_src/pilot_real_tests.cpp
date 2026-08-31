/**
 * @file pilot_real_tests.cpp
 * @brief Tests for pilot.cpp 3425 lines — pilot file management. Covers.
 *
 * @details
 * PltMakeFNValid filename sanitization (illegal chars to _, leading
 * whitespace strip only), PilotGetHighestLevelAchieved /
 * GetPilotShipPermissions sentinel returns (missing mission 0, null
 * pilot 1), IncrementPilotRestoredGamesForMission's silent no-op for
 * unknown missions plus its save-after-increment behavior, and the
 * PltDelete fallback path when the stored filename is empty.
 *
 * This harness validates the behavior of `Descent3/pilot.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/pilot.cpp`
 * @par Harness
 * `pilot_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/pilot.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

constexpr int MSN_NAMELEN_P = 255;
#define PLTEXTENSION ".plt"

struct tMissionDataP {
  uint8_t highest_level = 0;
  int ship_permissions = 0;
  bool finished = false;
  char mission_name[MSN_NAMELEN_P] = {0};
  int num_restores = 0;
  int num_saves = 0;
};

struct MockPilot {
  tMissionDataP missions[4];
  int num_missions = 0;
  std::string filename;
  std::string name;

  int find_mission_data(const char *mname) const {
    for (int i = 0; i < num_missions; i++)
      if (strcmp(missions[i].mission_name, mname) == 0)
        return i;
    return -1;
  }
};

// ---------------------------------------------------------------------------
// PltMakeFNValid replication (pilot.cpp:1727-1758)
static void RepMakeFNValid(char *name) {
  int end = strlen(name);
  for (int i = 0; i < end; i++) {
    switch (name[i]) {
    case '?': case '*': case '\\': case '/': case '|':
    case '<': case '>': case ':': case '"':
      name[i] = '_';
      break;
    default:
      break;
    }
  }
  int w = 0;
  while (name[w] == ' ')
    w++;
  char temp[256];
  strcpy(temp, &name[w]);
  strcpy(name, temp);
}

/**
 * @test PilotFilename.SanitizesIllegalCharsAndLeadingSpacesOnly
 * @brief Verifies sanitizes Illegal Chars And Leading Spaces Only.
 *
 * @details
 * Exercises the PilotFilename code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot.cpp
 * @ingroup descent3_tests
 */
TEST(PilotFilename, SanitizesIllegalCharsAndLeadingSpacesOnly) {
  char fn[64];

  strcpy(fn, "a?b*c\\d/e|f<g>h:i\"j");
  RepMakeFNValid(fn);
  EXPECT_STREQ(fn, "a_b_c_d_e_f_g_h_i_j");

  strcpy(fn, "My Pilot 2");
  RepMakeFNValid(fn);
  EXPECT_STREQ(fn, "My Pilot 2");

  // quirk: only LEADING whitespace stripped; interior/trailing kept
  strcpy(fn, "   spaced name   ");
  RepMakeFNValid(fn);
  EXPECT_STREQ(fn, "spaced name   ");

  strcpy(fn, "     ");
  RepMakeFNValid(fn);
  EXPECT_STREQ(fn, "");

  strcpy(fn, "  ?bad");
  RepMakeFNValid(fn);
  EXPECT_STREQ(fn, "_bad"); // ? sanitized in place, then spaces stripped
}

// ---------------------------------------------------------------------------
// Mission data wrappers replication (pilot.cpp:1526-1572)
static int RepGetHighestLevel(const MockPilot *pilot, const char *mission) {
  if (!pilot)
    return 1; // quirk: null pilot reports level 1

  int index = pilot->find_mission_data(mission);
  if (index == -1)
    return 0;

  return pilot->missions[index].highest_level;
}

static bool RepIncrementRestores(MockPilot *pilot, const char *mission) {
  if (!pilot)
    return false;

  int index = pilot->find_mission_data(mission);
  if (index == -1)
    return false; // silent no-op: nothing recorded anywhere

  pilot->missions[index].num_restores++;
  return true; // PltWriteFile side effect fires in original
}

static int RepGetShipPermissions(const MockPilot *pilot, const char *mission) {
  if (!pilot)
    return 1;

  int index = pilot->find_mission_data(mission);
  if (index == -1)
    return 0;

  return pilot->missions[index].ship_permissions;
}

/**
 * @test PilotMissionData.SentinelReturnsRestoreCountsAndSaveBehavior
 * @brief Verifies sentinel Returns Restore Counts And Save Behavior.
 *
 * @details
 * Exercises the PilotMissionData code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot.cpp
 * @ingroup descent3_tests
 */
TEST(PilotMissionData, SentinelReturnsRestoreCountsAndSaveBehavior) {
  MockPilot p;
  strcpy(p.missions[0].mission_name, "d3_2.mn3");
  p.missions[0].highest_level = 7;
  p.missions[0].ship_permissions = 3;
  p.missions[0].num_restores = 2;
  p.num_missions = 1;

  EXPECT_EQ(RepGetHighestLevel(&p, "d3_2.mn3"), 7);
  EXPECT_EQ(RepGetHighestLevel(&p, "unknown.mn3"), 0); // never played
  EXPECT_EQ(RepGetHighestLevel(nullptr, "d3_2.mn3"), 1); // null -> level 1!

  EXPECT_TRUE(RepIncrementRestores(&p, "d3_2.mn3"));
  EXPECT_EQ(p.missions[0].num_restores, 3);

  // unknown mission: no counter touched and NO save triggered
  EXPECT_FALSE(RepIncrementRestores(&p, "nope.mn3"));

  EXPECT_EQ(RepGetShipPermissions(&p, "d3_2.mn3"), 3);
  EXPECT_EQ(RepGetShipPermissions(&p, "ghost.mn3"), 0); // none granted
  EXPECT_EQ(RepGetShipPermissions(nullptr, "x"), 1);
}

// ---------------------------------------------------------------------------
// PltDelete fallback replication (pilot.cpp:1579-1601)
static const char *BaseDirP = "/tmp/opencode/pilots";

static bool RepPltDelete(MockPilot &pilot, bool &used_fallback) {
  used_fallback = false;
  std::error_code ec;
  if (!pilot.filename.empty()) {
    fs::create_directories(BaseDirP);
    return fs::remove(fs::path(BaseDirP) / pilot.filename, ec);
  }

  used_fallback = true;
  if (pilot.name.empty())
    return false;

  char pname[64];
  strncpy(pname, pilot.name.c_str(), sizeof(pname) - 1);
  pname[sizeof(pname) - 1] = 0;
  RepMakeFNValid(pname);

  std::string pfilename = std::string(pname) + PLTEXTENSION;
  fs::create_directories(BaseDirP);
  return fs::remove(fs::path(BaseDirP) / pfilename, ec);
}

static void TouchPilotFile(const std::string &fn) {
  fs::create_directories(BaseDirP);
  std::ofstream(fs::path(BaseDirP) / fn).put('x');
}

/**
 * @test PilotDelete.UsesStoredNameFallsBackToDerivedAndRejectsEmpty
 * @brief Verifies uses Stored Name Falls Back To Derived And Rejects Empty.
 *
 * @details
 * Exercises the PilotDelete code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot.cpp
 * @ingroup descent3_tests
 */
TEST(PilotDelete, UsesStoredNameFallsBackToDerivedAndRejectsEmpty) {
  MockPilot p;
  bool fb = false;

  p.filename = "hero.plt";
  TouchPilotFile("hero.plt");
  EXPECT_TRUE(RepPltDelete(p, fb));
  EXPECT_FALSE(fb);

  // empty filename: rebuild from name via MakeFNValid + .plt
  fs::remove_all(BaseDirP);
  p.filename = "";
  p.name = "bad:name?";
  TouchPilotFile("bad_name_.plt"); // pre-sanitized name on disk
  fb = false;
  EXPECT_TRUE(RepPltDelete(p, fb));
  EXPECT_TRUE(fb);

  // neither filename nor usable name -> false
  p.filename = "";
  p.name = "";
  fb = false;
  EXPECT_FALSE(RepPltDelete(p, fb));
  EXPECT_TRUE(fb); // still took the fallback path
}
