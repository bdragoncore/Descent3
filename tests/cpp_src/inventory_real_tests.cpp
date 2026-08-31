/**
 * @file inventory_real_tests.cpp
 * @brief Tests for Inventory.cpp (1535 lines).
 *
 * @details
 * Covers the type/id inventory: add/duplicate counting, countermeasure
 * routing, removal, position navigation/accessors, the selectable==useable
 * flag alias quirk, the INVAF_LEVELLAST namespace bug, Use() flow with
 * Osiris event dispatch, and Save/Read round trip.
 *
 * This harness validates the behavior of `Descent3/Inventory.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/Inventory.cpp`
 * @par Harness
 * `inventory_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/Inventory.cpp
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <map>

#include "Inventory.h"
#include "object.h"
#include "objinfo.h"
#include "levelgoal.h"
#include "multi.h"
#include "player.h"
#include "game.h"
#include "osiris_dll.h"
#include "weapon.h"
#include "player.h"
#include "hlsoundlib.h"
#include "hud.h"
#include "AIMain.h"
#include "stringtable.h"
#include "ObjScript.h"
#include "mem.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
object Objects[MAX_OBJECTS];
int Highest_object_index = 0;
object_info Object_info[MAX_OBJECT_IDS];
levelgoals Level_goals;
netgame_info Netgame;
weapon Weapons[MAX_WEAPONS];
texture GameTextures[MAX_TEXTURES];
bms_bitmap GameBitmaps[MAX_BITMAPS];
int Game_mode = 0;
float Frametime = 0.0f;

void levelgoals::Inform(char, int, int) {}

hlsSystem Sound_system;
hlsSystem::hlsSystem() {}
int hlsSystem::Play2dSound(int, float, float, uint16_t) { return -1; }
void hlsSystem::KillSoundLib(bool) {}
void MultiSendInventoryRemoveItem(int, int, int) {}
bool AINotify(object *, uint8_t, void *) { return true; }
const char *GetStringFromTable(int index) {
  static char buf[32];
  snprintf(buf, sizeof(buf), "str%d", index);
  return buf;
}
bool AddHUDMessage(const char *, ...) { return true; }

// Minimal object lookup for OBJNUM/ObjGet paths
static std::map<int, object *> g_objmap;
object *ObjGet(int handle) {
  auto it = g_objmap.find(handle);
  return it == g_objmap.end() ? nullptr : it->second;
}

// Ghosting / multiplayer / creation stubs
void ObjGhostObject(int) {}
void ObjUnGhostObject(int) {}
void MultiSendGhostObject(object *, bool) {}
void MultiSendObject(object *, int) {}
void MultiSendClientInventoryUseItem(int, int) {}
int Player_num = 0;
player Players[MAX_PLAYERS];
int ObjCreate(uint8_t type, uint16_t id, int roomnum, vector *pos, const matrix *orient, int parent_handle) {
  static int next = 50;
  (void)type; (void)id; (void)roomnum; (void)pos; (void)orient; (void)parent_handle;
  if (next >= MAX_OBJECTS)
    return -1;
  return next++;
}
void MultiSendObject(object *, uint8_t, uint8_t) {}
void SetObjectControlType(object *obj, int ct) { obj->control_type = ct; }
void SetObjectDeadFlag(object *obj, bool, bool) { obj->flags |= OF_DEAD; }
static int g_cm_created = 0;
void CreateCountermeasureFromObject(object *, int) { g_cm_created++; }

struct EvtRec { int event; int objnum; };
static std::vector<EvtRec> g_events;
static bool g_osiris_ret = true;
bool Osiris_CallEvent(object *obj, int event, tOSIRISEventInfo *) {
  g_events.push_back({event, (int)OBJNUM(obj)});
  return g_osiris_ret;
}
bool Osiris_BindScriptsToObject(object *) { return true; }
void Osiris_DetachScriptsFromObject(object *) {}
void InitObjectScripts(object *, bool) {}

/**
 * @brief GTest fixture for InventoryTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class InventoryTest : public ::testing::Test {
protected:
  Inventory inv;

  void SetUp() override {
    memset(Object_info, 0, sizeof(Object_info));
    Game_mode = 0;
    g_events.clear();
    g_osiris_ret = true;
    g_cm_created = 0;
  }

  // Marks an Object_info entry as a selectable powerup with names
  void SetupInfo(int id, uint32_t flags = OIF_INVEN_SELECTABLE, const char *name = "Lava Board") {
    strcpy(Object_info[id].name, name);
    strcpy(Object_info[id].icon_name, "icon.pgo");
    Object_info[id].description = mem_strdup("desc"); // heap-owned by design
    Object_info[id].flags = flags;
  }
};

/**
 * @test InventoryTest.EmptyInventoryBasics
 * @brief Verifies empty Inventory Basics.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, EmptyInventoryBasics) {
  EXPECT_EQ(inv.Size(), 0);
  EXPECT_FALSE(inv.CheckItem(OBJ_POWERUP, 3));
  EXPECT_FALSE(inv.Remove(OBJ_POWERUP, 3));
  EXPECT_FALSE(inv.Use(OBJ_POWERUP, 3, nullptr));
}

/**
 * @test InventoryTest.AddCreatesNodeAndDuplicatesStackCount
 * @brief Verifies add Creates Node And Duplicates Stack Count.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, AddCreatesNodeAndDuplicatesStackCount) {
  SetupInfo(3);
  ASSERT_TRUE(inv.Add(OBJ_POWERUP, 3));
  ASSERT_TRUE(inv.Add(OBJ_POWERUP, 3));
  ASSERT_TRUE(inv.Add(OBJ_POWERUP, 3));

  EXPECT_EQ(inv.Size(), 1);      // one unique entry
  inv.ResetPos();
  EXPECT_EQ(inv.GetPosCount(), 3); // ...with count 3
  EXPECT_TRUE(inv.CheckItem(OBJ_POWERUP, 3));
}

/**
 * @test InventoryTest.DistinctIdsAreDistinctNodes
 * @brief Verifies distinct Ids Are Distinct Nodes.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, DistinctIdsAreDistinctNodes) {
  SetupInfo(1);
  SetupInfo(2);
  inv.Add(OBJ_POWERUP, 1);
  inv.Add(OBJ_POWERUP, 2);
  EXPECT_EQ(inv.Size(), 2);
  EXPECT_EQ(inv.GetPosCount(), 1); // each holds a single item
}

/**
 * @test InventoryTest.InvalidTypesRejected
 * @brief Verifies invalid Types Rejected.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, InvalidTypesRejected) {
  EXPECT_FALSE(inv.Add(-1, 0));
  EXPECT_FALSE(inv.Add(OBJ_NONE, 0));
  EXPECT_EQ(inv.Size(), 0);
}

/**
 * @test InventoryTest.WeaponRoutesToCountermeasureEntry
 * @brief Verifies weapon Routes To Countermeasure Entry.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, WeaponRoutesToCountermeasureEntry) {
  inv.AddCounterMeasure(9);
  EXPECT_TRUE(inv.CheckItem(OBJ_WEAPON, 9));
  inv.AddCounterMeasure(9);
  inv.ResetPos();
  EXPECT_EQ(inv.GetPosCount(), 2);
}

/**
 * @test InventoryTest.RemoveDecrementsThenDeletesNode
 * @brief Verifies remove Decrements Then Deletes Node.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, RemoveDecrementsThenDeletesNode) {
  SetupInfo(4);
  inv.Add(OBJ_POWERUP, 4);
  inv.Add(OBJ_POWERUP, 4);

  ASSERT_TRUE(inv.Remove(OBJ_POWERUP, 4));
  EXPECT_EQ(inv.Size(), 1);
  EXPECT_EQ(inv.GetPosCount(), 1);

  ASSERT_TRUE(inv.Remove(OBJ_POWERUP, 4));
  EXPECT_EQ(inv.Size(), 0); // node gone entirely
  EXPECT_FALSE(inv.CheckItem(OBJ_POWERUP, 4));
}

/**
 * @test InventoryTest.NavigationWalksAllItemsAndValidatesSelectable
 * @brief Verifies navigation Walks All Items And Validates Selectable.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, NavigationWalksAllItemsAndValidatesSelectable) {
  SetupInfo(1);
  SetupInfo(2);
  SetupInfo(3, 0); // item 3 is NOT selectable
  inv.Add(OBJ_POWERUP, 1);
  inv.Add(OBJ_POWERUP, 2);
  inv.Add(OBJ_POWERUP, 3);

  int t, id;
  inv.ResetPos();
  EXPECT_TRUE(inv.AtBeginning());
  inv.GetPosTypeID(t, id); // NOTE: bool result only true for INVF_OBJECT entries
  EXPECT_EQ(id, 1);
  EXPECT_TRUE(inv.IsSelectable());
  EXPECT_STREQ(inv.GetPosName(), "Lava Board");
  EXPECT_STREQ(inv.GetPosIconName(), "icon.pgo");
  EXPECT_STREQ(inv.GetPosDescription(), "desc");

  inv.NextPos(false);
  EXPECT_FALSE(inv.AtBeginning());
  inv.GetPosTypeID(t, id);
  EXPECT_EQ(id, 2);

  inv.PrevPos(false);
  inv.GetPosTypeID(t, id);
  EXPECT_EQ(id, 1);

  inv.GotoPos(OBJ_POWERUP, 3); // park directly on the non-selectable item
  EXPECT_FALSE(inv.IsSelectable());

  // NextPos(false): move + ValidatePos snaps forward past nothing needed
  // here (item 1 wraps around and is selectable)
  inv.NextPos(false);
  inv.GetPosTypeID(t, id);
  EXPECT_EQ(id, 1);

  // skip=true bypasses ValidatePos: raw ring neighbor
  inv.GotoPos(OBJ_POWERUP, 3);
  inv.NextPos(true);
  inv.GetPosTypeID(t, id);
  EXPECT_EQ(id, 1);

  inv.GotoPos(OBJ_POWERUP, 2);
  EXPECT_FALSE(inv.AtEnd()); // next node exists

  inv.GotoPos(OBJ_POWERUP, 3);
  EXPECT_TRUE(inv.AtEnd()); // last node in the ring
}

/**
 * @test InventoryTest.QuirkNonSelectableAddLeavesPosNull
 * @brief Verifies quirk Non Selectable Add Leaves Pos Null.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, QuirkNonSelectableAddLeavesPosNull) {
  SetupInfo(31, 0);
  ASSERT_TRUE(inv.Add(OBJ_POWERUP, 31));

  uint16_t iflags = 0xFFFF; int fl = -1;
  // AddObjectItem only assigns pos=newnode when the item is selectable;
  // otherwise pos stays NULL and accessors report empties.
  inv.GetPosInfo(iflags, fl);
  EXPECT_EQ(iflags, 0u);
  EXPECT_EQ(fl, 0);

  inv.ResetPos(); // caller must reset explicitly
  inv.GetPosInfo(iflags, fl);
  EXPECT_EQ(iflags, 0u); // real iflags (no OIF_INVEN_* bits requested)
}

/**
 * @test InventoryTest.GotoPosByIdAndIndex
 * @brief Verifies goto Pos By Id And Index.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, GotoPosByIdAndIndex) {
  SetupInfo(5);
  SetupInfo(6);
  inv.Add(OBJ_POWERUP, 5);
  inv.Add(OBJ_POWERUP, 6);

  inv.GotoPos(OBJ_POWERUP, 6);
  int t, id;
  inv.GetPosTypeID(t, id);
  EXPECT_EQ(id, 6);

  inv.GotoPos(0);
  inv.GetPosTypeID(t, id);
  EXPECT_EQ(id, 5);
}

/**
 * @test InventoryTest.QuirkSelectableImpliesUseableViaSharedBit
 * @brief Verifies quirk Selectable Implies Useable Via Shared Bit.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, QuirkSelectableImpliesUseableViaSharedBit) {
  // INVF_SELECTABLE and INVF_USEABLE are both 0x0003 - one bit, two names.
  SetupInfo(8, OIF_INVEN_SELECTABLE);
  inv.Add(OBJ_POWERUP, 8);
  uint16_t iflags; int fl;
  inv.GetPosInfo(iflags, fl);
  EXPECT_TRUE(iflags & INVF_USEABLE); // set purely by the SELECTABLE flag

  SetupInfo(9, 0); // not selectable -> also NOT useable
  inv.Add(OBJ_POWERUP, 9);
  inv.GotoPos(OBJ_POWERUP, 9);
  inv.GetPosInfo(iflags, fl);
  EXPECT_FALSE(iflags & INVF_USEABLE);
  EXPECT_FALSE(inv.Use(OBJ_POWERUP, 9, nullptr)); // gate rejects
}

/**
 * @test InventoryTest.AddMapsInvafFlagsToInvfFlags
 * @brief Verifies add Maps Invaf Flags To Invf Flags.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, AddMapsInvafFlagsToInvfFlags) {
  SetupInfo(11, 0);
  // INVAF_* add-flags are translated into INVF_* item flags:
  // LEVELLAST -> MISSIONITEM, TIMEOUTONSPEW -> TIMEOUTONSPEW
  inv.Add(OBJ_POWERUP, 11, nullptr, -1, -1, INVAF_LEVELLAST | INVAF_TIMEOUTONSPEW);

  uint16_t iflags; int fl;
  inv.ResetPos(); // non-selectable add leaves pos NULL
  inv.GetPosInfo(iflags, fl);
  EXPECT_TRUE(iflags & INVF_MISSIONITEM);
  EXPECT_TRUE(iflags & INVF_TIMEOUTONSPEW);
}

/**
 * @test InventoryTest.GetTypeIDCountsStackedItems
 * @brief Verifies get Type IDCounts Stacked Items.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, GetTypeIDCountsStackedItems) {
  SetupInfo(41);
  inv.Add(OBJ_POWERUP, 41);
  inv.Add(OBJ_POWERUP, 41);
  inv.Add(OBJ_POWERUP, 41);

  EXPECT_EQ(inv.GetTypeIDCount(OBJ_POWERUP, 41), 3);
  EXPECT_EQ(inv.GetTypeIDCount(OBJ_POWERUP, 99), 0);
}

/**
 * @test InventoryTest.UseRecreatesObjectFiresEvtUseAndRemovesOnTrue
 * @brief Verifies use Recreates Object Fires Evt Use And Removes On True.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, UseRecreatesObjectFiresEvtUseAndRemovesOnTrue) {
  SetupInfo(12, OIF_INVEN_SELECTABLE | OIF_INVEN_TYPE_MISSION);
  inv.Add(OBJ_POWERUP, 12);

  static object player{};
  player.roomnum = 7;
  player.handle = 3;

  g_osiris_ret = true;
  ASSERT_TRUE(inv.Use(OBJ_POWERUP, 12, &player));

  ASSERT_EQ(g_events.size(), 1u);
  EXPECT_EQ(g_events[0].event, EVT_USE);
  EXPECT_GE(g_events[0].objnum, 50);                        // created by our stub allocator
  EXPECT_TRUE(Objects[g_events[0].objnum].flags & OF_DEAD); // remove-on-use
  EXPECT_FALSE(inv.CheckItem(OBJ_POWERUP, 12));
}

/**
 * @test InventoryTest.UseWithOsirisRefusalKeepsItem
 * @brief Verifies use With Osiris Refusal Keeps Item.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, UseWithOsirisRefusalKeepsItem) {
  SetupInfo(13, OIF_INVEN_SELECTABLE);
  inv.Add(OBJ_POWERUP, 13);

  static object player{};
  player.roomnum = 7;
  player.handle = 4;

  g_osiris_ret = false;
  // Script veto: the item is used but NOT consumed, and Use() reports
  // failure (ret is only set true when Osiris_CallEvent returns true)
  EXPECT_FALSE(inv.Use(OBJ_POWERUP, 13, &player));
  EXPECT_TRUE(inv.CheckItem(OBJ_POWERUP, 13));
}

/**
 * @test InventoryTest.SaveReadRoundTrip
 * @brief Verifies save Read Round Trip.
 *
 * @details
 * Exercises the InventoryTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Inventory.cpp
 * @ingroup descent3_tests
 */
TEST_F(InventoryTest, SaveReadRoundTrip) {
  SetupInfo(21, 0, "Alpha");
  SetupInfo(22, 0, "Beta");
  inv.Add(OBJ_POWERUP, 21);
  inv.Add(OBJ_POWERUP, 22);
  inv.Add(OBJ_POWERUP, 22);

  remove("/tmp/opencode/inv.dat");

  CFILE *cf = cfopen("/tmp/opencode/inv.dat", "wb");
  ASSERT_NE(cf, nullptr);
  ASSERT_GT(inv.SaveInventory(cf), 0);
  cfclose(cf);

  Inventory inv2;
  CFILE *in = cfopen("/tmp/opencode/inv.dat", "rb");
  ASSERT_NE(in, nullptr);
  ASSERT_GT(inv2.ReadInventory(in), 0);
  cfclose(in);

  EXPECT_EQ(inv2.Size(), 2);
  EXPECT_TRUE(inv2.CheckItem(OBJ_POWERUP, 21));
  EXPECT_TRUE(inv2.CheckItem(OBJ_POWERUP, 22));
  inv2.ResetPos();
  EXPECT_STREQ(inv2.GetPosName(), "Alpha");
  inv2.GotoPos(OBJ_POWERUP, 22);
  EXPECT_EQ(inv2.GetPosCount(), 2);
}
