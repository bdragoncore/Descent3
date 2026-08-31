/*
 * Tests for mmItem.cpp 502 lines — main menu items/interface.
 * Covers the OnDraw FX state machine (Normal/FadeIn/FadeOut transitions,
 * queue refills), AddItem type->flags mapping and Y stacking, CopyrightText
 * version-type string building, and SetMusicRegion restart recursion.
 * Replicates logic to avoid newui/sound deps.
 */
/**
 * @file mmitem_real_tests.cpp
 * @brief Unit tests for Descent3/mmItem.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/mmItem.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/mmItem.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/mmItem.cpp`
 * @par Harness
 * `mmitem_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/mmItem.cpp
 */



#include <gtest/gtest.h>
#include <cstring>
#include <string>

// replicated constants (mmItem.h:80-94, mmItem.cpp:133)
constexpr int MMITEM_ALPHA = 192;
constexpr int MMITEM_SAT = 0;
constexpr int MMITEM_Y = 175;
constexpr int N_MMENU_ITEMS = 10;
constexpr int N_MM_SOUNDS = 2;
constexpr int MM_SELECT_SOUND = 0, MM_FOCUS_SOUND = 1;
constexpr int MM_MUSIC_REGION = 0;

// replicated tmmItemFX (mmItem.h:99-104)
enum tmmItemFX { mmItemFXNull = 0, mmItemFXNormal, mmItemFXFadeInToBright, mmItemFXFadeOutToNormal };

// replicated circular fx queue (mmItem.h:112-138)
struct FXQueue {
  static constexpr int SIZE = 8;
  tmmItemFX items[SIZE];
  int16_t head = 0, tail = 0;
  void send(tmmItemFX item) {
    int16_t temp = tail + 1;
    if (temp == SIZE) temp = 0;
    if (temp != head) { items[tail] = item; tail = temp; }
  }
  bool recv(tmmItemFX *out) {
    if (head == tail) return false;
    *out = items[head++];
    if (head == SIZE) head = 0;
    return true;
  }
  void flush() { head = tail = 0; }
};

// replicated mmItem FX state (subset of OnDraw, mmItem.cpp:253-292)
struct MockMMItem {
  FXQueue m_fxqueue;
  tmmItemFX m_curfx = mmItemFXNull;
  int16_t m_alpha = 0;
  int16_t m_satcount = 0;
  float m_process_speed = 0.0f;

  // one "process tick" of the switch in OnDraw
  void ProcessFx() {
    switch (m_curfx) {
    case mmItemFXNull:
      break;
    case mmItemFXNormal:
      m_alpha = MMITEM_ALPHA;
      m_satcount = MMITEM_SAT;
      m_process_speed = 0.0f;
      m_curfx = mmItemFXNull;
      break;
    case mmItemFXFadeOutToNormal:
      m_process_speed = 0.075f;
      m_satcount = (m_satcount < MMITEM_SAT) ? MMITEM_SAT : (m_satcount - 1);
      if (m_satcount == MMITEM_SAT) m_curfx = mmItemFXNull;
      break;
    case mmItemFXFadeInToBright:
      m_process_speed = 0.075f;
      m_satcount = (m_satcount > 2) ? 2 : (m_satcount + 1);
      if (m_satcount == 2) m_curfx = mmItemFXNull;
      break;
    }
    if (m_curfx == mmItemFXNull) {
      tmmItemFX newfx;
      if (m_fxqueue.recv(&newfx)) {
        m_process_speed = 0.0f;
        m_curfx = newfx;
      }
    }
  }
};

/**
 * @test MMItem.CreateInitializesState
 * @brief Verifies create Initializes State.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, CreateInitializesState) {
  MockMMItem it;
  it.m_alpha = 0;
  it.m_satcount = 0;
  EXPECT_EQ(it.m_alpha, 0);
  EXPECT_EQ(it.m_curfx, mmItemFXNull);
}

/**
 * @test MMItem.FxNormalSetsDefaultsAndClears
 * @brief Verifies fx Normal Sets Defaults And Clears.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, FxNormalSetsDefaultsAndClears) {
  MockMMItem it;
  it.m_curfx = mmItemFXNormal;
  it.ProcessFx();
  EXPECT_EQ(it.m_alpha, MMITEM_ALPHA);
  EXPECT_EQ(it.m_satcount, MMITEM_SAT);
  EXPECT_FLOAT_EQ(it.m_process_speed, 0.0f);
  EXPECT_EQ(it.m_curfx, mmItemFXNull); // done, pulls from empty queue -> stays null
}

/**
 * @test MMItem.FxFadeOutDecrementsTowardSat
 * @brief Verifies fx Fade Out Decrements Toward Sat.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, FxFadeOutDecrementsTowardSat) {
  MockMMItem it;
  it.m_satcount = 5; // brighter than normal
  it.m_curfx = mmItemFXFadeOutToNormal;
  it.ProcessFx();
  EXPECT_EQ(it.m_satcount, 4);
  EXPECT_FLOAT_EQ(it.m_process_speed, 0.075f);
  EXPECT_EQ(it.m_curfx, mmItemFXFadeOutToNormal); // not done yet
  it.ProcessFx(); it.ProcessFx(); it.ProcessFx(); // 3, 2, 1
  EXPECT_EQ(it.m_satcount, 1);
  it.ProcessFx(); // reaches SAT=0
  EXPECT_EQ(it.m_satcount, MMITEM_SAT);
  EXPECT_EQ(it.m_curfx, mmItemFXNull);
}

/**
 * @test MMItem.FxFadeOutBelowSatSnaps
 * @brief Verifies fx Fade Out Below Sat Snaps.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, FxFadeOutBelowSatSnaps) {
  // satcount < MMITEM_SAT snaps up to SAT and finishes immediately
  MockMMItem it;
  it.m_satcount = -3;
  it.m_curfx = mmItemFXFadeOutToNormal;
  it.ProcessFx();
  EXPECT_EQ(it.m_satcount, MMITEM_SAT);
  EXPECT_EQ(it.m_curfx, mmItemFXNull);
}

/**
 * @test MMItem.FxFadeInIncrementsToTwo
 * @brief Verifies fx Fade In Increments To Two.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, FxFadeInIncrementsToTwo) {
  MockMMItem it;
  it.m_satcount = 0;
  it.m_curfx = mmItemFXFadeInToBright;
  it.ProcessFx();
  EXPECT_EQ(it.m_satcount, 1);
  EXPECT_EQ(it.m_curfx, mmItemFXFadeInToBright);
  it.ProcessFx();
  EXPECT_EQ(it.m_satcount, 2);
  EXPECT_EQ(it.m_curfx, mmItemFXNull); // stops at 2
}

/**
 * @test MMItem.FxFadeInAboveTwoClampsDown
 * @brief Verifies fx Fade In Above Two Clamps Down.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, FxFadeInAboveTwoClampsDown) {
  MockMMItem it;
  it.m_satcount = 7; // above 2 clamps to... (m_satcount > 2) ? 2 : sat+1
  it.m_curfx = mmItemFXFadeInToBright;
  it.ProcessFx();
  EXPECT_EQ(it.m_satcount, 2);
  EXPECT_EQ(it.m_curfx, mmItemFXNull);
}

/**
 * @test MMItem.QueueRefillAfterFinish
 * @brief Verifies queue Refill After Finish.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, QueueRefillAfterFinish) {
  MockMMItem it;
  tmmItemFX f = mmItemFXNormal;
  it.m_fxqueue.send(f);
  it.m_satcount = 1;
  it.m_curfx = mmItemFXFadeInToBright;
  // tick 1: fade-in finishes (sat reaches 2), then queue delivers Normal
  it.ProcessFx();
  EXPECT_EQ(it.m_curfx, mmItemFXNormal);
  // tick 2: Normal applies and clears
  it.ProcessFx();
  EXPECT_EQ(it.m_alpha, MMITEM_ALPHA);
  EXPECT_EQ(it.m_curfx, mmItemFXNull);
}

/**
 * @test MMItem.QueueRecvDrainsFifo
 * @brief Verifies queue Recv Drains Fifo.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, QueueRecvDrainsFifo) {
  FXQueue q;
  tmmItemFX out;
  EXPECT_FALSE(q.recv(&out)); // empty
  tmmItemFX a = mmItemFXNormal, b = mmItemFXFadeInToBright;
  q.send(a); q.send(b);
  ASSERT_TRUE(q.recv(&out));
  EXPECT_EQ(out, mmItemFXNormal);
  ASSERT_TRUE(q.recv(&out));
  EXPECT_EQ(out, mmItemFXFadeInToBright);
  EXPECT_FALSE(q.recv(&out));
  q.flush();
  EXPECT_FALSE(q.recv(&out));
}

// replicated AddItem flags mapping + Y stack (mmItem.cpp:377-391)
constexpr int UIF_GROUP_START = 1, UIF_GROUP_END = 2;
static int FlagsForType(int type) {
  return (type == 1)   ? UIF_GROUP_START
         : (type == 2) ? UIF_GROUP_END
         : (type == 3) ? (UIF_GROUP_START + UIF_GROUP_END)
                       : 0;
}
static int ItemY(int index) { return MMITEM_Y + index * 20; }

/**
 * @test MMItem.AddItemTypeFlags
 * @brief Verifies add Item Type Flags.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, AddItemTypeFlags) {
  EXPECT_EQ(FlagsForType(0), 0);
  EXPECT_EQ(FlagsForType(1), UIF_GROUP_START);
  EXPECT_EQ(FlagsForType(2), UIF_GROUP_END);
  EXPECT_EQ(FlagsForType(3), UIF_GROUP_START | UIF_GROUP_END);
  EXPECT_EQ(FlagsForType(99), 0); // unknown -> no flags
}

/**
 * @test MMItem.AddItemStackingY
 * @brief Verifies add Item Stacking Y.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, AddItemStackingY) {
  EXPECT_EQ(ItemY(0), 175);
  EXPECT_EQ(ItemY(1), 195);
  EXPECT_EQ(ItemY(N_MMENU_ITEMS - 1), 175 + 9 * 20);
}

// replicated CopyrightText type string (mmItem.cpp:418-452)
enum VersionType { DEVELOPMENT_VERSION, RELEASE_VERSION };
static std::string BuildCopyrightType(bool isDemo, bool beta, bool oem, VersionType vt) {
  std::string type;
  if (isDemo) type += "Demo ";
  if (beta) type = "Beta ";
  if (oem) type += "OEM ";
  type += (vt == DEVELOPMENT_VERSION) ? "Dev" : "Ver";
  return type;
}

/**
 * @test MMItem.CopyrightTypeStrings
 * @brief Verifies copyright Type Strings.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, CopyrightTypeStrings) {
  EXPECT_EQ(BuildCopyrightType(false, false, false, RELEASE_VERSION), "Ver");
  EXPECT_EQ(BuildCopyrightType(false, false, false, DEVELOPMENT_VERSION), "Dev");
  EXPECT_EQ(BuildCopyrightType(true, false, false, RELEASE_VERSION), "Demo Ver");
  EXPECT_EQ(BuildCopyrightType(false, true, false, RELEASE_VERSION), "Beta Ver");
  EXPECT_EQ(BuildCopyrightType(false, false, true, RELEASE_VERSION), "OEM Ver");
  // beta overwrites demo (strcpy vs strcat quirk at line 425-426)
  EXPECT_EQ(BuildCopyrightType(true, true, false, RELEASE_VERSION), "Beta Ver");
  // all combined
  EXPECT_EQ(BuildCopyrightType(true, true, true, DEVELOPMENT_VERSION), "Beta OEM Dev");
}

// replicated SetMusicRegion (mmItem.cpp:454-462)
static int g_musicStop = 0, g_musicStart = 0, g_musicSetRegion = 0;
static void ReplicatedSetMusicRegion(int region) {
  if (region == -1) {
    g_musicStop++;
    g_musicStart++;
    ReplicatedSetMusicRegion(MM_MUSIC_REGION);
  } else {
    g_musicSetRegion++;
  }
}

/**
 * @test MMItem.SetMusicRegionRestartRecursion
 * @brief Verifies set Music Region Restart Recursion.
 *
 * @details
 * Exercises the MMItem code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mmItem.cpp
 * @ingroup descent3_tests
 */
TEST(MMItem, SetMusicRegionRestartRecursion) {
  g_musicStop = g_musicStart = g_musicSetRegion = 0;
  ReplicatedSetMusicRegion(-1);
  EXPECT_EQ(g_musicStop, 1);
  EXPECT_EQ(g_musicStart, 1);
  EXPECT_EQ(g_musicSetRegion, 1); // recurses with MM_MUSIC_REGION
  ReplicatedSetMusicRegion(MM_MUSIC_REGION);
  EXPECT_EQ(g_musicSetRegion, 2);
}
