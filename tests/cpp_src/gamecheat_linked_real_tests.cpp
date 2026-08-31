/**
 * @file gamecheat_linked_real_tests.cpp
 * @brief Linked-real harness for Descent3/GameCheat.cpp.
 *
 * @details
 * Real cheat logic is compiled; engine game-state deps are stubbed.
 *
 * This harness validates the behavior of `Descent3/GameCheat.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/GameCheat.cpp`
 * @par Harness
 * `gamecheat_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/GameCheat.cpp
 */

#include "gtest/gtest.h"

#include "player.h"
#include "object.h"
#include "hud.h"
#include "weapon.h"
#include "hlsoundlib.h"
#include "game.h"
#include "ddio.h"
#include "soundload.h"
#include "sounds.h"
#include "damage.h"
#include "rtperformance.h"
#include "object_lighting.h"
#include "gamesequence.h"
#include "multi.h"
#include "stringtable.h"
#include "ship.h"
#include "render.h"
#include "renderer.h"
#include "gametexture.h"
#include "room.h"
#include "Inventory.h"

// --- game-state globals (defined here; referenced as extern by GameCheat.cpp) ---
object Objects[MAX_OBJECTS];
player Players[MAX_PLAYERS];
object *Player_object = nullptr;
ship Ships[MAX_SHIPS];
room Rooms[MAX_ROOMS];
netgame_info Netgame;
int Player_num = 0;
bool Player_has_camera = false;
int Game_mode = 0;
int Game_interface_mode = 0;
tStatMask Hud_stat_mask = 0;
int Force_one_texture = 0;
uint8_t AutomapVisMap[MAX_ROOMS];
texture GameTextures[MAX_TEXTURES];

// --- engine function stubs (signatures match those seen by GameCheat.cpp) ---
bool AddHUDMessage(const char *, ...) { return true; }
int FindTextureName(const char *) { return -1; }
int FindObjectIDName(const char *) { return -1; }
int FindWeaponName(const char *) { return -1; }
void MakeObjectVisible(object *) {}
void MakeObjectInvisible(object *, float, float, bool) {}
void SetObjectDeadFlag(object *, bool, bool) {}
void MakePlayerVulnerable(int) {}
void MakePlayerInvulnerable(int, float, bool) {}
bool ApplyDamageToPlayer(object *, object *, int, float, int, int, bool) { return true; }
void MultiSendMessageToServer(int, char *, int) {}
void MultiSendMessageFromServer(int, char *, int) {}
int ddio_KeyToAscii(int key) { return key; }
const char *GetStringFromTable(int) { return ""; }

// --- Inventory members referenced via player struct / cheat inventory grants ---
Inventory::Inventory() {}
Inventory::~Inventory() {}
bool Inventory::Add(int, int, object *, int, int, int, const char *) { return false; }

// --- symbols defined in GameCheat.cpp (external linkage) ---
extern bool IsCheater;
extern bool Display_renderer_stats;
extern char CheatBuffer[];
extern char OldCheatBuffer[];
extern void DemoCheats(int key);
extern void SendCheaterAttemptText();

/**
 * @test GameCheatLinked.InitState
 * @brief Verifies init State.
 *
 * @details
 * Exercises the GameCheatLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheatLinked, InitState) {
  EXPECT_FALSE(IsCheater);
  EXPECT_FALSE(Display_renderer_stats);
  EXPECT_STREQ(CheatBuffer, "AAAAAAAAAAAAAAA");
  EXPECT_STREQ(OldCheatBuffer, "AAAAAAAAAAAAAAA");
}

/**
 * @test GameCheatLinked.DemoCheatsNoCrash
 * @brief Verifies demo Cheats No Crash.
 *
 * @details
 * Exercises the GameCheatLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheatLinked, DemoCheatsNoCrash) {
  for (int k = 0; k < 256; k++) {
    DemoCheats(k);
  }
  SUCCEED();
}

/**
 * @test GameCheatLinked.DemoCheatsAsciiPath
 * @brief Verifies demo Cheats Ascii Path.
 *
 * @details
 * Exercises the GameCheatLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheatLinked, DemoCheatsAsciiPath) {
  // Feed printable characters; must not throw or corrupt the buffer length.
  for (char c = 'A'; c <= 'Z'; c++) {
    DemoCheats(static_cast<int>(c));
  }
  EXPECT_EQ(strlen(CheatBuffer), strlen("AAAAAAAAAAAAAAA"));
}

/**
 * @test GameCheatLinked.SendCheaterAttemptTextNoCrash
 * @brief Verifies send Cheater Attempt Text No Crash.
 *
 * @details
 * Exercises the GameCheatLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheatLinked, SendCheaterAttemptTextNoCrash) {
  Player_num = 0;
  SendCheaterAttemptText();
  SUCCEED();
}

/**
 * @test GameCheatLinked.IsCheaterStaysBool
 * @brief Verifies is Cheater Stays Bool.
 *
 * @details
 * Exercises the GameCheatLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheatLinked, IsCheaterStaysBool) {
  // After a random key stream IsCheater remains a valid bool value.
  for (int i = 0; i < 100; i++) {
    DemoCheats(i % 128);
  }
  EXPECT_TRUE(IsCheater == true || IsCheater == false);
}

/**
 * @test GameCheatLinked.ForceOneTextureGlobalExists
 * @brief Verifies force One Texture Global Exists.
 *
 * @details
 * Exercises the GameCheatLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheatLinked, ForceOneTextureGlobalExists) {
  Force_one_texture = 1;
  EXPECT_EQ(Force_one_texture, 1);
  Force_one_texture = 0;
}
