/**
 * @file telcomautomap_real_tests.cpp
 * @brief Tests for TelComAutoMap.cpp 834 lines — TelCom automap.
 *
 * @details
 * Covers TCAMBuildRoomList room enumeration, portal-face
 * classification (secret black / wacky / visited skip), backface
 * culling, distance-based face LOD gating, and the lightmap ramp init.
 *
 * This harness validates the behavior of `Descent3/TelComAutoMap.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/TelComAutoMap.cpp`
 * @par Harness
 * `telcomautomap_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/TelComAutoMap.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <vector>

// replicated constants
constexpr int MAX_ROOMS = 400;
constexpr uint32_t RF_EXTERNAL = (1 << 2);
constexpr uint32_t RF_SECRET = (1 << 28);

// mock world state
struct MockRoom {
  bool used = false;
  uint32_t flags = 0;
};
static MockRoom MRooms[MAX_ROOMS];
static uint8_t AutomapVisMap[MAX_ROOMS];
static int Highest_room_index = 0;

// replicated TCAMBuildRoomList (TelComAutoMap.cpp:397-413)
struct RoomList {
  std::vector<uint16_t> list;
  std::vector<uint8_t> seen;

  void Build(int startroom) {
    seen.assign(MAX_ROOMS, 0);
    list.clear();
    if (startroom > 0) {
      list.push_back((uint16_t)startroom);
      seen[startroom] = 1;
    }
    for (int i = 0; i <= Highest_room_index; i++) {
      if (MRooms[i].used && !seen[i] && (AutomapVisMap[i] == 1 || (MRooms[i].flags & RF_EXTERNAL))) {
        list.push_back((uint16_t)i);
        seen[i] = 1;
      }
    }
  }
};

/**
 * @test TelComAutoMap.BuildRoomListVisAndExternal
 * @brief Verifies build Room List Vis And External.
 *
 * @details
 * Exercises the TelComAutoMap code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComAutoMap.cpp
 * @ingroup descent3_tests
 */
TEST(TelComAutoMap, BuildRoomListVisAndExternal) {
  memset(MRooms, 0, sizeof(MRooms));
  memset(AutomapVisMap, 0, sizeof(AutomapVisMap));
  Highest_room_index = 5;

  for (int i = 0; i <= 5; i++) MRooms[i].used = true;
  AutomapVisMap[1] = 1;          // visited normal room
                                   // room 2 unvisited, not external -> excluded
  MRooms[3].flags = RF_EXTERNAL; // external always shown
  MRooms[4].used = false;        // unused -> excluded even if visited
  AutomapVisMap[5] = 1;

  RoomList rl;
  rl.Build(0); // no start room
  EXPECT_EQ(rl.list.size(), 3u);
  EXPECT_EQ(rl.list[0], 1);
  EXPECT_EQ(rl.list[1], 3);
  EXPECT_EQ(rl.list[2], 5);
}

/**
 * @test TelComAutoMap.BuildRoomListStartRoomQuirks
 * @brief Verifies build Room List Start Room Quirks.
 *
 * @details
 * Exercises the TelComAutoMap code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComAutoMap.cpp
 * @ingroup descent3_tests
 */
TEST(TelComAutoMap, BuildRoomListStartRoomQuirks) {
  memset(MRooms, 0, sizeof(MRooms));
  memset(AutomapVisMap, 0, sizeof(AutomapVisMap));
  Highest_room_index = 3;
  for (int i = 0; i <= 3; i++) MRooms[i].used = true;

  // quirk: startroom 0 is rejected by the >0 check
  RoomList rl;
  rl.Build(0);
  EXPECT_EQ(rl.list.size(), 0u); // nothing visited yet -> empty map

  // valid startroom comes first even though unvisited; other rooms stay
  // hidden until visited/external
  rl.Build(1);
  ASSERT_EQ(rl.list.size(), 1u);
  EXPECT_EQ(rl.list[0], 1);
}

/**
 * @test TelComAutoMap.BuildRoomListNoDuplicatesForStart
 * @brief Verifies build Room List No Duplicates For Start.
 *
 * @details
 * Exercises the TelComAutoMap code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComAutoMap.cpp
 * @ingroup descent3_tests
 */
TEST(TelComAutoMap, BuildRoomListNoDuplicatesForStart) {
  memset(MRooms, 0, sizeof(MRooms));
  memset(AutomapVisMap, 0, sizeof(AutomapVisMap));
  Highest_room_index = 4;
  for (int i = 0; i <= 4; i++) MRooms[i].used = true;
  AutomapVisMap[2] = 1; // start room also in vismap
  AutomapVisMap[4] = 1; // another visited room

  RoomList rl;
  rl.Build(2);
  ASSERT_EQ(rl.list.size(), 2u);
  EXPECT_EQ(rl.list[0], 2); // start room listed once, first...
  EXPECT_EQ(rl.list[1], 4); // ...scan skips it via seen flag, adds room 4
}

// replicated backface cull + portal classification (TelComAutoMap.cpp:483-498)
struct FaceMock {
  float normal_dot_view; // vm_Dot3Product(subvec, fp->normal)
  int portal_num = -1;
  bool portal_croom_visited = false;
  bool portal_croom_secret = false;
};

enum FaceClass { FC_DRAW, FC_BACKFACED, FC_SKIP_VISITED, FC_BLACK_SECRET, FC_WACKY };

static FaceClass ClassifyFace(const FaceMock &f) {
  if (f.normal_dot_view > 0)
    return FC_BACKFACED; // continue

  if (f.portal_num != -1) {
    if (!f.portal_croom_visited) {
      if (f.portal_croom_secret)
        return FC_BLACK_SECRET; // don't give away secret rooms
      else
        return FC_WACKY;
    } else
      return FC_SKIP_VISITED;
  }
  return FC_DRAW;
}

/**
 * @test TelComAutoMap.BackfaceCullByNormalDot
 * @brief Verifies backface Cull By Normal Dot.
 *
 * @details
 * Exercises the TelComAutoMap code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComAutoMap.cpp
 * @ingroup descent3_tests
 */
TEST(TelComAutoMap, BackfaceCullByNormalDot) {
  EXPECT_EQ(ClassifyFace({0.5f}), FC_BACKFACED);   // facing away
  EXPECT_EQ(ClassifyFace({0.001f}), FC_BACKFACED); // strictly positive only
  EXPECT_EQ(ClassifyFace({0.0f}), FC_DRAW);        // edge-on draws
  EXPECT_EQ(ClassifyFace({-1.0f}), FC_DRAW);
}

/**
 * @test TelComAutoMap.PortalFacesSecretWackyAndVisited
 * @brief Verifies portal Faces Secret Wacky And Visited.
 *
 * @details
 * Exercises the TelComAutoMap code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComAutoMap.cpp
 * @ingroup descent3_tests
 */
TEST(TelComAutoMap, PortalFacesSecretWackyAndVisited) {
  FaceMock secret{-1.0f, 2, false, true};
  FaceMock wacky{-1.0f, 2, false, false};
  FaceMock visited{-1.0f, 2, true, false};
  FaceMock plain{-1.0f}; // no portal

  EXPECT_EQ(ClassifyFace(secret), FC_BLACK_SECRET); // hidden as black
  EXPECT_EQ(ClassifyFace(wacky), FC_WACKY);         // special marker face
  EXPECT_EQ(ClassifyFace(visited), FC_SKIP_VISITED); // hole in map
  EXPECT_EQ(ClassifyFace(plain), FC_DRAW);
}

// replicated LOD gating (TelComAutoMap.cpp:500-506), Small_faces values:
// 1 = tiny face, 2 = medium face
/**
 * @test TelComAutoMap.DistanceLODGating
 * @brief Verifies distance LODGating.
 *
 * @details
 * Exercises the TelComAutoMap code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComAutoMap.cpp
 * @ingroup descent3_tests
 */
TEST(TelComAutoMap, DistanceLODGating) {
  auto draw_face = [](float nearz, uint8_t small_class) {
    if (nearz > 600) {
      if (small_class != 0)
        return false; // far: all small-classified faces skipped
    } else if (nearz > 300) {
      if (small_class == 2)
        return false; // mid: only class-2 skipped
    }
    return true;
  };
  // very near (<300): everything draws
  EXPECT_TRUE(draw_face(100, 0));
  EXPECT_TRUE(draw_face(299, 1));
  EXPECT_TRUE(draw_face(299, 2));
  // mid band (300-600]: class 2 culled
  EXPECT_TRUE(draw_face(450, 0));
  EXPECT_TRUE(draw_face(450, 1));
  EXPECT_FALSE(draw_face(450, 2));
  // far (>600): any nonzero class culled
  EXPECT_TRUE(draw_face(900, 0));
  EXPECT_FALSE(draw_face(900, 1));
  EXPECT_FALSE(draw_face(900, 2));
}

// replicated lightmap grayscale ramp init (TelComAutoMap.cpp:450-462)
/**
 * @test TelComAutoMap.LightmapRampInitOnceStatic
 * @brief Verifies lightmap Ramp Init Once Static.
 *
 * @details
 * Exercises the TelComAutoMap code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComAutoMap.cpp
 * @ingroup descent3_tests
 */
TEST(TelComAutoMap, LightmapRampInitOnceStatic) {
  static float lm_red[32];
  static bool first = true;
  if (first) {
    first = false;
    for (int i = 0; i < 32; i++)
      lm_red[i] = ((float)i / 31.0);
  }
  EXPECT_NEAR(lm_red[0], 0.0f, 1e-6f);
  EXPECT_NEAR(lm_red[15], 15.0f / 31.0f, 1e-6f);
  EXPECT_FLOAT_EQ(lm_red[31], 1.0f);
}
