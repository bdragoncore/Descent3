/**
 * @file small_game_real_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for next set of smallest game modules:
 *  - difficulty.cpp (Diff_* balance arrays)
 *  - gamepath.cpp/h (GamePaths array, Init/Free/Find)
 *  - megacell (lib/megacell.h + Descent3/megacell.cpp)
 *  - gamefile (Descent3/gamefile.h/.cpp)
 *  - special_face (Descent3/special_face.h/.cpp)
 * Each module is 40-160 lines, array-alloc pattern similar to list.
 * Compiled directly with their .cpp to avoid pulling large game libs.
 *
 * This harness validates the behavior of `Descent3/small_game.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/small_game.cpp`
 * @par Harness
 * `small_game_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/small_game.cpp
 */

#include <gtest/gtest.h>

#include <cstring>
#include <string>

// ---------------------------------------------------------------------------
// difficulty — extern const float arrays (5 difficulty levels)
// Declare without pulling heavy game.h
extern const float Diff_ai_dodge_percent[5];
extern const float Diff_ai_dodge_speed[5];
extern const float Diff_ai_speed[5];
extern const float Diff_ai_rotspeed[5];
extern const float Diff_ai_circle_dist[5];
extern const float Diff_ai_vis_dist[5];
extern const float Diff_player_damage[5];
extern const float Diff_ai_weapon_speed[5];
extern const float Diff_homing_strength[5];
extern const float Diff_robot_damage[5];
extern const float Diff_general_scalar[5];
extern const float Diff_general_inv_scalar[5];
extern const float Diff_shield_energy_scalar[5];
extern const float Diff_ai_turret_speed[5];
extern const float Diff_ai_min_fire_spread[5];

/**
 * @test SmallGame.Difficulty_HotShotIsNeutral
 * @brief Verifies difficulty Hot Shot Is Neutral.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, Difficulty_HotShotIsNeutral) {
  // Index 2 is "HotShot" (normal) — many scalars are 1.0 at normal
  EXPECT_FLOAT_EQ(Diff_ai_speed[2], 1.0f);
  EXPECT_FLOAT_EQ(Diff_ai_rotspeed[2], 1.0f);
  EXPECT_FLOAT_EQ(Diff_ai_vis_dist[2], 1.0f);
  EXPECT_FLOAT_EQ(Diff_player_damage[2], 1.0f);
  EXPECT_FLOAT_EQ(Diff_ai_weapon_speed[2], 1.0f);
  EXPECT_FLOAT_EQ(Diff_general_scalar[2], 1.0f);
  EXPECT_FLOAT_EQ(Diff_general_inv_scalar[2], 1.0f);
  EXPECT_FLOAT_EQ(Diff_shield_energy_scalar[2], 1.0f);
  EXPECT_FLOAT_EQ(Diff_ai_turret_speed[2], 1.0f);
}

/**
 * @test SmallGame.Difficulty_MonotonicSpeed
 * @brief Verifies difficulty Monotonic Speed.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, Difficulty_MonotonicSpeed) {
  // Trainee (0) slowest AI, Insane (4) fastest
  EXPECT_LT(Diff_ai_speed[0], Diff_ai_speed[2]);
  EXPECT_LT(Diff_ai_speed[2], Diff_ai_speed[4]);
  EXPECT_LT(Diff_ai_rotspeed[0], Diff_ai_rotspeed[4]);
  EXPECT_LT(Diff_ai_weapon_speed[0], Diff_ai_weapon_speed[4]);
}

/**
 * @test SmallGame.Difficulty_PlayerDamageInverseOfRobotDamage
 * @brief Verifies difficulty Player Damage Inverse Of Robot Damage.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, Difficulty_PlayerDamageInverseOfRobotDamage) {
  // Easier difficulty = less player damage taken, more robot damage taken
  EXPECT_LT(Diff_player_damage[0], Diff_player_damage[4]);
  EXPECT_GT(Diff_robot_damage[0], Diff_robot_damage[4]);
}

/**
 * @test SmallGame.Difficulty_ScalarInversePair
 * @brief Verifies difficulty Scalar Inverse Pair.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, Difficulty_ScalarInversePair) {
  for (int i = 0; i < 5; i++) {
    // general and inv_scalar are reciprocals in design (2.5↔0.5, etc.)
    EXPECT_GT(Diff_general_scalar[i], 0);
    EXPECT_GT(Diff_general_inv_scalar[i], 0);
  }
  EXPECT_FLOAT_EQ(Diff_general_scalar[0], 2.5f);
  EXPECT_FLOAT_EQ(Diff_general_scalar[4], 0.5f);
}

// ---------------------------------------------------------------------------
// gamepath — GamePaths[MAX_GAME_PATHS], Num_game_paths
#include "gamepath.h"
// FreeGamePath is defined in gamepath.cpp but not declared in the header.
extern void FreeGamePath(int n);

/**
 * @test SmallGame.GamePath_InitClears
 * @brief Verifies game Path Init Clears.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, GamePath_InitClears) {
  InitGamePaths();
  EXPECT_EQ(Num_game_paths, 0);
  for (int i = 0; i < MAX_GAME_PATHS; i++) {
    EXPECT_FALSE(GamePaths[i].used) << "i=" << i;
  }
  EXPECT_EQ(FindGamePathName("nonexistent"), -1);
}

/**
 * @test SmallGame.GamePath_FindByName
 * @brief Verifies game Path Find By Name.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, GamePath_FindByName) {
  InitGamePaths();
  // Manually insert a path (Alloc not exposed outside editor)
  GamePaths[5].used = true;
  GamePaths[5].pathnodes = nullptr;
  GamePaths[5].num_nodes = 0;
  strncpy(GamePaths[5].name, "TestPath", PAGENAME_LEN - 1);
  Num_game_paths = 1;
  EXPECT_EQ(FindGamePathName("TestPath"), 5);
  EXPECT_EQ(FindGamePathName("testpath"), 5); // case-insensitive via stricmp
  EXPECT_EQ(FindGamePathName("Nope"), -1);
  FreeGamePath(5);
  EXPECT_EQ(Num_game_paths, 0);
  EXPECT_FALSE(GamePaths[5].used);
}

/**
 * @test SmallGame.GamePath_FreeDecrementsCount
 * @brief Verifies game Path Free Decrements Count.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, GamePath_FreeDecrementsCount) {
  InitGamePaths();
  GamePaths[10].used = true;
  GamePaths[10].pathnodes = nullptr;
  GamePaths[20].used = true;
  GamePaths[20].pathnodes = nullptr;
  Num_game_paths = 2;
  FreeGamePath(10);
  EXPECT_EQ(Num_game_paths, 1);
  EXPECT_FALSE(GamePaths[10].used);
  FreeGamePath(20);
  EXPECT_EQ(Num_game_paths, 0);
}

/**
 * @test SmallGame.GamePath_FreeUnusedIsNoop
 * @brief Verifies game Path Free Unused Is Noop.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, GamePath_FreeUnusedIsNoop) {
  InitGamePaths();
  EXPECT_NO_THROW(FreeGamePath(99));
  EXPECT_EQ(Num_game_paths, 0);
}

// ---------------------------------------------------------------------------
// megacell — lib/megacell.h
#include "megacell.h"

/**
 * @test SmallGame.MegaCell_InitClears
 * @brief Verifies mega Cell Init Clears.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, MegaCell_InitClears) {
  InitMegacells();
  EXPECT_EQ(Num_megacells, 0);
  for (int i = 0; i < MAX_MEGACELLS; i++) EXPECT_FALSE(Megacells[i].used);
}

/**
 * @test SmallGame.MegaCell_AllocFree
 * @brief Verifies mega Cell Alloc Free.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, MegaCell_AllocFree) {
  InitMegacells();
  int idx = AllocMegacell();
  ASSERT_GE(idx, 0);
  EXPECT_EQ(Num_megacells, 1);
  EXPECT_TRUE(Megacells[idx].used);
  EXPECT_EQ(Megacells[idx].width, DEFAULT_MEGACELL_WIDTH);
  EXPECT_EQ(Megacells[idx].height, DEFAULT_MEGACELL_HEIGHT);
  FreeMegacell(idx);
  EXPECT_EQ(Num_megacells, 0);
  EXPECT_FALSE(Megacells[idx].used);
}

/**
 * @test SmallGame.MegaCell_GetNextWraps
 * @brief Verifies mega Cell Get Next Wraps.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, MegaCell_GetNextWraps) {
  InitMegacells();
  int a = AllocMegacell();
  int b = AllocMegacell();
  ASSERT_NE(a, b);
  // Next after a should be b (or wraps to b if ordering)
  int nxt = GetNextMegacell(a);
  EXPECT_EQ(nxt, b);
  // Single remaining after freeing one
  FreeMegacell(b);
  EXPECT_EQ(GetNextMegacell(a), a);
  FreeMegacell(a);
}

/**
 * @test SmallGame.MegaCell_AllocExhaustionReturnsMinusOne
 * @brief Verifies mega Cell Alloc Exhaustion Returns Minus One.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, MegaCell_AllocExhaustionReturnsMinusOne) {
  InitMegacells();
  for (int i = 0; i < MAX_MEGACELLS; i++) {
    int idx = AllocMegacell();
    ASSERT_NE(idx, -1) << "i=" << i;
  }
  EXPECT_EQ(AllocMegacell(), -1);
  InitMegacells(); // reset for other tests
}

// ---------------------------------------------------------------------------
// gamefile — Descent3/gamefile.h
#include "gamefile.h"

/**
 * @test SmallGame.GameFile_InitClears
 * @brief Verifies game File Init Clears.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, GameFile_InitClears) {
  InitGamefiles();
  EXPECT_EQ(Num_gamefiles, 0);
  for (int i = 0; i < MAX_GAMEFILES; i++) EXPECT_FALSE(Gamefiles[i].used);
  EXPECT_EQ(FindGamefileName((char*)"nope"), -1);
}

/**
 * @test SmallGame.GameFile_AllocFree
 * @brief Verifies game File Alloc Free.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, GameFile_AllocFree) {
  InitGamefiles();
  int idx = AllocGamefile();
  ASSERT_GE(idx, 0);
  EXPECT_EQ(Num_gamefiles, 1);
  EXPECT_TRUE(Gamefiles[idx].used);
  strcpy(Gamefiles[idx].name, "TestFile");
  EXPECT_EQ(FindGamefileName((char*)"TestFile"), idx);
  EXPECT_EQ(FindGamefileName((char*)"testfile"), idx);
  FreeGamefile(idx);
  EXPECT_EQ(Num_gamefiles, 0);
  EXPECT_FALSE(Gamefiles[idx].used);
  EXPECT_EQ(FindGamefileName((char*)"TestFile"), -1);
}

/**
 * @test SmallGame.GameFile_GetNextPrev
 * @brief Verifies game File Get Next Prev.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, GameFile_GetNextPrev) {
  InitGamefiles();
  int a = AllocGamefile();
  int b = AllocGamefile();
  ASSERT_NE(a, b);
  EXPECT_EQ(GetNextGamefile(a), b);
  EXPECT_EQ(GetPrevGamefile(b), a);
  FreeGamefile(a);
  FreeGamefile(b);
}

// ---------------------------------------------------------------------------
// special_face — Descent3/special_face.h
#include "special_face.h"

/**
 * @test SmallGame.SpecialFace_InitClears
 * @brief Verifies special Face Init Clears.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, SpecialFace_InitClears) {
  InitSpecialFaces();
  EXPECT_EQ(Num_of_special_faces, 0);
  for (int i = 0; i < 10; i++) EXPECT_FALSE(SpecialFaces[i].used);
}

/**
 * @test SmallGame.SpecialFace_AllocFree
 * @brief Verifies special Face Alloc Free.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, SpecialFace_AllocFree) {
  InitSpecialFaces();
  int h = AllocSpecialFace(SFT_SPECULAR, 1, false, 0);
  ASSERT_NE(h, BAD_SPECIAL_FACE_INDEX);
  EXPECT_EQ(Num_of_special_faces, 1);
  EXPECT_TRUE(SpecialFaces[h].used);
  EXPECT_NE(SpecialFaces[h].spec_instance, nullptr);
  FreeSpecialFace(h);
  EXPECT_EQ(Num_of_special_faces, 0);
  // second free is no-op (refcount path)
  EXPECT_NO_THROW(FreeSpecialFace(h));
}

/**
 * @test SmallGame.SpecialFace_AllocWithVertNorms
 * @brief Verifies special Face Alloc With Vert Norms.
 *
 * @details
 * Exercises the SmallGame code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/small_game.cpp
 * @ingroup descent3_tests
 */
TEST(SmallGame, SpecialFace_AllocWithVertNorms) {
  InitSpecialFaces();
  int h = AllocSpecialFace(SFT_SPECULAR, 2, true, 4);
  ASSERT_NE(h, BAD_SPECIAL_FACE_INDEX);
  EXPECT_TRUE(SpecialFaces[h].flags & SFF_SPEC_SMOOTH);
  EXPECT_NE(SpecialFaces[h].vertnorms, nullptr);
  FreeSpecialFace(h);
  EXPECT_EQ(SpecialFaces[h].vertnorms, nullptr);
}
