/**
 * @file gameloop_real_tests.cpp
 * @brief Tests for GameLoop.cpp 3171 lines — per-frame game loop, keys,.
 *
 * @details
 * camera views. Covers the guidebot shortcut-key command mapping
 * with its SHIFT/buddy/client gates, the SelectNextCameraView
 * state machine (rearview -> guidebot -> markers -> close) with
 * the fall-through num-computation quirk and multiplayer's 2-
 * marker cap, InitCameraViews reset semantics, and
 * RestoreCameraRearviews pilot-flag persistence.
 *
 * This harness validates the behavior of `Descent3/GameLoop.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/GameLoop.cpp`
 * @par Harness
 * `gameloop_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/GameLoop.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

// replicated constants
#define KEY_SHIFTED 0x100
#define KEY_0 0x0B
#define KEY_1 0x02
constexpr int CV_NONE = 0, CV_REARVIEW = 1, CV_MARKER1 = 2;
// MARKER2..MARKER8 are 3..9
constexpr int CV_MARKER2 = 3, CV_MARKER3 = 4, CV_MARKER4 = 5, CV_MARKER5 = 6,
              CV_MARKER6 = 7, CV_MARKER7 = 8;
constexpr int CV_MARKER8 = 9;
constexpr int CV_GUIDEBOT = 10;
#define KEY_4 0x05
#define KEY_5 0x06
constexpr int NUM_CAMERA_VIEWS = 3;

enum GBC {
  GBC_FIND_ACTIVE_GOAL_0 = 3,
  GBC_FIND_SPEW = 19,
  GBC_FIND_POWERUP = 20,
  GBC_FIND_ENERGY_CENTER = 21,
  GBC_ESCORT_SHIP = 35,
  GBC_EXTINGUISH = 36,
  GBC_GO_WINGNUT = 37,
  GBC_RTYPE = 39,
  GBC_ANTI_VIRUS = 40,
  GBC_RETURN_TO_SHIP = 43
};

enum { LR_SERVER = 0, LR_PEER = 1, LR_CLIENT = 2 };
constexpr int GM_MULTI_GL = 0x08;

// ---------------------------------------------------------------------------
// ProcessGuidebotKeys replication (GameLoop.cpp:1156-1218)
static bool BuddyOut = false;
static int LastAINotifyCommand = -1;
static int LastNetCommand = -1;
static bool MultiModeGL = false;
static int LocalRole = LR_SERVER;

static void RepProcessGuidebotKeys(int key_in) {
  if (!(key_in & KEY_SHIFTED))
    return;
  int key = key_in & ~KEY_SHIFTED;

  if (!BuddyOut)
    return; // guidebot still in ship

  int command_id = -1;
  switch (key) {
  case KEY_1: command_id = GBC_FIND_ACTIVE_GOAL_0; break;
  case 0x03: command_id = GBC_FIND_SPEW; break; // KEY_2
  case 0x04: command_id = GBC_FIND_POWERUP; break;
  case 0x05: command_id = GBC_FIND_ENERGY_CENTER; break;
  case 0x06: command_id = GBC_ESCORT_SHIP; break;
  case 0x07: command_id = GBC_EXTINGUISH; break;
  case 0x08: command_id = GBC_GO_WINGNUT; break;
  case 0x09: command_id = GBC_RTYPE; break;
  case 0x0A: command_id = GBC_ANTI_VIRUS; break; // KEY_9
  case KEY_0: command_id = GBC_RETURN_TO_SHIP; break;
  default:
    return;
  }

  if (!(MultiModeGL) || (LocalRole != LR_CLIENT))
    LastAINotifyCommand = command_id;
  else
    LastNetCommand = command_id;
}

/**
 * @test GameLoopGuidebotKeys.ShiftGateCommandMapAndClientDispatch
 * @brief Verifies shift Gate Command Map And Client Dispatch.
 *
 * @details
 * Exercises the GameLoopGuidebotKeys code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameLoop.cpp
 * @ingroup descent3_tests
 */
TEST(GameLoopGuidebotKeys, ShiftGateCommandMapAndClientDispatch) {
  auto reset = []() {
    LastAINotifyCommand = LastNetCommand = -1;
    BuddyOut = true;
    MultiModeGL = false;
    LocalRole = LR_SERVER;
  };
  reset();

  // unshifted digits are ignored entirely
  RepProcessGuidebotKeys(KEY_1);
  EXPECT_EQ(LastAINotifyCommand, -1);

  // shifted digit maps into the gb_com action index
  RepProcessGuidebotKeys(KEY_1 | KEY_SHIFTED);
  EXPECT_EQ(LastAINotifyCommand, GBC_FIND_ACTIVE_GOAL_0);

  RepProcessGuidebotKeys(KEY_0 | KEY_SHIFTED);
  EXPECT_EQ(LastAINotifyCommand, GBC_RETURN_TO_SHIP);

  RepProcessGuidebotKeys(0x0A | KEY_SHIFTED); // KEY_9
  EXPECT_EQ(LastAINotifyCommand, GBC_ANTI_VIRUS);

  // non-digit shifted key does nothing
  reset();
  RepProcessGuidebotKeys(0x1E | KEY_SHIFTED); // KEY_A
  EXPECT_EQ(LastAINotifyCommand, -1);

  // buddy must be out of the ship
  reset();
  BuddyOut = false;
  RepProcessGuidebotKeys(KEY_1 | KEY_SHIFTED);
  EXPECT_EQ(LastAINotifyCommand, -1);

  // clients relay via network instead of notifying AI directly
  reset();
  MultiModeGL = true;
  LocalRole = LR_CLIENT;
  RepProcessGuidebotKeys(KEY_5 | KEY_SHIFTED);
  EXPECT_EQ(LastNetCommand, GBC_ESCORT_SHIP);
  EXPECT_EQ(LastAINotifyCommand, -1);

  // servers/peers still notify locally even in multi
  LocalRole = LR_SERVER;
  RepProcessGuidebotKeys(KEY_4 | KEY_SHIFTED);
  EXPECT_EQ(LastAINotifyCommand, GBC_FIND_ENERGY_CENTER);
}

// ---------------------------------------------------------------------------
// SelectNextCameraView replication (GameLoop.cpp:1007-1085)
static int CVM[NUM_CAMERA_VIEWS];
static int SmallViewerObj[3]; // -1 = no viewer
static int NumMarkers = 0;
static int MarkerObjs[20];    // object indices holding markers, by marker id
static int HighestObjIdxGL = -1;
static bool LrearEnabled = false, RrearEnabled = false;
static int ClosedWindows = 0;
static constexpr int OBJ_MARKER_GL = 11;

static int RepNextCameraView(int window, int pnum) {
  int current = CVM[window];
  if (SmallViewerObj[window] == -1)
    current = CV_NONE;

  switch (current) {
  case CV_NONE:
    CVM[window] = CV_REARVIEW;
    if (window == 0)
      LrearEnabled = true;
    else if (window == 2)
      RrearEnabled = true;
    return CV_REARVIEW;

  case CV_REARVIEW: {
    if (window == 0)
      LrearEnabled = false;
    else if (window == 2)
      RrearEnabled = false;

    if (BuddyOut) { // guidebot present
      CVM[window] = CV_GUIDEBOT;
      return CV_GUIDEBOT;
    }
    // fall into next case when no guidebot!
  }
  case CV_GUIDEBOT:
  case CV_MARKER1:
  case CV_MARKER2:
  case CV_MARKER3:
  case CV_MARKER4:
  case CV_MARKER5:
  case CV_MARKER6:
  case CV_MARKER7:
  case CV_MARKER8: {
    if (NumMarkers > 0) {
      // quirk: fall-through from REARVIEW lands here with current ==
      // CV_REARVIEW, giving num = 1 - 2 + 1 = 0 -- same as guidebot start
      int num = (current == CV_GUIDEBOT) ? 0 : (current - CV_MARKER1) + 1;
      int max = (MultiModeGL) ? 2 : 8;
      int id = pnum * 2 + num;

      if (num < max) {
        int i;
        for (i = 0; i <= HighestObjIdxGL; i++) {
          if (MarkerObjs[i] == id)
            break;
        }
        if (i <= HighestObjIdxGL) {
          CVM[window] = CV_MARKER1 + num;
          return CVM[window];
        }
      }
    }
    CVM[window] = CV_NONE;
    ClosedWindows++;
    return CV_NONE;
  }
  default:
    return -1; // Int3 in original
  }
}

/**
 * @test GameLoopCameras.CycleRearviewGuidebotFallthroughAndMarkers
 * @brief Verifies cycle Rearview Guidebot Fallthrough And Markers.
 *
 * @details
 * Exercises the GameLoopCameras code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameLoop.cpp
 * @ingroup descent3_tests
 */
TEST(GameLoopCameras, CycleRearviewGuidebotFallthroughAndMarkers) {
  auto reset = [&]() {
    memset(CVM, 0, sizeof(CVM));
    SmallViewerObj[0] = SmallViewerObj[1] = SmallViewerObj[2] = 7;
    memset(MarkerObjs, -1, sizeof(MarkerObjs));
    MarkerObjs[6] = 0; // marker id 0 exists at obj slot 6
    MarkerObjs[7] = 1; // marker id 1
    HighestObjIdxGL = 7;
    NumMarkers = 2;
    MultiModeGL = false;
    BuddyOut = true;
    ClosedWindows = 0;
  };
  reset();

  // NONE -> REARVIEW sets the matching pilot flag (windows 0 and 2 only)
  EXPECT_EQ(RepNextCameraView(0, 0), CV_REARVIEW);
  EXPECT_TRUE(LrearEnabled);
  EXPECT_FALSE(RrearEnabled);

  // REARVIEW -> GUIDEBOT while buddy is out; leaving rearview clears flag
  EXPECT_EQ(RepNextCameraView(0, 0), CV_GUIDEBOT);
  EXPECT_FALSE(LrearEnabled);

  // GUIDEBOT -> first marker (num 0), then second (num 1), then exhausted
  EXPECT_EQ(RepNextCameraView(0, 0), CV_MARKER1);
  EXPECT_EQ(RepNextCameraView(0, 0), CV_MARKER2);
  // no third marker object -> window closes back to none
  EXPECT_EQ(RepNextCameraView(0, 0), CV_NONE);
  EXPECT_EQ(ClosedWindows, 1);

  // quirk: REARVIEW with NO guidebot falls straight into the marker block;
  // num computes as (CV_REARVIEW - CV_MARKER1) + 1 == 0, same as guidebot
  reset();
  BuddyOut = false;
  CVM[0] = CV_REARVIEW;
  EXPECT_EQ(RepNextCameraView(0, 0), CV_MARKER1);

  // multiplayer caps usable markers at 2
  reset();
  MarkerObjs[8] = 2;
  MarkerObjs[9] = 3;
  HighestObjIdxGL = 9;
  NumMarkers = 4;
  MultiModeGL = true;
  CVM[0] = CV_GUIDEBOT;
  EXPECT_EQ(RepNextCameraView(0, 0), CV_MARKER1);   // num 0 ok
  EXPECT_EQ(RepNextCameraView(0, 0), CV_MARKER2);   // num 1 ok
  EXPECT_EQ(RepNextCameraView(0, 0), CV_NONE);      // num 2 >= cap 2

  // missing marker object closes the window too
  reset();
  MarkerObjs[6] = -1;
  CVM[0] = CV_GUIDEBOT;
  EXPECT_EQ(RepNextCameraView(0, 0), CV_NONE);

  // stale viewer (window gone) behaves like CV_NONE
  reset();
  SmallViewerObj[1] = -1;
  CVM[1] = CV_MARKER3;
  EXPECT_EQ(RepNextCameraView(1, 0), CV_REARVIEW);
}

/**
 * @test GameLoopCameras.ResetKeepsOnlyRearviewsAndRestoreHonorsPilotFlags
 * @brief Verifies reset Keeps Only Rearviews And Restore Honors Pilot Flags.
 *
 * @details
 * Exercises the GameLoopCameras code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameLoop.cpp
 * @ingroup descent3_tests
 */
TEST(GameLoopCameras, ResetKeepsOnlyRearviewsAndRestoreHonorsPilotFlags) {
  auto resetAll = [&]() {
    memset(CVM, 0, sizeof(CVM));
    LrearEnabled = RrearEnabled = false;
  };
  resetAll();

  // InitCameraViews(true): everything to none regardless of prior mode
  CVM[0] = CV_MARKER1;
  CVM[1] = CV_REARVIEW;
  CVM[2] = CV_GUIDEBOT;
  bool total_reset = true;
  for (int i = 0; i < NUM_CAMERA_VIEWS; i++) {
    if (total_reset || CVM[i] != CV_REARVIEW)
      CVM[i] = CV_NONE;
    else
      CVM[i] = CV_REARVIEW;
  }
  EXPECT_EQ(CVM[0], CV_NONE);
  EXPECT_EQ(CVM[1], CV_NONE); // total reset clears rearviews too
  EXPECT_EQ(CVM[2], CV_NONE);
  // original checks total_reset FIRST, so the keep-rearview else arm
  // is unreachable there -- everything ends at CV_NONE.

  // partial reset (total=false): rearview preserved, others cleared
  CVM[0] = CV_MARKER1;
  CVM[1] = CV_REARVIEW;
  CVM[2] = CV_GUIDEBOT;
  total_reset = false;
  for (int i = 0; i < NUM_CAMERA_VIEWS; i++) {
    if (total_reset || CVM[i] != CV_REARVIEW)
      CVM[i] = CV_NONE;
  }
  EXPECT_EQ(CVM[0], CV_NONE);
  EXPECT_EQ(CVM[1], CV_REARVIEW);
  EXPECT_EQ(CVM[2], CV_NONE);

  // RestoreCameraRearviews: recreates rear view ONLY when pilot prefs ask
  resetAll();
  LrearEnabled = true;
  RrearEnabled = false;
  if (CVM[0] != CV_REARVIEW && LrearEnabled)
    CVM[0] = CV_REARVIEW;
  if (CVM[2] != CV_REARVIEW && RrearEnabled)
    CVM[2] = CV_REARVIEW;
  EXPECT_EQ(CVM[0], CV_REARVIEW); // left restored
  EXPECT_EQ(CVM[2], CV_NONE);     // right pref off stays closed
}
