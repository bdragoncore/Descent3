/**
 * @file hudmessage_real_tests.cpp
 * @brief Tests for hudmessage.cpp (~1850 lines).
 *
 * @details
 * Covers the HUD message list (dedup, ring overflow, word wrap, color-code
 * carry), CorrectHudMessage truncation, chat destination parsing,
 * BreakupHUDInputMessage continuation logic, pause/expiry/blink rendering.
 *
 * This harness validates the behavior of `Descent3/hudmessage.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/hudmessage.cpp`
 * @par Harness
 * `hudmessage_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/hudmessage.cpp
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "hud.h"
#include "grdefs.h"
#include "game.h"
#include "player.h"
#include "multi.h"
#include "game2dll.h"
#include "gamefont.h"
#include "demofile.h"
#include "dedicated_server.h"
#include "marker.h"
#include "Mission.h"
#include "objinfo.h"
#include "levelgoal.h"
#include "weapon.h"
#include "osiris_dll.h"
#include "ddio.h"
#include "controls.h"
#include "hlsoundlib.h"
#include "mem.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
float Gametime = 0.0f;
int Game_window_x = 0, Game_window_y = 0, Game_window_w = 640, Game_window_h = 480;
int Max_window_w = 800, Max_window_h = 600;
bool Small_hud_flag = false;
float Hud_aspect_y = 1.0f;
int Game_fonts[8];
bool Dedicated_server = false;
uint32_t Demo_flags = 0;
int Marker_message = 0;
int Player_num = 0;
player Players[MAX_PLAYERS];
netgame_info Netgame;
netplayer NetPlayers[MAX_NET_PLAYERS];
dllinfo DLLInfo;

// ---------------------------------------------------------------------------
// Capture / control stubs
// ---------------------------------------------------------------------------
struct PutRec {
  int x, y;
  std::string text;
  ddgr_color color;
};
static std::vector<PutRec> g_puts;
static ddgr_color g_cur_color = 0;
static int g_char_w = 10; // pixels per character for width stub

void grtext_SetColor(ddgr_color col) { g_cur_color = col; }
void grtext_SetAlpha(uint8_t) {}
void grtext_SetFlags(int) {}
void grtext_SetFont(int) {}
void grtext_SetFontScale(float) {}
void grtext_Reset() {}
void grtext_Puts(int x, int y, const char *str) { g_puts.push_back({x, y, str, g_cur_color}); }
int grtext_GetTextLineWidth(const char *str) { return (int)strlen(str) * g_char_w; }
int grfont_GetHeight(int) { return 12; }

void t_dirty_rect::reset() { memset(r, 0, sizeof(r)); }
void t_dirty_rect::fill(ddgr_color) {}

hlsSystem Sound_system;
hlsSystem::hlsSystem() {}
int hlsSystem::Play2dSound(int, float, float, uint16_t) { return -1; }
int hlsSystem::Play2dSound(int, int, float, float, uint16_t) { return -1; }
void hlsSystem::StopSoundLooping(int) {}
void hlsSystem::KillSoundLib(bool) {}

int Game_mode = 0;
int FindArg(const char *, int) { return 0; }
void SuspendControls() {}
int Team_game = 0;
float Frametime = 0.0f;
tHUDItem *GetHUDItem(int) { return nullptr; }
void FreeHUDItem(int) {}
int grtext_GetTextHeight(const char *) { return 12; }
void grtext_SetParameters(int, int, int, int, int) {}
void grtext_Flush() {}
int RenderHUDGetTextLineWidth(const char *str) { return (int)strlen(str) * g_char_w; }

void DemoWriteHudMessage(uint32_t, bool, char *) {}
void PrintDedicatedMessage(const char *, ...) {}
void MultiSendMessageFromServer(int, char *, int) {}
void MultiSendMessageToServer(int, char *, int) {}
void MultiSendRequestTypeIcon(bool) {}
int ddio_KeyToAscii(int) { return 0; }
void ddio_KeyFlush() {}
float ddio_KeyDownTime(int) { return 0.0f; }
void ResumeControls() {}
void DropMarker(char *) {}
void CallGameDLL(int, dllinfo *) {}
const char *GetStringFromTable(int index) {
  static char buf[32];
  snprintf(buf, sizeof(buf), "str%d", index);
  return buf;
}
void ResetHUDLevelItems() {}
void DemoWritePersistantHUDMessage(uint32_t, int, int, float, int, int, char *) {}
void AddHUDItem(tHUDItem *) {}
void textaux_WordWrap(const char *, char *, int, int) {}
tMission Current_mission;

// Inventory support (player members)
object Objects[MAX_OBJECTS];
object_info Object_info[MAX_OBJECT_IDS];
levelgoals Level_goals;
weapon Weapons[MAX_WEAPONS];
texture GameTextures[MAX_TEXTURES];
bms_bitmap GameBitmaps[MAX_BITMAPS];
static std::map<int, object *> g_objmap;
object *ObjGet(int handle) {
  auto it = g_objmap.find(handle);
  return it == g_objmap.end() ? nullptr : it->second;
}
void levelgoals::Inform(char, int, int) {}
void CreateCountermeasureFromObject(object *, int) {}
void ObjGhostObject(int) {}
void ObjUnGhostObject(int) {}
void MultiSendGhostObject(object *, bool) {}
void MultiSendObject(object *, uint8_t, uint8_t) {}
void MultiSendInventoryRemoveItem(int, int, int) {}
void MultiSendClientInventoryUseItem(int, int) {}
int ObjCreate(uint8_t, uint16_t, int, vector *, const matrix *, int) { return -1; }
void SetObjectControlType(object *obj, int ct) { obj->control_type = ct; }
void SetObjectDeadFlag(object *, bool, bool) {}
bool AINotify(object *, uint8_t, void *) { return true; }
bool Osiris_CallEvent(object *, int, tOSIRISEventInfo *) { return true; }
bool Osiris_BindScriptsToObject(object *) { return true; }
void Osiris_DetachScriptsFromObject(object *) {}
void InitObjectScripts(object *, bool) {}

// Renderer no-ops for MsgListConsole::Draw
#include "renderer.h"
#include "3d.h"
void rend_SetZBufferState(int8_t) {}
void rend_SetTextureType(texture_type) {}
void rend_SetAlphaType(int8_t) {}
void rend_SetLighting(light_state) {}
void rend_SetFlatColor(ddgr_color) {}
void rend_SetFullScreen(bool) {}
void *GSDLWindow = nullptr;
bool Game_fullscreen = false;
void rend_SetAlphaValue(unsigned char) {}
void rend_DrawPolygon2D(int, g3Point **, int) {}
void rend_DrawLine(int, int, int, int) {}

// Not exported in any header
void RenderScrollingHUDMessages();
int BreakupHUDInputMessage(char *str);

// ---------------------------------------------------------------------------

/**
 * @brief GTest fixture for HudMessageTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class HudMessageTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_puts.clear();
    g_cur_color = 0;
    g_char_w = 10;
    Gametime = 10.0f;
    Game_window_w = 640;
    Max_window_w = 800;
    Small_hud_flag = false;
    Dedicated_server = false;
    Demo_flags = 0;
    Num_hud_messages = 0;
    memset(Players, 0, sizeof(Players));
    memset(NetPlayers, 0, sizeof(NetPlayers));
  }

  // Renders and returns the captured message texts
  std::vector<std::string> RenderTexts() {
    g_puts.clear();
    RenderScrollingHUDMessages();
    std::vector<std::string> out;
    for (auto &p : g_puts)
      out.push_back(p.text);
    return out;
  }

  void ConnectPlayer(int pn, const char *callsign) {
    strcpy(Players[pn].callsign, callsign);
    NetPlayers[pn].flags |= NPF_CONNECTED;
    NetPlayers[pn].sequence = NETSEQ_PLAYING;
  }
};

/**
 * @test HudMessageTest.DedupsConsecutiveIdenticalMessages
 * @brief Verifies dedups Consecutive Identical Messages.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DedupsConsecutiveIdenticalMessages) {
  EXPECT_TRUE(AddColoredHUDMessage(GR_RGB(255, 0, 0), "hello"));
  EXPECT_FALSE(AddColoredHUDMessage(GR_RGB(255, 0, 0), "hello")); // same as last
  EXPECT_TRUE(AddColoredHUDMessage(GR_RGB(255, 0, 0), "world"));
  EXPECT_EQ(Num_hud_messages, 2);
}

/**
 * @test HudMessageTest.RingOverflowKeepsNewestThree
 * @brief Verifies ring Overflow Keeps Newest Three.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, RingOverflowKeepsNewestThree) {
  AddHUDMessage("m1");
  AddHUDMessage("m2");
  AddHUDMessage("m3");
  AddHUDMessage("m4"); // punts m1
  EXPECT_EQ(Num_hud_messages, 3);

  auto texts = RenderTexts();
  ASSERT_EQ(texts.size(), 3u);
  EXPECT_EQ(texts[0], "m2");
  EXPECT_EQ(texts[1], "m3");
  EXPECT_EQ(texts[2], "m4");
}

/**
 * @test HudMessageTest.DefaultColorIsGreen
 * @brief Verifies default Color Is Green.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DefaultColorIsGreen) {
  AddHUDMessage("plain"); // no color specified
  auto texts = RenderTexts();
  ASSERT_EQ(texts.size(), 1u);
  EXPECT_EQ(g_puts[0].color, GR_RGB(0, 255, 0));
}

/**
 * @test HudMessageTest.WordWrapSplitsAtLimit
 * @brief Verifies word Wrap Splits At Limit.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, WordWrapSplitsAtLimit) {
  Game_window_w = 180; // wrap budget = 126px at 10px/char
  AddHUDMessage("aaaa bbbb cccc");

  auto texts = RenderTexts();
  ASSERT_EQ(texts.size(), 2u);
  EXPECT_EQ(texts[0], "aaaa bbbb");
  EXPECT_EQ(texts[1], "cccc");
}

/**
 * @test HudMessageTest.WrapCarriesColorCodeToContinuationLine
 * @brief Verifies wrap Carries Color Code To Continuation Line.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, WrapCarriesColorCodeToContinuationLine) {
  Game_window_w = 180;
  std::string msg = "\x01\x11\x22\x33 aaaa bbbb cccc dddd eeee";
  AddColoredHUDMessage(GR_RGB(255, 255, 255), "%s", msg.c_str());

  auto texts = RenderTexts();
  ASSERT_GE(texts.size(), 3u);
  // Every wrapped line must start with the 4-byte color code
  for (auto &t : texts)
    EXPECT_EQ(t.substr(0, 4), std::string("\x01\x11\x22\x33", 4));
}

/**
 * @test HudMessageTest.OversizeSingleTokenTruncatedToLimit
 * @brief Verifies oversize Single Token Truncated To Limit.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, OversizeSingleTokenTruncatedToLimit) {
  std::string big(250, 'x');
  AddHUDMessage("%s", big.c_str());
  EXPECT_EQ(Num_hud_messages, 1);

  auto texts = RenderTexts();
  ASSERT_EQ(texts.size(), 1u);
  EXPECT_EQ(texts[0].size(), HUD_MESSAGE_LENGTH - 1); // 199 chars
}

/**
 * @test HudMessageTest.ExpiryPuntsOldestAfterFiveSeconds
 * @brief Verifies expiry Punts Oldest After Five Seconds.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, ExpiryPuntsOldestAfterFiveSeconds) {
  AddHUDMessage("old");
  Gametime = 11.0f;
  AddHUDMessage("new");

  Gametime = 16.5f; // > 5s past the timer
  auto texts1 = RenderTexts();
  EXPECT_EQ(texts1.size(), 2u);    // draws happen BEFORE the expiry check...
  EXPECT_EQ(Num_hud_messages, 1);  // ...and each render punts one message

  auto texts2 = RenderTexts();
  ASSERT_EQ(texts2.size(), 1u);
  EXPECT_EQ(texts2[0], "new");
}

/**
 * @test HudMessageTest.PauseBlocksRenderAndExpiry
 * @brief Verifies pause Blocks Render And Expiry.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, PauseBlocksRenderAndExpiry) {
  AddHUDMessage("held");
  HUDPauseMessages();

  Gametime = 20.0f;
  g_puts.clear();
  RenderScrollingHUDMessages();
  EXPECT_TRUE(g_puts.empty());   // paused -> nothing drawn
  EXPECT_EQ(Num_hud_messages, 1); // and nothing expired

  HUDUnpauseMessages();
  auto texts = RenderTexts();
  ASSERT_EQ(texts.size(), 1u);
  EXPECT_EQ(texts[0], "held");
}

/**
 * @test HudMessageTest.BlinkingMessagesSkipEveryOtherHalfSecond
 * @brief Verifies blinking Messages Skip Every Other Half Second.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, BlinkingMessagesSkipEveryOtherHalfSecond) {
  Gametime = 40.0f; // add inside the display window
  EXPECT_TRUE(AddBlinkingHUDMessage("blink"));

  Gametime = 40.1f; // frac 0.1 -> draw phase
  auto on = RenderTexts();
  ASSERT_EQ(on.size(), 1u);
  EXPECT_EQ(on[0], "blink");

  Gametime = 40.6f; // frac 0.6 -> skip phase
  auto off = RenderTexts();
  EXPECT_TRUE(off.empty());

  Gametime = 41.1f; // frac 0.1 again -> draw phase (still within 5s window)
  EXPECT_EQ(RenderTexts().size(), 1u);
}

// --- GetMessageDestination -------------------------------------------------

/**
 * @test HudMessageTest.DestinationNoColonMeansEveryone
 * @brief Verifies destination No Colon Means Everyone.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DestinationNoColonMeansEveryone) {
  int dest = 42;
  const char *rest = GetMessageDestination("hi all", &dest);
  EXPECT_EQ(dest, MULTI_SEND_MESSAGE_ALL);
  EXPECT_STREQ(rest, "hi all");
}

/**
 * @test HudMessageTest.DestinationNumericPlayerNumber
 * @brief Verifies destination Numeric Player Number.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DestinationNumericPlayerNumber) {
  int dest = -99;
  const char *rest = GetMessageDestination("2:hello", &dest);
  EXPECT_EQ(dest, 2);
  EXPECT_STREQ(rest, "hello");
}

/**
 * @test HudMessageTest.DestinationOutOfRangeNumberFallsBackToAll
 * @brief Verifies destination Out Of Range Number Falls Back To All.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DestinationOutOfRangeNumberFallsBackToAll) {
  int dest = -99;
  const char *rest = GetMessageDestination("99:hello", &dest); // MAX_PLAYERS=32
  EXPECT_EQ(dest, MULTI_SEND_MESSAGE_ALL);
  EXPECT_STREQ(rest, "hello");
}

/**
 * @test HudMessageTest.DestinationExactNameMatch
 * @brief Verifies destination Exact Name Match.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DestinationExactNameMatch) {
  ConnectPlayer(1, "Bob");
  int dest = -99;
  const char *rest = GetMessageDestination("Bob:hi", &dest);
  EXPECT_EQ(dest, 1);
  EXPECT_STREQ(rest, "hi");
}

/**
 * @test HudMessageTest.DestinationPartialNameMatch
 * @brief Verifies destination Partial Name Match.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DestinationPartialNameMatch) {
  ConnectPlayer(1, "Bob");
  int dest = -99;
  const char *rest = GetMessageDestination("Bo:hi", &dest); // prefix match
  EXPECT_EQ(dest, 1);
  EXPECT_STREQ(rest, "hi");
}

/**
 * @test HudMessageTest.DestinationIgnoresDisconnectedPlayers
 * @brief Verifies destination Ignores Disconnected Players.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DestinationIgnoresDisconnectedPlayers) {
  strcpy(Players[1].callsign, "Bob");
  NetPlayers[1].flags = 0; // not connected
  int dest = -99;
  GetMessageDestination("Bob:hi", &dest);
  EXPECT_EQ(dest, MULTI_SEND_MESSAGE_ALL);
}

/**
 * @test HudMessageTest.DestinationTeamKeywordUsesLocalTeam
 * @brief Verifies destination Team Keyword Uses Local Team.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, DestinationTeamKeywordUsesLocalTeam) {
  Players[Player_num].team = 1; // blue
  int dest = -99;
  const char *rest = GetMessageDestination("team:go", &dest);
  EXPECT_EQ(dest, MULTI_SEND_MESSAGE_BLUE_TEAM);
  EXPECT_STREQ(rest, "go");
}

// --- BreakupHUDInputMessage ------------------------------------------------

/**
 * @test HudMessageTest.BreakupWithoutSpaceReturnsZero
 * @brief Verifies breakup Without Space Returns Zero.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, BreakupWithoutSpaceReturnsZero) {
  strcpy(HudInputMessage, "oneword");
  char out[MAX_HUD_INPUT_LEN];
  EXPECT_EQ(BreakupHUDInputMessage(out), 0);
}

/**
 * @test HudMessageTest.BreakupSplitsAtLastSpace
 * @brief Verifies breakup Splits At Last Space.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, BreakupSplitsAtLastSpace) {
  strcpy(HudInputMessage, "one two three");
  char out[MAX_HUD_INPUT_LEN];
  int len = BreakupHUDInputMessage(out);
  EXPECT_EQ(len, 5);
  EXPECT_STREQ(out, "three");
  EXPECT_STREQ(HudInputMessage, "one two"); // consumed part truncated
}

/**
 * @test HudMessageTest.BreakupPreservesDestinationPrefix
 * @brief Verifies breakup Preserves Destination Prefix.
 *
 * @details
 * Exercises the HudMessageTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hudmessage.cpp
 * @ingroup descent3_tests
 */
TEST_F(HudMessageTest, BreakupPreservesDestinationPrefix) {
  strcpy(HudInputMessage, "7:a b c"); // addressed to player 7
  char out[MAX_HUD_INPUT_LEN];
  int len = BreakupHUDInputMessage(out);
  EXPECT_EQ(len, 3);
  EXPECT_STREQ(out, "7:c");
  EXPECT_STREQ(HudInputMessage, "7:a b");
}
