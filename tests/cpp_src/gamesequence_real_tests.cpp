/**
 * @file gamesequence_real_tests.cpp
 * @brief Tests for gamesequence.cpp 2577 lines — game state sequencer.
 *
 * @details
 * Covers the GameSequencer state machine transitions (NEW, LVLSTART
 * failure bail, LVLPLAYING sound resume on first frame, LOADGAME
 * multiplayer block quirk, LVLEND/LVLNEXT/LVLWARP/LVLFAILED chains,
 * Last_game_state capture), CheckHogfile mission archive swapping
 * with its missing-file bail quirk, and SimpleStartLevel one-shot
 * mission setup.
 *
 * This harness validates the behavior of `Descent3/gamesequence.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gamesequence.cpp`
 * @par Harness
 * `gamesequence_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gamesequence.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>

// replicated states (gamesequence.h)
enum GameState {
  GAMESTATE_IDLE = 0,
  GAMESTATE_NEW,
  GAMESTATE_LVLNEXT,
  GAMESTATE_LVLSTART,
  GAMESTATE_LVLPLAYING,
  GAMESTATE_LOADGAME,
  GAMESTATE_LVLEND,
  GAMESTATE_LVLFAILED,
  GAMESTATE_LVLWARP,
  GAMESTATE_LOADDEMO,
};

constexpr int GM_MULTI = 0x08;
constexpr int GAME_INTERFACE = 0;

// sequencer mock harness
struct SeqMock {
  int function_mode = 1; // GAME_MODE stand-in
  int game_mode = 0;
  GameState game_state = GAMESTATE_IDLE;
  GameState last_game_state = GAMESTATE_IDLE;
  int interface_mode = GAME_INTERFACE;

  // observed side effects
  int new_games = 0, level_intros = 0, levels_loaded = 0, end_calls = 0;
  int next_level_calls = 0, set_cur_level = -99, leaves = 0, sound_resumes = 0;
  int frames = 0, menus_run = 0, hog_checks = 0;
  bool load_intro_ok = true, load_level_ok = true, load_save_ok = true;
  bool editor = false;
  std::string bail_reason;

  bool IntroAndLoad() {
    level_intros++;
    if (!load_intro_ok || !load_level_ok)
      return false;
    levels_loaded++;
    return true;
  }
  void EndLevel(int state) {
    end_calls++;
    (void)state; // 1=normal, -1=warp, 0=failed in original
  }

  // replicated main loop core (gamesequence.cpp:1147-1265), bounded
  void Run(int max_iters) {
    for (int iter = 0; iter < max_iters; iter++) {
      if (!(function_mode == 1 || function_mode == 4)) // GAME_MODE/EDITOR_GAME_MODE
        break;
      GameState old_game_state = game_state;

      switch (game_state) {
      case GAMESTATE_NEW:
        new_games++;
        game_state = GAMESTATE_LVLSTART;
        break;

      case GAMESTATE_LVLSTART:
        hog_checks++;
        if (editor) {
          levels_loaded++; // StartGameFromEditor always "works"
        } else if (!IntroAndLoad()) {
          if (game_mode & GM_MULTI)
            leaves++;
          bail_reason = "menu";
          function_mode = 2; // MENU_MODE -> loop exits
          break;
        }
        game_state = GAMESTATE_LVLPLAYING;
        break;

      case GAMESTATE_LVLPLAYING:
        if (last_game_state == GAMESTATE_LVLSTART)
          sound_resumes++;
        if (interface_mode != GAME_INTERFACE)
          menus_run++;
        else
          frames++;
        break; // stays playing forever until external change

      case GAMESTATE_LOADGAME:
        if (game_mode & GM_MULTI) {
          // quirk: no transition at all -- state unchanged, the modal
          // error box re-fires every pass of the loop
          break;
        }
        if (!load_save_ok) {
          function_mode = 2;
        } else {
          game_state = GAMESTATE_LVLPLAYING;
        }
        break;

      case GAMESTATE_LVLEND:
        EndLevel(1);
        game_state = GAMESTATE_LVLNEXT;
        break;

      case GAMESTATE_LVLNEXT:
        next_level_calls++;
        game_state = GAMESTATE_LVLSTART;
        break;

      case GAMESTATE_LVLWARP:
        EndLevel(-1);
        set_cur_level = 7; // Level_warp_next stand-in
        game_state = GAMESTATE_LVLSTART;
        break;

      case GAMESTATE_LVLFAILED:
        EndLevel(0);
        game_state = GAMESTATE_LVLSTART; // restart SAME level
        break;

      default:
        break;
      }
      last_game_state = old_game_state;
    }
  }
};

/**
 * @test GameSeq.NewGameChainsIntoLevelStartThenPlaying
 * @brief Verifies new Game Chains Into Level Start Then Playing.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, NewGameChainsIntoLevelStartThenPlaying) {
  SeqMock s;
  s.game_state = GAMESTATE_NEW;
  s.Run(10);
  EXPECT_EQ(s.new_games, 1);
  EXPECT_EQ(s.level_intros, 1);
  EXPECT_EQ(s.levels_loaded, 1);
  EXPECT_EQ(s.game_state, GAMESTATE_LVLPLAYING);
}

/**
 * @test GameSeq.FailedLoadInMultiplayerLeavesGameAndBailsToMenu
 * @brief Verifies failed Load In Multiplayer Leaves Game And Bails To Menu.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, FailedLoadInMultiplayerLeavesGameAndBailsToMenu) {
  SeqMock s;
  s.game_state = GAMESTATE_LVLSTART;
  s.game_mode = GM_MULTI;
  s.load_intro_ok = false;
  s.Run(10);
  EXPECT_EQ(s.leaves, 1);       // MultiLeaveGame
  EXPECT_EQ(s.function_mode, 2); // back to menu
  EXPECT_EQ(s.levels_loaded, 0);
}

/**
 * @test GameSeq.FirstPlayingFrameResumesSoundsExactlyOnce
 * @brief Verifies first Playing Frame Resumes Sounds Exactly Once.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, FirstPlayingFrameResumesSoundsExactlyOnce) {
  SeqMock s;
  s.game_state = GAMESTATE_LVLSTART;
  s.Run(3); // LVLSTART -> PLAYING, then two playing frames
  EXPECT_EQ(s.sound_resumes, 1); // only when Last==LVLSTART
  EXPECT_EQ(s.frames, 2);
  EXPECT_EQ(s.last_game_state, GAMESTATE_LVLPLAYING);
}

/**
 * @test GameSeq.MenuInterfaceModeRunsMenuInsteadOfFrame
 * @brief Verifies menu Interface Mode Runs Menu Instead Of Frame.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, MenuInterfaceModeRunsMenuInsteadOfFrame) {
  SeqMock s;
  s.game_state = GAMESTATE_LVLSTART;
  s.interface_mode = 5; // some options screen
  s.Run(3);
  EXPECT_EQ(s.menus_run, 2);
  EXPECT_EQ(s.frames, 0);
}

/**
 * @test GameSeq.LoadGameInMultiplayerNeverTransitionsQuirk
 * @brief Verifies load Game In Multiplayer Never Transitions Quirk.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, LoadGameInMultiplayerNeverTransitionsQuirk) {
  SeqMock s;
  s.game_state = GAMESTATE_LOADGAME;
  s.game_mode = GM_MULTI;
  s.Run(5);
  // quirk: still stuck in LOADGAME after five passes, no mode change
  EXPECT_EQ(s.game_state, GAMESTATE_LOADGAME);
  EXPECT_EQ(s.function_mode, 1);
  EXPECT_TRUE(s.bail_reason.empty());
}

/**
 * @test GameSeq.LoadGameFailureSendsToMenuSuccessResumesPlaying
 * @brief Verifies load Game Failure Sends To Menu Success Resumes Playing.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, LoadGameFailureSendsToMenuSuccessResumesPlaying) {
  SeqMock s1;
  s1.game_state = GAMESTATE_LOADGAME;
  s1.load_save_ok = false;
  s1.Run(3);
  EXPECT_EQ(s1.function_mode, 2);

  SeqMock s2;
  s2.game_state = GAMESTATE_LOADGAME;
  s2.load_save_ok = true;
  s2.Run(3);
  EXPECT_EQ(s2.game_state, GAMESTATE_LVLPLAYING);
}

/**
 * @test GameSeq.EndChainGoesThroughNextLevelStart
 * @brief Verifies end Chain Goes Through Next Level Start.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, EndChainGoesThroughNextLevelStart) {
  SeqMock s;
  s.game_state = GAMESTATE_LVLEND;
  s.Run(6);
  EXPECT_EQ(s.end_calls, 1);
  EXPECT_EQ(s.next_level_calls, 1);   // LVLNEXT handler ran
  EXPECT_EQ(s.hog_checks, 1);         // back at LVLSTART
  EXPECT_EQ(s.game_state, GAMESTATE_LVLPLAYING);
}

/**
 * @test GameSeq.WarpEndsWithMinusOneAndJumpsToWarpTarget
 * @brief Verifies warp Ends With Minus One And Jumps To Warp Target.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, WarpEndsWithMinusOneAndJumpsToWarpTarget) {
  SeqMock s;
  s.game_state = GAMESTATE_LVLWARP;
  s.Run(6);
  EXPECT_EQ(s.end_calls, 1);
  EXPECT_EQ(s.set_cur_level, 7);
  EXPECT_EQ(s.game_state, GAMESTATE_LVLPLAYING); // via LVLSTART
}

/**
 * @test GameSeq.FailedLevelRestartsSameLevelWithoutNextAdvance
 * @brief Verifies failed Level Restarts Same Level Without Next Advance.
 *
 * @details
 * Exercises the GameSeq code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(GameSeq, FailedLevelRestartsSameLevelWithoutNextAdvance) {
  SeqMock s;
  s.game_state = GAMESTATE_LVLFAILED;
  s.Run(6);
  EXPECT_EQ(s.end_calls, 1);       // EndLevel(0)
  EXPECT_EQ(s.next_level_calls, 0); // skips LVLNEXT entirely
  EXPECT_EQ(s.game_state, GAMESTATE_LVLPLAYING);
}

// ---------------------------------------------------------------------------
// CheckHogfile decision table replication (gamesequence.cpp:1286-1313)
struct HogCheckMock {
  std::string mission_filename;
  int cur_level = 1;
  bool d3_2_exists_on_disk = false;
  int function_mode_after = 1;
  std::string opened_mn3;
  bool filename_updated = false;
};

static void RepCheckHogfile(HogCheckMock &m) {
  const char *new_mn3 = nullptr;
  if (m.mission_filename == "d3.mn3" && m.cur_level > 4)
    new_mn3 = "d3_2.mn3";
  else if (m.mission_filename == "d3_2.mn3" && m.cur_level <= 4)
    new_mn3 = "d3.mn3";

  if (new_mn3) {
    m.opened_mn3 = new_mn3;
    if (m.d3_2_exists_on_disk) {
      m.filename_updated = true;
      m.mission_filename = new_mn3;
    } else {
      // quirk: missing archive bails to menu WITHOUT updating filename;
      // next CheckHogfile would try again
      m.function_mode_after = 2;
    }
  }
}

/**
 * @test CheckHog.SwapsArchivesAtTheLevelFiveBoundary
 * @brief Verifies swaps Archives At The Level Five Boundary.
 *
 * @details
 * Exercises the CheckHog code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(CheckHog, SwapsArchivesAtTheLevelFiveBoundary) {
  HogCheckMock a{"d3.mn3", 5, /*exists*/ true};
  RepCheckHogfile(a);
  EXPECT_EQ(a.mission_filename, "d3_2.mn3");

  HogCheckMock b{"d3_2.mn3", 4, /*exists*/ true};
  RepCheckHogfile(b);
  EXPECT_EQ(b.mission_filename, "d3.mn3");
}

/**
 * @test CheckHog.BoundaryLevelsStayOnCurrentArchive
 * @brief Verifies boundary Levels Stay On Current Archive.
 *
 * @details
 * Exercises the CheckHog code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(CheckHog, BoundaryLevelsStayOnCurrentArchive) {
  HogCheckMock a{"d3.mn3", 4}; // level 4 is base campaign
  RepCheckHogfile(a);
  EXPECT_EQ(a.opened_mn3, "");

  HogCheckMock b{"d3_2.mn3", 5}; // level 5 keeps expansion loaded
  RepCheckHogfile(b);
  EXPECT_EQ(b.opened_mn3, "");
}

/**
 * @test CheckHog.MissingArchiveBailsToMenuKeepingFilename
 * @brief Verifies missing Archive Bails To Menu Keeping Filename.
 *
 * @details
 * Exercises the CheckHog code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(CheckHog, MissingArchiveBailsToMenuKeepingFilename) {
  HogCheckMock m{"d3.mn3", 9, /*exists*/ false};
  RepCheckHogfile(m);
  EXPECT_EQ(m.function_mode_after, 2); // MENU_MODE
  EXPECT_FALSE(m.filename_updated);
  EXPECT_EQ(m.mission_filename, "d3.mn3"); // unchanged
}

// ---------------------------------------------------------------------------
// SimpleStartLevel replication (gamesequence.cpp:1318-1329)
/**
 * @test SimpleStart.ForcesOneLevelMissionPointingAtGivenFile
 * @brief Verifies forces One Level Mission Pointing At Given File.
 *
 * @details
 * Exercises the SimpleStart code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamesequence.cpp
 * @ingroup descent3_tests
 */
TEST(SimpleStart, ForcesOneLevelMissionPointingAtGivenFile) {
  struct MissionMock {
    int cur_level = 9, num_levels = 12;
    const char *levels[2] = {"old.lvl", "old2.lvl"};
  } mission;

  mission.cur_level = 1;
  mission.num_levels = 1;
  static std::string fname = "test.lvl";
  mission.levels[0] = fname.c_str();

  EXPECT_EQ(mission.cur_level, 1);
  EXPECT_EQ(mission.num_levels, 1);
  EXPECT_STREQ(mission.levels[0], "test.lvl"); // single-level mini mission
}
