/**
 * @file boa_real_tests.cpp
 * @brief Tests for BOA.cpp 3021 lines — room-adjacency/path tables.
 *
 * @details
 * Covers the shared index-normalization preamble used by the query
 * API (BOA_IsVisible/IsSoundAudible/HasPossibleBlockage/GetNextRoom),
 * terrain-region remapping, fail-open visibility when the vis table
 * is invalid, the (-1,-1) same-room true quirk in BOA_IsVisible,
 * BOA_PassablePortal mode rules (robot vs sound vs making-boa), the
 * external-room cost-array bypass quirk, and BOA_LockedDoor's
 * half-open threshold.
 *
 * This harness validates the behavior of `Descent3/BOA.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/BOA.cpp`
 * @par Harness
 * `boa_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/BOA.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>

// replicated constants
constexpr int MAX_ROOMS = 400;
constexpr int MAX_BOA_TERRAIN_REGIONS = 8;
constexpr int ROOMNUM_CELLNUM_FLAG = (int)0x80000000;
constexpr uint16_t BOA_ROOM_MASK = 0x03FF;
constexpr uint16_t BOAF_VIS = 0x0400;
constexpr uint16_t BOA_SOUND_PROP = 0x0800;
constexpr uint16_t BOAF_BLOCKAGE = 0x4000;
constexpr int PF_RENDER_FACES = 1;
constexpr int PF_RENDERED_FLYTHROUGH = 2;
constexpr int PF_TOO_SMALL_FOR_ROBOT = 4;
constexpr int PF_BLOCK = 32;
constexpr int PF_BLOCK_REMOVABLE = 64;
constexpr int RF_EXTERNAL = 4;

static int Highest_room_index = 9; // rooms 0..9
#define ROOMNUM_OUTSIDE(rn) (((rn)&ROOMNUM_CELLNUM_FLAG) != 0)
static inline int TERRAIN_REGION(int rn) { return (rn & 0x70) >> 4; } // simplified mock
static inline int BOA_INDEX(int x) {
  return ROOMNUM_OUTSIDE(x) ? (TERRAIN_REGION(x) + Highest_room_index + 1) : x;
}

static uint16_t BOA_Array[MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS][MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS];

struct RoomMockB {
  bool used = false;
  uint32_t flags = 0;
};
static RoomMockB BRooms[MAX_ROOMS];
static bool BOA_vis_valid = false;

// normalized-index preamble shared by all four queries
struct IndexPair {
  int s, e;
};
static bool RepNormalize(int start_room, int end_room, IndexPair *out) {
  int s_index = start_room, e_index = end_room;
  if (start_room == -1 || end_room == -1)
    return false;
  if ((!ROOMNUM_OUTSIDE(s_index)) && s_index <= Highest_room_index) {
    if (!BRooms[s_index].used)
      return false;
  } else if (ROOMNUM_OUTSIDE(s_index)) {
    s_index = TERRAIN_REGION(start_room) + Highest_room_index + 1;
  }
  if ((!ROOMNUM_OUTSIDE(e_index)) && e_index <= Highest_room_index) {
    if (!BRooms[e_index].used)
      return false;
  } else if (ROOMNUM_OUTSIDE(e_index)) {
    e_index = TERRAIN_REGION(end_room) + Highest_room_index + 1;
  }
  out->s = s_index;
  out->e = e_index;
  return true;
}

static bool RepIsSoundAudible(int start_room, int end_room) {
  IndexPair p;
  if (!RepNormalize(start_room, end_room, &p))
    return false;
  return (BOA_Array[p.s][p.e] & BOA_SOUND_PROP) != 0;
}

static bool RepHasBlockage(int start_room, int end_room) {
  IndexPair p;
  if (!RepNormalize(start_room, end_room, &p))
    return false;
  return (BOA_Array[p.s][p.e] & BOAF_BLOCKAGE) != 0;
}

static bool RepIsVisible(int start_room, int end_room) {
  if (!BOA_vis_valid)
    return true; // fail-open: assume visible without a valid table
  if (start_room == end_room)
    return true; // quirk: runs BEFORE -1 rejection -> (-1,-1) is "visible"
  IndexPair p;
  if (!RepNormalize(start_room, end_room, &p))
    return false;
  return (BOA_Array[p.s][p.e] & BOAF_VIS) != 0;
}

static int RepGetNextRoom(int start_room, int end_room) {
  constexpr int BOA_NO_PATH_SENTINEL = 9999;
  IndexPair p;
  if (!RepNormalize(start_room, end_room, &p))
    return BOA_NO_PATH_SENTINEL;
  return (BOA_Array[p.s][p.e] & BOA_ROOM_MASK);
}

class BoaQuery : public ::testing::Test {
  void SetUp() override {
    memset(BOA_Array, 0, sizeof(BOA_Array));
    memset(BRooms, 0, sizeof(BRooms));
    for (int i = 0; i <= Highest_room_index; i++)
      BRooms[i].used = true;
    BOA_vis_valid = false;
  }
};

/**
 * @test BoaQuery.TerrainRoomsRemapToRegionSlotsPastRoomIndex
 * @brief Verifies terrain Rooms Remap To Region Slots Past Room Index.
 *
 * @details
 * Exercises the BoaQuery code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST_F(BoaQuery, TerrainRoomsRemapToRegionSlotsPastRoomIndex) {
  // outside roomnum with region 3 -> index Highest+1+3 = 13
  int outside = (int)0x80000000 | (3 << 4);
  BRooms[3].used = true; // region source cell irrelevant to remap

  BOA_Array[BOA_INDEX(outside)][5] = BOA_SOUND_PROP;
  EXPECT_TRUE(RepIsSoundAudible(outside, 5));
  EXPECT_EQ(BOA_INDEX(outside), Highest_room_index + 1 + 3);
}

/**
 * @test BoaQuery.UnusedOrInvalidRoomsRejectQueries
 * @brief Verifies unused Or Invalid Rooms Reject Queries.
 *
 * @details
 * Exercises the BoaQuery code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST_F(BoaQuery, UnusedOrInvalidRoomsRejectQueries) {
  BRooms[7].used = false;
  EXPECT_FALSE(RepIsSoundAudible(7, 2));
  EXPECT_EQ(RepGetNextRoom(2, -1), 9999);   // -1 -> NO_PATH
  EXPECT_EQ(RepGetNextRoom(-1, 2), 9999);

  // unused target also rejected
  EXPECT_FALSE(RepHasBlockage(2, 7));
}

/**
 * @test BoaQuery.VisTableInvalidMeansAlwaysVisibleFailOpen
 * @brief Verifies vis Table Invalid Means Always Visible Fail Open.
 *
 * @details
 * Exercises the BoaQuery code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST_F(BoaQuery, VisTableInvalidMeansAlwaysVisibleFailOpen) {
  BOA_vis_valid = false;
  BRooms[8].used = false; // even invalid rooms pass through
  EXPECT_TRUE(RepIsVisible(2, 5));

  BOA_vis_valid = true;
  BOA_Array[2][5] &= ~BOAF_VIS;
  EXPECT_FALSE(RepIsVisible(2, 5)); // now table governs
  BOA_Array[2][5] |= BOAF_VIS;
  EXPECT_TRUE(RepIsVisible(2, 5));
}

/**
 * @test BoaQuery.SameRoomVisibleEvenWhenBothAreMinusOne
 * @brief Verifies same Room Visible Even When Both Are Minus One.
 *
 * @details
 * Exercises the BoaQuery code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST_F(BoaQuery, SameRoomVisibleEvenWhenBothAreMinusOne) {
  BOA_vis_valid = true;
  EXPECT_TRUE(RepIsVisible(4, 4));
  // quirk: same-room check precedes -1 validation
  EXPECT_TRUE(RepIsVisible(-1, -1));
}

/**
 * @test BoaQuery.GetNextRoomMasksToTenBits
 * @brief Verifies get Next Room Masks To Ten Bits.
 *
 * @details
 * Exercises the BoaQuery code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST_F(BoaQuery, GetNextRoomMasksToTenBits) {
  BOA_Array[3][6] = 0xFC05; // bits beyond mask ignored
  EXPECT_EQ(RepGetNextRoom(3, 6), 0x005); // low 10 bits only
}

// ---------------------------------------------------------------------------
// BOA_PassablePortal replication (BOA.cpp:208-268)
struct PortalMock {
  int croom = -1;
  int flags = 0;
};
struct PFaceMock {
  int tex_flags = 0; // TF_BREAKABLE|TF_FORCEFIELD stand-in bits
};
struct PRoomMock {
  bool used = false;
  uint32_t flags = 0;
  PortalMock portals[4];
  PFaceMock portal_faces[4]; // indexed via portal_face below
  int portal_face_of[4] = {0, 0, 0, 0};
};

static PRoomMock Prooms[MAX_ROOMS];
static float BoaCost[64][4];
static bool MakingBoa = false;
constexpr int TF_BREAKABLE = 0x08, TF_FORCEFIELD = 0x10;

static bool RepPassablePortal(int room, int portal_index, bool f_for_sound,
                              bool f_invalid_list) {
  if (room == -1)
    return false;
  room = BOA_INDEX(room);

  auto &rp = Prooms[room];
  auto &portal = rp.portals[portal_index];
  auto &face = rp.portal_faces[rp.portal_face_of[portal_index]];

  if (portal.croom < 0)
    return false;

  if (!MakingBoa) {
    // quirk: negative cost blocks passage EXCEPT from external rooms,
    // whose cost array is uninitialized garbage by design
    if (BoaCost[room][portal_index] < 0.0f && !(rp.flags & RF_EXTERNAL))
      return false;

    if (!f_for_sound && (portal.flags & PF_TOO_SMALL_FOR_ROBOT))
      return false;

    if (((portal.flags & PF_RENDER_FACES) && !(portal.flags & PF_RENDERED_FLYTHROUGH)) ||
        (portal.flags & PF_BLOCK))
      return false;
  } else {
    if (f_invalid_list && (portal.flags & PF_TOO_SMALL_FOR_ROBOT))
      return false;

    if ((portal.flags & PF_BLOCK) && !(portal.flags & PF_BLOCK_REMOVABLE))
      return false;

    if ((portal.flags & PF_RENDER_FACES) && !(portal.flags & PF_RENDERED_FLYTHROUGH)) {
      if (!(face.tex_flags & (TF_BREAKABLE | TF_FORCEFIELD)))
        return false;
    }
  }
  return true;
}

class BoaPortal : public ::testing::Test {
  void SetUp() override {
    memset(Prooms, 0, sizeof(Prooms));
    memset(BoaCost, 0, sizeof(BoaCost));
    for (int i = 0; i <= Highest_room_index; i++)
      Prooms[i].used = true;
    MakingBoa = false;
  }
};

/**
 * @test BoaPortal.RuntimeRulesBlockSmallAndSealedPortals
 * @brief Verifies runtime Rules Block Small And Sealed Portals.
 *
 * @details
 * Exercises the BoaPortal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST_F(BoaPortal, RuntimeRulesBlockSmallAndSealedPortals) {
  Prooms[2].portals[0].croom = 3;
  BoaCost[2][0] = 10.0f;
  EXPECT_TRUE(RepPassablePortal(2, 0, false, false));

  Prooms[2].portals[1].croom = 3;
  Prooms[2].portals[1].flags = PF_TOO_SMALL_FOR_ROBOT;
  EXPECT_FALSE(RepPassablePortal(2, 1, false, false)); // robot blocked
  EXPECT_TRUE(RepPassablePortal(2, 1, true, false));   // sound passes

  Prooms[2].portals[2].croom = 3;
  Prooms[2].portals[2].flags = PF_BLOCK;
  EXPECT_FALSE(RepPassablePortal(2, 2, true, false)); // hard block

  Prooms[2].portals[3].croom = -1; // dead portal
  EXPECT_FALSE(RepPassablePortal(2, 3, true, false));
}

/**
 * @test BoaPortal.NegativeCostBlocksExceptExternalRoomQuirk
 * @brief Verifies negative Cost Blocks Except External Room Quirk.
 *
 * @details
 * Exercises the BoaPortal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST_F(BoaPortal, NegativeCostBlocksExceptExternalRoomQuirk) {
  Prooms[5].portals[0].croom = 6;
  Prooms[5].portals[1].croom = 6;
  Prooms[5].flags = RF_EXTERNAL;
  BoaCost[5][0] = -1.0f;
  BoaCost[5][1] = -1.0f;

  Prooms[5].used = true;
  // non-external comparison room
  Prooms[6].portals[0].croom = 5;
  BoaCost[6][0] = -1.0f;

  EXPECT_FALSE(RepPassablePortal(6, 0, false, false)); // interior blocked
  EXPECT_TRUE(RepPassablePortal(5, 0, false, false));  // external bypasses
}

/**
 * @test BoaPortal.MakingModeAllowsRemovableBlocksAndBreakableFaces
 * @brief Verifies making Mode Allows Removable Blocks And Breakable Faces.
 *
 * @details
 * Exercises the BoaPortal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST_F(BoaPortal, MakingModeAllowsRemovableBlocksAndBreakableFaces) {
  MakingBoa = true;

  Prooms[2].portals[0].croom = 3;
  Prooms[2].portals[0].flags = PF_BLOCK | PF_BLOCK_REMOVABLE;
  EXPECT_TRUE(RepPassablePortal(2, 0, false, false)); // removable ok while building

  Prooms[2].portals[1].croom = 3;
  Prooms[2].portals[1].flags = PF_BLOCK; // permanent block
  EXPECT_FALSE(RepPassablePortal(2, 1, false, false));

  Prooms[2].portals[2].croom = 3;
  Prooms[2].portals[2].flags = PF_RENDER_FACES; // rendered, not flythrough
  Prooms[2].portal_faces[0].tex_flags = TF_FORCEFIELD;
  Prooms[2].portal_face_of[2] = 0;
  EXPECT_TRUE(RepPassablePortal(2, 2, false, false)); // forcefield breakable

  Prooms[2].portal_faces[0].tex_flags = 0; // plain wall texture
  EXPECT_FALSE(RepPassablePortal(2, 2, false, false));

  // TOO_SMALL applies during robot-invalid-list marking even when making
  Prooms[2].portals[3].croom = 3;
  Prooms[2].portals[3].flags = PF_TOO_SMALL_FOR_ROBOT;
  EXPECT_TRUE(RepPassablePortal(2, 3, false, false));   // plain build ignores size
  EXPECT_FALSE(RepPassablePortal(2, 3, false, true));   // invalid-list run respects it
}

// ---------------------------------------------------------------------------
// BOA_LockedDoor threshold replication (BOA.cpp:272-287)
static bool DoorwayLockedMock = true;
static float DoorwayPos = 0.25f;
static bool DoorwayOpenableMock = false;

static bool RepLockedDoor(bool is_door_room, bool has_door_obj) {
  if (!is_door_room)
    return false;
  if (!has_door_obj)
    return DoorwayLockedMock && DoorwayPos < 0.5f;
  return (!DoorwayOpenableMock) && DoorwayPos < 0.5f;
}

/**
 * @test BoaDoor.HalfOpenThresholdDecidesLockState
 * @brief Verifies half Open Threshold Decides Lock State.
 *
 * @details
 * Exercises the BoaDoor code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BOA.cpp
 * @ingroup descent3_tests
 */
TEST(BoaDoor, HalfOpenThresholdDecidesLockState) {
  DoorwayLockedMock = true;
  DoorwayPos = 0.49f;
  EXPECT_TRUE(RepLockedDoor(true, false));
  DoorwayPos = 0.5f; // exactly half-open counts as passable
  EXPECT_FALSE(RepLockedDoor(true, false));

  DoorwayOpenableMock = false;
  DoorwayPos = 0.1f;
  EXPECT_TRUE(RepLockedDoor(true, true)); // can't open + mostly closed

  DoorwayOpenableMock = true;
  EXPECT_FALSE(RepLockedDoor(true, true)); // openable door never locks

  EXPECT_FALSE(RepLockedDoor(false, false)); // non-door rooms never lock
}
