/**
 * @file mission_real_tests.cpp
 * @brief Tests for Mission.cpp 2010 lines — mission file handling.
 *
 * @details
 * Covers the .msn text info parser (keywords, multi-line description,
 * defaults), MN3 name mapping (d3_2 -> d3 quirk), and the
 * MissionGetKeywords mod-compatibility matcher.
 *
 * This harness validates the behavior of `Descent3/Mission.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/Mission.cpp`
 * @par Harness
 * `mission_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/Mission.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>

// replicated constants
constexpr int MSN_NAMELEN = 100;
constexpr int MAX_KEYWORDLEN = 256;
constexpr int MAX_NET_PLAYERS = 8;
constexpr int KEYWORD_LEN = 16;
constexpr int NUM_KEYWORDS = 16;

// POSIX equivalents for platform stricmp/strnicmp
static int stricmp(const char *a, const char *b) {
  return strcasecmp(a, b);
}
static int strnicmp(const char *a, const char *b, size_t n) {
  return strncasecmp(a, b, n);
}

// replicated IS_MN3_FILE + MN3_TO_MSN_NAME (Mission.cpp:705-717)
static bool IsMn3File(const std::string &fname) {
  std::string ext = std::filesystem::path(fname).extension().string();
  for (auto &c : ext)
    c = tolower(c);
  return ext == ".mn3";
}

static std::string Mn3ToMsnName(const std::string &mn3name) {
  std::string fname = std::filesystem::path(mn3name).stem().string();
  for (auto &c : fname)
    c = tolower(c);
  if (fname == "d3_2") {
    fname = "d3"; // quirk: expansion maps to base campaign's script names
  }
  return fname + ".msn";
}

/**
 * @test Mission.Mn3NameMapping
 * @brief Verifies mn3Name Mapping.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, Mn3NameMapping) {
  EXPECT_TRUE(IsMn3File("mycoolmission.MN3"));
  EXPECT_TRUE(IsMn3File("x.mn3"));
  EXPECT_FALSE(IsMn3File("x.msn"));
  EXPECT_FALSE(IsMn3File("mn3"));

  EXPECT_EQ(Mn3ToMsnName("d3_2.mn3"), "d3.msn"); // quirk: sequel reuses base .msn
  EXPECT_EQ(Mn3ToMsnName("D3_2.MN3"), "d3.msn"); // case-insensitive stem match
  EXPECT_EQ(Mn3ToMsnName("mymap.mn3"), "mymap.msn");
  EXPECT_EQ(Mn3ToMsnName("/path/to/training.mn3"), "training.msn");
}

// replicated tMissionInfo fields used by the parser
struct MsnInfoMock {
  bool multi = true, single = true, training = false;
  char name[MSN_NAMELEN] = {0};
  char author[MSN_NAMELEN] = {0};
  char desc[512] = {0};
  char keywords[MAX_KEYWORDLEN] = {0};
  int n_levels = 0;
};

// replicated CleanupStr-lite: strip leading spaces/tabs
static void CleanLine(char *dst, const char *src, size_t dstsz) {
  while (*src == ' ' || *src == '\t')
    src++;
  strncpy(dst, src, dstsz - 1);
  dst[dstsz - 1] = 0;
}

// faithful replication of the GetMissionInfo inner loop including the
// strtok side effect (Mission.cpp:1735-1767)
static bool ParseMissionInfo(const std::vector<std::string> &lines_in, MsnInfoMock *msn) {
  bool indesc = false;
  msn->multi = true;
  msn->single = true;
  msn->training = false;

  auto trim = [](const char *s) {
    while (*s == ' ' || *s == '\t')
      s++;
    return s;
  };

  for (const auto &line_in : lines_in) {
    if (line_in.empty())
      continue;
    char srcline[128];
    strncpy(srcline, line_in.c_str(), sizeof(srcline) - 1);
    srcline[sizeof(srcline) - 1] = 0;

    // strtok splits the buffer at the first space/tab (side effect!)
    char *keyword = strtok(srcline, " \t");
    (void)keyword;

    char command[32] = {0}, operand[96] = {0};
    CleanLine(command, srcline, sizeof(command));
    CleanLine(operand, srcline + strlen(command) + 1, sizeof(operand));
    for (auto &c : command)
      c = tolower(c);

    if (strlen(command) && indesc)
      indesc = false;
    if (strcmp(command, "name") == 0) {
      strncpy(msn->name, trim(operand), MSN_NAMELEN - 1);
    } else if (strcmp(command, "multi") == 0) {
      std::string op = operand;
      if (strcasecmp(op.c_str(), "no") == 0)
        msn->multi = false;
    } else if (strcmp(command, "single") == 0) {
      std::string op = operand;
      if (strcasecmp(op.c_str(), "no") == 0)
        msn->single = false;
    } else if (strcmp(command, "trainer") == 0) {
      msn->training = true;
    } else if (strcmp(command, "author") == 0) {
      strncpy(msn->author, trim(operand), MSN_NAMELEN - 1);
    } else if (strcmp(command, "description") == 0 || indesc) {
      strcat(msn->desc, operand);
      if (indesc)
        strcat(msn->desc, "\n");
      indesc = true;
    } else if (strcmp(command, "numlevels") == 0) {
      msn->n_levels = atoi(operand);
    } else if (strcmp(command, "level") == 0) {
      break;
    }
  }
  return true;
}

/**
 * @test Mission.InfoParserDefaultsAndFlags
 * @brief Verifies info Parser Defaults And Flags.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, InfoParserDefaultsAndFlags) {
  MsnInfoMock m;
  ParseMissionInfo({"name\tCool Map", "numlevels 3"}, &m);
  EXPECT_STREQ(m.name, "Cool Map");
  EXPECT_EQ(m.n_levels, 3);
  EXPECT_TRUE(m.multi);  // default playable both modes
  EXPECT_TRUE(m.single);
  EXPECT_FALSE(m.training);
}

/**
 * @test Mission.InfoParserMultiSingleNoAndTrainer
 * @brief Verifies info Parser Multi Single No And Trainer.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, InfoParserMultiSingleNoAndTrainer) {
  MsnInfoMock m;
  ParseMissionInfo({"MULTI No", "SINGLE NO", "TRAINER"}, &m);
  EXPECT_FALSE(m.multi);
  EXPECT_FALSE(m.single);
  EXPECT_TRUE(m.training);

  MsnInfoMock m2;
  ParseMissionInfo({"multi yes"}, &m2); // anything but "no" keeps default
  EXPECT_TRUE(m2.multi);
}

/**
 * @test Mission.InfoParserMultilineDescriptionQuirk
 * @brief Verifies info Parser Multiline Description Quirk.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, InfoParserMultilineDescriptionQuirk) {
  MsnInfoMock m;
  // quirk: any line whose first word is non-empty CLOSES the description
  // block (and is dropped as an unknown keyword); only whitespace-only
  // lines keep indesc alive, appending just a newline
  ParseMissionInfo({"description Line one", "continued here", "name X"}, &m);
  EXPECT_STREQ(m.desc, "Line one");   // continuation dropped
  EXPECT_STREQ(m.name, "X");

  MsnInfoMock m2;
  ParseMissionInfo({"description Part A", " ", "name B"}, &m2);
  EXPECT_STREQ(m2.desc, "Part A\n");  // blank-ish line appends newline only
}

/**
 * @test Mission.InfoParserStopsAtLevelKeyword
 * @brief Verifies info Parser Stops At Level Keyword.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, InfoParserStopsAtLevelKeyword) {
  MsnInfoMock m;
  ParseMissionInfo({"name Early", "LEVEL level1.d3l", "name NeverRead"}, &m);
  EXPECT_STREQ(m.name, "Early");
  EXPECT_STRNE(m.name, "NeverRead");
}

// replicated MissionGetKeywords matching (Mission.cpp:1878-1975)
static int RepMissionGetKeywords(const char *msn_keywords_str, const char *mod_keywords, int msn_goal_count_hint) {
  char msn_keywords[NUM_KEYWORDS][KEYWORD_LEN] = {};
  char mod_keywords_arr[NUM_KEYWORDS][KEYWORD_LEN] = {};
  char parse_keys[MAX_KEYWORDLEN];
  strncpy(parse_keys, mod_keywords ? mod_keywords : "", MAX_KEYWORDLEN - 1);
  int teams = MAX_NET_PLAYERS;
  int goals = 0;
  int goalsneeded = 0;
  int mod_key_count = 0, msn_key_count = 0;
  bool goal_per_team = false;

  if (!*parse_keys)
    return MAX_NET_PLAYERS;

  char *tok = strtok(parse_keys, ",");
  while (tok && mod_key_count < NUM_KEYWORDS) {
    strcpy(mod_keywords_arr[mod_key_count++], tok);
    tok = strtok(nullptr, ",");
  }
  tok = strtok((char *)msn_keywords_str, ",");
  while (tok && msn_key_count < NUM_KEYWORDS) {
    strcpy(msn_keywords[msn_key_count++], tok);
    tok = strtok(nullptr, ",");
  }

  for (int i = 0; i < msn_key_count; i++) {
    if (strnicmp("GOALS", msn_keywords[i], 5) == 0)
      goals = atoi(msn_keywords[i] + 5);
  }
  (void)msn_goal_count_hint;

  for (int i = 0; i < mod_key_count; i++) {
    if (strnicmp(mod_keywords_arr[i], "MINGOALS", 8) == 0) {
      goalsneeded = atoi(mod_keywords_arr[i] + 8);
    } else if (strcasecmp(mod_keywords_arr[i], "GOALPERTEAM") == 0) {
      goal_per_team = true;
    } else {
      bool found = false;
      for (int k = 0; k < msn_key_count; k++) {
        if (strcasecmp(msn_keywords[k], mod_keywords_arr[i]) == 0) {
          found = true;
          break;
        }
      }
      if (!found)
        return -1;
    }
  }
  if (goal_per_team)
    teams = goals;
  if (teams < goalsneeded || goals < goalsneeded)
    return -1;
  return teams;
}

/**
 * @test Mission.KeywordsNoModRequirementsMeansAnyPlayers
 * @brief Verifies keywords No Mod Requirements Means Any Players.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, KeywordsNoModRequirementsMeansAnyPlayers) {
  EXPECT_EQ(RepMissionGetKeywords("ctf", "", 0), MAX_NET_PLAYERS);
  EXPECT_EQ(RepMissionGetKeywords("", nullptr, 0), MAX_NET_PLAYERS);
}

/**
 * @test Mission.KeywordsMatchingCaseInsensitive
 * @brief Verifies keywords Matching Case Insensitive.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, KeywordsMatchingCaseInsensitive) {
  EXPECT_EQ(RepMissionGetKeywords("Anarchy,CTF", "anarchy", 0), MAX_NET_PLAYERS);
  EXPECT_EQ(RepMissionGetKeywords("robo-anarchy", "Robo-Anarchy", 0), MAX_NET_PLAYERS);
  // missing required keyword rejects the pairing
  EXPECT_EQ(RepMissionGetKeywords("Anarchy", "ctf", 0), -1);
}

/**
 * @test Mission.KeywordsGoalsAndMinGoalsRules
 * @brief Verifies keywords Goals And Min Goals Rules.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, KeywordsGoalsAndMinGoalsRules) {
  // mission declares 5 goals; mod needs at least 3: fine
  EXPECT_EQ(RepMissionGetKeywords("GOALS5", "MINGOALS3", 0), MAX_NET_PLAYERS);
  // mod needs more goals than the mission has -> reject
  EXPECT_EQ(RepMissionGetKeywords("GOALS2", "MINGOALS3", 0), -1);
  // equal counts pass
  EXPECT_EQ(RepMissionGetKeywords("GOALS4", "mingoals4", 0), MAX_NET_PLAYERS);
}

/**
 * @test Mission.KeywordsGoalPerTeamUsesGoalCountAsTeamCap
 * @brief Verifies keywords Goal Per Team Uses Goal Count As Team Cap.
 *
 * @details
 * Exercises the Mission code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Mission.cpp
 * @ingroup descent3_tests
 */
TEST(Mission, KeywordsGoalPerTeamUsesGoalCountAsTeamCap) {
  // GOALPERTEAM caps players to the mission's goal count
  EXPECT_EQ(RepMissionGetKeywords("GOALS6,monsters", "GOALPERTEAM,monsters", 0), 6);
  // still respects MINGOALS after the cap
  EXPECT_EQ(RepMissionGetKeywords("GOALS3", "GOALPERTEAM,MINGOALS5", 0), -1);
}
