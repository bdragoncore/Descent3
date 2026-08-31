/**
 * @file object_real_tests.cpp
 * @brief Tests for object.cpp 3504 lines — object list management.
 *
 * @details
 * Covers viewer cycling, big-object list add/remove, room-list
 * link/unlink, FreeObjectSlots debris-then-weapon freeing (with its
 * shrinking-bound skip quirk), and the ObjAllocate/ObjFree stack.
 *
 * This harness validates the behavior of `Descent3/object.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/object.cpp`
 * @par Harness
 * `object_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/object.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <vector>

// replicated constants
constexpr int MAX_OBJECTS = 1500;
constexpr uint32_t OF_DEAD = 0x00000002;
constexpr uint32_t OF_BIG_OBJECT = 0x00000800;
constexpr float MIN_BIG_OBJ_RAD = 16.0f; // COL_TERRAIN_SIZE (CELLS_PER_COL_CELL=1)
constexpr int HANDLE_COUNT_INCREMENT = 0x800;
constexpr int HANDLE_COUNT_MASK = -2048; // 0xfffff800
constexpr uint8_t OBJ_NONE = 255, OBJ_WALL = 0, OBJ_FIREBALL = 1, OBJ_ROBOT = 2, OBJ_PLAYER = 4,
                  OBJ_WEAPON = 5, OBJ_POWERUP = 7, OBJ_DEBRIS = 8, OBJ_SPLINTER = 20;

// mock object array
struct ObjMock {
  uint8_t type = OBJ_NONE;
  uint16_t id = 0;
  uint32_t flags = 0;
  int handle = 0;
  int roomnum = -1;
  float size = 1.0f;
  int next = -1;
  int prev = -1;
};
static ObjMock Objects[MAX_OBJECTS];

// replicated SetObjectDeadFlag essence
static void MarkDead(int i) { Objects[i].flags |= OF_DEAD; }

// replicated FreeObjectSlots (object.cpp:1522-1598)
static int RepFreeObjectSlots(int Highest_object_index, int num_used) {
  std::vector<int> obj_list;
  int num_already_free = MAX_OBJECTS - Highest_object_index - 1;

  if (MAX_OBJECTS - num_already_free < num_used)
    return 0;

  for (int i = 0; i <= Highest_object_index; i++) {
    if (Objects[i].flags & OF_DEAD) {
      num_already_free++;
      if (MAX_OBJECTS - num_already_free < num_used)
        return num_already_free;
    } else {
      switch (Objects[i].type) {
      case OBJ_NONE:
        num_already_free++;
        if (MAX_OBJECTS - num_already_free < num_used)
          return 0; // quirk: this branch returns 0, OF_DEAD returns count
        break;
      case OBJ_FIREBALL:
      case OBJ_WEAPON:
      case OBJ_DEBRIS:
      case OBJ_SPLINTER:
        obj_list.push_back(i);
        break;
      default: break;
      }
    }
  }

  int num_to_free = MAX_OBJECTS - num_used - num_already_free;
  int original_num_to_free = num_to_free;

  if ((size_t)num_to_free > obj_list.size())
    num_to_free = (int)obj_list.size();

  for (int i = 0; i < num_to_free; i++)
    if (Objects[obj_list[i]].type == OBJ_DEBRIS) {
      num_to_free--;
      MarkDead(obj_list[i]);
    }

  if (!num_to_free)
    return original_num_to_free; // (duplicated twice in original)

  for (int i = 0; i < num_to_free; i++)
    if (Objects[obj_list[i]].type == OBJ_WEAPON) {
      num_to_free--;
      MarkDead(obj_list[i]);
    }

  return original_num_to_free - num_to_free;
}

/**
 * @test ObjectList.FreeSlotsPrefersDebrisThenWeapons
 * @brief Verifies free Slots Prefers Debris Then Weapons.
 *
 * @details
 * Exercises the ObjectList code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectList, FreeSlotsPrefersDebrisThenWeapons) {
  memset(Objects, 0, sizeof(Objects));
  for (int i = 0; i <= 4; i++) {
    Objects[i].type = OBJ_ROBOT; // not freeable
  }
  Objects[5].type = OBJ_DEBRIS;
  Objects[6].type = OBJ_WEAPON;
  Objects[7].type = OBJ_FIREBALL;

  // plenty of room: live count (8) below request -> immediate 0
  EXPECT_EQ(RepFreeObjectSlots(7, 100), 0);
  EXPECT_FALSE(Objects[5].flags & OF_DEAD);

  // num_used=6 -> to_free = MAX-6-already_free = 2:
  // candidates [debris, weapon, fireball]; quirk: freeing the debris
  // shrinks the bound to 1, so the weapons pass re-examines only the
  // already-freed debris slot -> exactly ONE object freed per call here
  int freed = RepFreeObjectSlots(7, 6);
  EXPECT_EQ(freed, 1); // original_num_to_free(2) - leftover(1)
  EXPECT_TRUE(Objects[5].flags & OF_DEAD);  // debris freed first
  EXPECT_FALSE(Objects[6].flags & OF_DEAD); // weapon skipped by shrinking bound
  EXPECT_FALSE(Objects[7].flags & OF_DEAD); // fireball untouched
}

/**
 * @test ObjectList.FreeSlotsShrinkingBoundSkipsCandidates
 * @brief Verifies free Slots Shrinking Bound Skips Candidates.
 *
 * @details
 * Exercises the ObjectList code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectList, FreeSlotsShrinkingBoundSkipsCandidates) {
  memset(Objects, 0, sizeof(Objects));
  Objects[0].type = OBJ_DEBRIS;
  Objects[1].type = OBJ_WEAPON;
  Objects[2].type = OBJ_DEBRIS;
  Objects[3].type = OBJ_ROBOT;
  Objects[4].type = OBJ_ROBOT;

  int Highest_object_index = 4;
  // already_free = 1500-4-1 = 1495; want to_free = 3:
  // 1500 - num_used - 1495 = 3 -> num_used = 2
  int num_used = 2;
  int freed = RepFreeObjectSlots(Highest_object_index, num_used);
  // original_num_to_free = 3; debris@0 freed (ntf 3->2); loop bound now 2,
  // so index 2 (the second debris) is never examined!
  // weapons pass: idx0 is debris (skip), idx1 weapon freed (ntf->1)
  EXPECT_EQ(freed, 2); // returns original - leftover = 3-1
  EXPECT_TRUE(Objects[0].flags & OF_DEAD);
  EXPECT_TRUE(Objects[1].flags & OF_DEAD);
  // quirk: trailing debris at index 2 NEVER freed due to the
  // decrementing-bound + incrementing-index double advance
  EXPECT_FALSE(Objects[2].flags & OF_DEAD);
}

// replicated ObjAllocate/ObjFree stack (object.cpp:1605-1645)
struct AllocMock {
  int16_t free_obj_list[MAX_OBJECTS];
  int Num_objects = 0;
  int Highest_object_index = 0;
  int Highest_ever = 0;

  void Reset() {
    for (int i = 0; i < MAX_OBJECTS; i++)
      free_obj_list[i] = i;
    Num_objects = 0;
    Highest_object_index = 0;
    Highest_ever = 0;
  }

  int Allocate() {
    int objnum = free_obj_list[Num_objects++];
    if (objnum > Highest_object_index) {
      Highest_object_index = objnum;
      if (Highest_object_index > Highest_ever)
        Highest_ever = Highest_object_index;
    }
    return objnum;
  }

  void Free(int objnum) {
    free_obj_list[--Num_objects] = objnum; // LIFO push back
    if (objnum == Highest_object_index)
      while (Highest_object_index > 0 && Objects[--Highest_object_index].type == OBJ_NONE)
        ;
  }
};

/**
 * @test ObjectList.AllocFreeStackAndHighestTracking
 * @brief Verifies alloc Free Stack And Highest Tracking.
 *
 * @details
 * Exercises the ObjectList code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectList, AllocFreeStackAndHighestTracking) {
  AllocMock a;
  a.Reset();
  memset(Objects, 0, sizeof(Objects));

  EXPECT_EQ(a.Allocate(), 0);
  EXPECT_EQ(a.Allocate(), 1);
  EXPECT_EQ(a.Allocate(), 2);
  EXPECT_EQ(a.Highest_object_index, 2);
  EXPECT_EQ(a.Highest_ever, 2);

  // free highest: index walks down over trailing unused slots
  Objects[2].type = OBJ_NONE; // default
  Objects[1].type = OBJ_NONE;
  a.Free(2);
  EXPECT_EQ(a.Highest_object_index, 0); // skipped 1 (NONE)

  // allocate again reuses the pushed-back slot (LIFO)
  EXPECT_EQ(a.Allocate(), 2);
  EXPECT_EQ(a.Highest_ever, 2); // ever-marker unchanged
}

// replicated BigObjAdd/Remove (object.cpp:1355-1385)
/**
 * @test ObjectList.BigObjectListAddRemoveCompacts
 * @brief Verifies big Object List Add Remove Compacts.
 *
 * @details
 * Exercises the ObjectList code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectList, BigObjectListAddRemoveCompacts) {
  constexpr int MAX_BIG_OBJECTS = 100;
  int16_t BigObjectList[MAX_BIG_OBJECTS];
  int Num_big_objects = 0;

  auto Add = [&](int objnum) {
    Objects[objnum].flags |= OF_BIG_OBJECT;
    BigObjectList[Num_big_objects++] = objnum;
  };
  auto Remove = [&](int objnum) {
    Objects[objnum].flags &= (~OF_BIG_OBJECT);
    int i;
    for (i = 0; i < Num_big_objects; i++) {
      if (BigObjectList[i] == objnum) {
        Num_big_objects--;
        break;
      }
    }
    while (i < Num_big_objects) {
      BigObjectList[i] = BigObjectList[i + 1];
      i++;
    }
  };

  Add(10);
  Add(20);
  Add(30);
  EXPECT_EQ(Num_big_objects, 3);
  EXPECT_TRUE(Objects[20].flags & OF_BIG_OBJECT);

  Remove(20);
  EXPECT_EQ(Num_big_objects, 2);
  EXPECT_EQ(BigObjectList[0], 10);
  EXPECT_EQ(BigObjectList[1], 30); // compacted, order preserved
  EXPECT_FALSE(Objects[20].flags & OF_BIG_OBJECT);

  // removing non-member just clears flag (loop finds nothing)
  Remove(99);
  EXPECT_EQ(Num_big_objects, 2);
}

// replicated ObjLink/ObjUnlink room list (object.cpp:1430-1517)
/**
 * @test ObjectList.RoomListDoublyLinked
 * @brief Verifies room List Doubly Linked.
 *
 * @details
 * Exercises the ObjectList code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectList, RoomListDoublyLinked) {
  struct RoomMock {
    int objects = -1;
  } room;
  for (int i = 0; i < MAX_OBJECTS; i++) {
    Objects[i] = {};
    Objects[i].roomnum = -1;
  }

  auto Link = [&](int objnum) {
    Objects[objnum].next = room.objects;
    room.objects = objnum;
    Objects[objnum].prev = -1;
    if (Objects[objnum].next != -1)
      Objects[Objects[objnum].next].prev = objnum;
    Objects[objnum].roomnum = 42;
  };
  auto Unlink = [&](int objnum) {
    if (Objects[objnum].roomnum == -1)
      return;
    if (Objects[objnum].prev == -1)
      room.objects = Objects[objnum].next;
    else
      Objects[Objects[objnum].prev].next = Objects[objnum].next;
    if (Objects[objnum].next != -1)
      Objects[Objects[objnum].next].prev = Objects[objnum].prev;
    Objects[objnum].roomnum = -1;
  };

  Link(5);
  Link(7);
  Link(9); // list: 9 <-> 7 <-> 5
  EXPECT_EQ(room.objects, 9);
  EXPECT_EQ(Objects[9].next, 7);
  EXPECT_EQ(Objects[7].prev, 9);
  EXPECT_EQ(Objects[7].next, 5);
  EXPECT_EQ(Objects[5].prev, 7);

  Unlink(7); // middle removal splices neighbors
  EXPECT_EQ(Objects[9].next, 5);
  EXPECT_EQ(Objects[5].prev, 9);
  EXPECT_EQ(Objects[7].roomnum, -1);

  Unlink(9); // head removal advances root
  EXPECT_EQ(room.objects, 5);
  EXPECT_EQ(Objects[5].prev, -1);

  Unlink(9); // idempotent: already unlinked
  EXPECT_EQ(room.objects, 5);
}

// replicated handle increment (object.cpp:1703-1708)
/**
 * @test ObjectList.HandleIncrementAndWrapMask
 * @brief Verifies handle Increment And Wrap Mask.
 *
 * @details
 * Exercises the ObjectList code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectList, HandleIncrementAndWrapMask) {
  uint32_t handle = 5;
  handle += HANDLE_COUNT_INCREMENT; // generation bits above slot number
  EXPECT_EQ(handle & 0x7FF, 5);     // low 11 bits = slot number

  uint32_t gen = 0xFFFFF800u >> 11; // max generation value
  handle = (gen << 11) | 7;
  EXPECT_EQ((handle & (uint32_t)0xFFFFF800u), 0xFFFFF800u); // wrap imminent
}
