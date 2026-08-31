/**
 * @file door_ship_vclip_real_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Next smallest alloc-array modules: door (60), ship (30), vclip (200)
 * Each follows Init/Alloc/Free/GetNext/Find pattern. Compiled directly
 * to hit real array bounds and Int3 paths (RELEASE silenced).
 *
 * This harness validates the behavior of `Descent3/door_ship_vclip.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/door_ship_vclip.cpp`
 * @par Harness
 * `door_ship_vclip_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/door_ship_vclip.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>
#include <cstring>
#include "robotfirestruct.h"

// Stubs to link door/ship/vclip without pulling full game
int LoadPolyModel(const std::filesystem::path &filename, int pageable) {
  (void)filename;
  (void)pageable;
  return -1;
}
int Low_vidmem = 0;
int paged_in_count = 0;
int paged_in_num = 0;
int Mem_low_memory_mode = 0;
void WBClearInfo(otype_wb_info *wb) {
  if (wb) memset(wb, 0, sizeof(otype_wb_info) * 21);
}
struct object;
void WBClearInfo(object *obj) { (void)obj; }

// ---------------------------------------------------------------------------
// door — Descent3/door.h + door.cpp (258 lines)
#include "door.h"

/**
 * @test DoorsShipsVclips.Door_InitClears
 * @brief Verifies door Init Clears.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Door_InitClears) {
  InitDoors();
  EXPECT_EQ(Num_doors, 0);
  for (int i = 0; i < MAX_DOORS; i++) {
    EXPECT_EQ(Doors[i].used, 0) << "i=" << i;
    EXPECT_EQ(Doors[i].model_handle, -1);
  }
  EXPECT_EQ(FindDoorName("nope"), -1);
}

/**
 * @test DoorsShipsVclips.Door_AllocFree
 * @brief Verifies door Alloc Free.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Door_AllocFree) {
  InitDoors();
  int a = AllocDoor();
  ASSERT_GE(a, 0);
  EXPECT_EQ(Num_doors, 1);
  EXPECT_EQ(Doors[a].used, 1);
  EXPECT_EQ(Doors[a].flags, 0);
  strcpy(Doors[a].name, "TestDoor");
  EXPECT_EQ(FindDoorName("TestDoor"), a);
  EXPECT_EQ(FindDoorName("testdoor"), a);
  FreeDoor(a);
  EXPECT_EQ(Num_doors, 0);
  EXPECT_EQ(Doors[a].used, 0);
  EXPECT_EQ(FindDoorName("TestDoor"), -1);
}

/**
 * @test DoorsShipsVclips.Door_GetNextPrevWrap
 * @brief Verifies door Get Next Prev Wrap.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Door_GetNextPrevWrap) {
  InitDoors();
  int a = AllocDoor();
  int b = AllocDoor();
  ASSERT_NE(a, b);
  EXPECT_EQ(GetNextDoor(a), b);
  EXPECT_EQ(GetPrevDoor(b), a);
  // wrap-around
  EXPECT_EQ(GetNextDoor(b), a);
  EXPECT_EQ(GetPrevDoor(a), b);
  // single remaining
  FreeDoor(b);
  EXPECT_EQ(GetNextDoor(a), a);
  EXPECT_EQ(GetPrevDoor(a), a);
  FreeDoor(a);
}

/**
 * @test DoorsShipsVclips.Door_AllocExhaustion
 * @brief Verifies door Alloc Exhaustion.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Door_AllocExhaustion) {
  InitDoors();
  for (int i = 0; i < MAX_DOORS; i++) ASSERT_NE(AllocDoor(), -1) << i;
  EXPECT_EQ(AllocDoor(), -1);
  InitDoors();
}

// ---------------------------------------------------------------------------
// ship — Descent3/ship.h + ship.cpp (307 lines)
#include "ship.h"

/**
 * @test DoorsShipsVclips.Ship_InitClears
 * @brief Verifies ship Init Clears.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Ship_InitClears) {
  InitShips();
  EXPECT_EQ(Num_ships, 0);
  for (int i = 0; i < MAX_SHIPS; i++) EXPECT_EQ(Ships[i].used, 0) << i;
  EXPECT_EQ(FindShipName("nope"), -1);
}

/**
 * @test DoorsShipsVclips.Ship_AllocFree
 * @brief Verifies ship Alloc Free.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Ship_AllocFree) {
  InitShips();
  int s = AllocShip();
  ASSERT_GE(s, 0);
  EXPECT_EQ(Num_ships, 1);
  EXPECT_EQ(Ships[s].used, 1);
  EXPECT_FLOAT_EQ(Ships[s].size, 4.0f);
  EXPECT_EQ(Ships[s].model_handle, -1);
  EXPECT_EQ(Ships[s].dying_model_handle, -1);
  strcpy(Ships[s].name, "TestShip");
  EXPECT_EQ(FindShipName("TestShip"), s);
  FreeShip(s);
  EXPECT_EQ(Num_ships, 0);
  EXPECT_EQ(Ships[s].used, 0);
}

/**
 * @test DoorsShipsVclips.Ship_GetNextWraps
 * @brief Verifies ship Get Next Wraps.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Ship_GetNextWraps) {
  InitShips();
  int a = AllocShip();
  int b = AllocShip();
  ASSERT_NE(a, b);
  EXPECT_EQ(GetNextShip(a), b);
  EXPECT_EQ(GetPrevShip(b), a);
  FreeShip(b);
  EXPECT_EQ(GetNextShip(a), a);
  FreeShip(a);
}

/**
 * @test DoorsShipsVclips.Ship_AllocExhaustion
 * @brief Verifies ship Alloc Exhaustion.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Ship_AllocExhaustion) {
  InitShips();
  for (int i = 0; i < MAX_SHIPS; i++) ASSERT_NE(AllocShip(), -1) << i;
  EXPECT_EQ(AllocShip(), -1);
  InitShips();
}

// ---------------------------------------------------------------------------
// vclip — Descent3/vclip.h + vclip.cpp (631 lines, but Alloc/Free simple)
#include "vclip.h"
#include "Inventory.h"

// Provide definitions for extern globals referenced by ship/vclip/bitmap
// Must be after the headers that declare them, with correct C++ type.
Inventory::Inventory() {}
Inventory::~Inventory() {}
object Objects[10];
player Players[10];

/**
 * @test DoorsShipsVclips.Vclip_InitClears
 * @brief Verifies vclip Init Clears.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Vclip_InitClears) {
  InitVClips();
  EXPECT_EQ(Num_vclips, 0);
  for (int i = 0; i < MAX_VCLIPS; i++) EXPECT_EQ(GameVClips[i].used, 0) << i;
  EXPECT_EQ(FindVClipName("nope"), -1);
}

/**
 * @test DoorsShipsVclips.Vclip_AllocFree
 * @brief Verifies vclip Alloc Free.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Vclip_AllocFree) {
  InitVClips();
  int v = AllocVClip();
  ASSERT_GE(v, 0);
  EXPECT_EQ(Num_vclips, 1);
  EXPECT_EQ(GameVClips[v].used, 1);
  EXPECT_NE(GameVClips[v].frames, nullptr);
  EXPECT_FLOAT_EQ(GameVClips[v].frame_time, 0.07f);
  EXPECT_TRUE(GameVClips[v].flags & VCF_NOT_RESIDENT);
  strcpy(GameVClips[v].name, "TestVClip");
  EXPECT_EQ(FindVClipName("TestVClip"), v);
  FreeVClip(v);
  EXPECT_EQ(Num_vclips, 0);
}

/**
 * @test DoorsShipsVclips.Vclip_GetNextFindNotNeeded_AllocMany
 * @brief Verifies vclip Get Next Find Not Needed Alloc Many.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Vclip_GetNextFindNotNeeded_AllocMany) {
  InitVClips();
  int a = AllocVClip();
  int b = AllocVClip();
  ASSERT_NE(a, b);
  // vclip has no GetNext; just verify two allocs distinct and find works
  strcpy(GameVClips[a].name, "VClipA");
  strcpy(GameVClips[b].name, "VClipB");
  EXPECT_EQ(FindVClipName("VClipA"), a);
  EXPECT_EQ(FindVClipName("VClipB"), b);
  FreeVClip(a);
  FreeVClip(b);
  EXPECT_EQ(Num_vclips, 0);
}

/**
 * @test DoorsShipsVclips.Vclip_AllocExhaustion
 * @brief Verifies vclip Alloc Exhaustion.
 *
 * @details
 * Exercises the DoorsShipsVclips code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door_ship_vclip.cpp
 * @ingroup descent3_tests
 */
TEST(DoorsShipsVclips, Vclip_AllocExhaustion) {
  InitVClips();
  for (int i = 0; i < MAX_VCLIPS; i++) {
    int v = AllocVClip();
    ASSERT_NE(v, -1) << "i=" << i;
  }
  EXPECT_EQ(AllocVClip(), -1);
  // cleanup via FreeAllVClips is atexit-registered; manual reset for next tests
  for (int i = 0; i < MAX_VCLIPS; i++) if (GameVClips[i].used) FreeVClip(i);
  EXPECT_EQ(Num_vclips, 0);
}
