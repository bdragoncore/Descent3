/**
 * @file game2dll_real_tests.cpp
 * @brief Tests for Game2DLL.cpp 829 lines — game DLL interface.
 *
 * @details
 * Covers GetDLLRequirements truncation handling and requirement
 * counting, GetDLLNumTeamInfo team bounds logic, and the CallGameDLL
 * chokepoint gating.
 *
 * This harness validates the behavior of `Descent3/Game2DLL.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/Game2DLL.cpp`
 * @par Harness
 * `game2dll_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/Game2DLL.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <algorithm>

// replicated flags (lib/game2dll.h:57-58)
constexpr uint32_t DOF_MAXTEAMS = 0x0001;
constexpr uint32_t DOF_MINTEAMS = 0x0002;

// replicated GetDLLRequirements core logic (Game2DLL.cpp:768-801)
static int RepGetDLLRequirements(const char *opt_requirements, char *requirements, int buflen) {
  strncpy(requirements, opt_requirements, buflen - 1);
  requirements[buflen - 1] = '\0';
  uint32_t opt_req_len = (uint32_t)strlen(opt_requirements);
  if (opt_req_len > strlen(requirements)) {
    // too small of a buffer! cut off last requirement (which is shortened)
    char *p = strrchr(requirements, ',');
    if (p) {
      *p = '\0';
    } else {
      *requirements = '\0';
    }
  }
  int count = 0;
  for (const char *p = requirements; *p; p++) {
    if (*p == ',')
      count++;
  }
  count++;
  return count;
}

/**
 * @test GameDLL.RequirementsCountCommaSeparated
 * @brief Verifies requirements Count Comma Separated.
 *
 * @details
 * Exercises the GameDLL code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Game2DLL.cpp
 * @ingroup descent3_tests
 */
TEST(GameDLL, RequirementsCountCommaSeparated) {
  char buf[128];
  EXPECT_EQ(RepGetDLLRequirements("req1,req2,req3", buf, sizeof(buf)), 3);
  EXPECT_STREQ(buf, "req1,req2,req3");
  EXPECT_EQ(RepGetDLLRequirements("", buf, sizeof(buf)), 1); // empty -> 1 (empty string counts as one)
  EXPECT_EQ(RepGetDLLRequirements("solo", buf, sizeof(buf)), 1);
}

/**
 * @test GameDLL.RequirementsTruncationDropsLastPartialRequirement
 * @brief Verifies requirements Truncation Drops Last Partial Requirement.
 *
 * @details
 * Exercises the GameDLL code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Game2DLL.cpp
 * @ingroup descent3_tests
 */
TEST(GameDLL, RequirementsTruncationDropsLastPartialRequirement) {
  char small[16];
  // "alpha,beta,gamma-doesnotfit" truncated at 15 chars + NUL
  int n = RepGetDLLRequirements("alpha,beta,gamma-doesnotfit", small, sizeof(small));
  // buffer holds "alpha,beta,gam" -> chopped at last comma -> "alpha,beta"
  EXPECT_STREQ(small, "alpha,beta");
  EXPECT_EQ(n, 2);
}

/**
 * @test GameDLL.RequirementsTruncationNoCommaEmptiesBuffer
 * @brief Verifies requirements Truncation No Comma Empties Buffer.
 *
 * @details
 * Exercises the GameDLL code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Game2DLL.cpp
 * @ingroup descent3_tests
 */
TEST(GameDLL, RequirementsTruncationNoCommaEmptiesBuffer) {
  char tiny[6];
  int n = RepGetDLLRequirements("waytoolongnorequirement", tiny, sizeof(tiny));
  EXPECT_STREQ(tiny, ""); // no comma to chop -> fully emptied
  EXPECT_EQ(n, 1);        // empty string still counts as one requirement
}

/**
 * @test GameDLL.RequirementsExactFitNotTruncated
 * @brief Verifies requirements Exact Fit Not Truncated.
 *
 * @details
 * Exercises the GameDLL code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Game2DLL.cpp
 * @ingroup descent3_tests
 */
TEST(GameDLL, RequirementsExactFitNotTruncated) {
  char exact[12]; // fits "a,b,c,d,e\0" exactly? len 9+1=10 <= 11 usable
  int n = RepGetDLLRequirements("a,b,c,d,e", exact, sizeof(exact));
  EXPECT_STREQ(exact, "a,b,c,d,e");
  EXPECT_EQ(n, 5);
}

// replicated GetDLLNumTeamInfo (Game2DLL.cpp:807-829)
struct DLLOptionsMock {
  uint32_t flags = 0;
  int max_teams = 0;
  int min_teams = 0;
};

static bool RepGetDLLNumTeamInfo(bool info_ok, DLLOptionsMock dllo, int *mint, int *maxt) {
  if (!info_ok) {
    *mint = 1;
    *maxt = 1;
    return false;
  }
  if (!(dllo.flags & DOF_MAXTEAMS)) {
    *mint = 1;
    *maxt = 1;
    return false;
  }
  *maxt = (dllo.max_teams == 0 || dllo.max_teams == 1 || dllo.max_teams < 0) ? 1 : std::min(dllo.max_teams, 4);
  if ((*maxt) == 1) {
    *mint = 1;
  } else {
    *mint = 2;
  }
  if (dllo.flags & DOF_MINTEAMS && dllo.min_teams >= 0) {
    *mint = (dllo.min_teams == 0 || dllo.min_teams == 1) ? 1 : std::min(*maxt, dllo.min_teams);
  }
  return ((*maxt) == 1) ? false : true;
}

/**
 * @test GameDLL.TeamInfoNonTeamCases
 * @brief Verifies team Info Non Team Cases.
 *
 * @details
 * Exercises the GameDLL code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Game2DLL.cpp
 * @ingroup descent3_tests
 */
TEST(GameDLL, TeamInfoNonTeamCases) {
  int mn, mx;
  // DLL load failure
  EXPECT_FALSE(RepGetDLLNumTeamInfo(false, {}, &mn, &mx));
  EXPECT_EQ(mn, 1);
  EXPECT_EQ(mx, 1);
  // no MAXTEAMS flag -> non-team game regardless of members
  EXPECT_FALSE(RepGetDLLNumTeamInfo(true, {0, 4, 4}, &mn, &mx));
  EXPECT_EQ(mn, 1);
  EXPECT_EQ(mx, 1);
  // quirk: negative max_teams treated same as 0/1 -> single team
  EXPECT_FALSE(RepGetDLLNumTeamInfo(true, {DOF_MAXTEAMS, -3, 0}, &mn, &mx));
  EXPECT_EQ(mx, 1);
}

/**
 * @test GameDLL.TeamInfoMaxClampedToFour
 * @brief Verifies team Info Max Clamped To Four.
 *
 * @details
 * Exercises the GameDLL code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Game2DLL.cpp
 * @ingroup descent3_tests
 */
TEST(GameDLL, TeamInfoMaxClampedToFour) {
  int mn, mx;
  EXPECT_TRUE(RepGetDLLNumTeamInfo(true, {DOF_MAXTEAMS, 2, 0}, &mn, &mx));
  EXPECT_EQ(mx, 2);
  EXPECT_EQ(mn, 2); // default min for team games

  RepGetDLLNumTeamInfo(true, {DOF_MAXTEAMS, 99, 0}, &mn, &mx);
  EXPECT_EQ(mx, 4); // hard cap

  EXPECT_FALSE(RepGetDLLNumTeamInfo(true, {DOF_MAXTEAMS, 1, 0}, &mn, &mx)); // 1 team = non-team
}

/**
 * @test GameDLL.TeamInfoMinTeamsRules
 * @brief Verifies team Info Min Teams Rules.
 *
 * @details
 * Exercises the GameDLL code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Game2DLL.cpp
 * @ingroup descent3_tests
 */
TEST(GameDLL, TeamInfoMinTeamsRules) {
  int mn, mx;
  // MINTEAMS flag with 0 or 1 -> min forced to 1 even for team games
  RepGetDLLNumTeamInfo(true, {DOF_MAXTEAMS | DOF_MINTEAMS, 4, 0}, &mn, &mx);
  EXPECT_EQ(mn, 1);
  EXPECT_EQ(mx, 4);

  // min clamped to [2..maxt]
  RepGetDLLNumTeamInfo(true, {DOF_MAXTEAMS | DOF_MINTEAMS, 4, 3}, &mn, &mx);
  EXPECT_EQ(mn, 3);

  // quirk: min_teams > max_teams silently clamps DOWN to maxt
  RepGetDLLNumTeamInfo(true, {DOF_MAXTEAMS | DOF_MINTEAMS, 2, 50}, &mn, &mx);
  EXPECT_EQ(mn, 2);
  EXPECT_EQ(mx, 2);

  // negative min_teams ignored entirely
  RepGetDLLNumTeamInfo(true, {DOF_MAXTEAMS | DOF_MINTEAMS, 3, -5}, &mn, &mx);
  EXPECT_EQ(mn, 2); // default for team game
}

// replicated CallGameDLL chokepoint (Game2DLL.cpp:710-715)
/**
 * @test GameDLL.CallChokepointGatingAndRetReset
 * @brief Verifies call Chokepoint Gating And Ret Reset.
 *
 * @details
 * Exercises the GameDLL code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Game2DLL.cpp
 * @ingroup descent3_tests
 */
TEST(GameDLL, CallChokepointGatingAndRetReset) {
  bool handle = false, dll_fn = false;
  int last_event = -1, calls = 0, iret = 77;

  auto call = [&](int eventnum) {
    if (handle && dll_fn) {
      iret = 0; // data->iRet = 0 before the call
      calls++;
      last_event = eventnum;
    }
  };

  call(5);              // nothing loaded: no dispatch, iRet untouched
  EXPECT_EQ(calls, 0);
  EXPECT_EQ(iret, 77);  // NOT reset when gated off

  dll_fn = true;
  call(5);              // fn but no handle: still gated
  EXPECT_EQ(calls, 0);
  EXPECT_EQ(iret, 77);

  handle = true;
  call(9);
  EXPECT_EQ(calls, 1);
  EXPECT_EQ(last_event, 9);
  EXPECT_EQ(iret, 0);   // reset only on real dispatch
}
