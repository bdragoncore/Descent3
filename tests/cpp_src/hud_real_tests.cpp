/**
 * @file hud_real_tests.cpp
 * @brief Tests for hud.cpp 2233 lines — the HUD item subsystem (Add/Free/.
 *
 * @details
 * Reset/FindCustomtext2). The REAL hud.cpp is compiled into this
 * test; message console, cockpit, music, and demo hooks are stubbed.
 * HUD_array has file-local linkage, reached via GetHUDItem().
 *
 * This harness validates the behavior of `Descent3/hud.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/hud.cpp`
 * @par Harness
 * `hud_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/hud.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>

#include "hud.h"
#include "mem.h"
#include "game2dll.h"
#include "ctlconfig.h"
#include "object.h"
#include "player.h"
#include "ship.h"
#include "polymodel.h"
#include "pilot.h"
#include "SmallViews.h"

// not exposed in hud.h
tHUDItem *GetHUDItem(int id);
void ResetHUDLevelItems();
#define STAT_SCORE 0x0800 // hud.cpp maps STAT_SCORE onto STAT_TIMER

// ---- stubs for symbols hud.cpp references but we do not exercise ----
bool Dedicated_server = false;
bool HUD_disabled = false;
int Clear_screen = 0;
float Frametime = 0;
int Max_window_w = 640, Max_window_h = 480;
int Game_window_w = 640, Game_window_h = 480, Game_window_x = 0, Game_window_y = 0;
int Player_num = 0;
bool Cinematic_inuse = false;
bool Guided_missile_smallview = false;
uint32_t Demo_flags = 0, Demo_paused = 0, Demo_make_movie = 0;
dllinfo DLLInfo;
int Game_fonts[8] = {};
t_cfg_element Cfg_key_elements[1] = {};
t_cfg_element Cfg_joy_elements[1] = {};
char HUD_msg_con_placeholder; // consoles are static members inside hud.cpp

void ResetGameMessages() {}
void ResetHUDMessages() {}
void CloseHUDMessageConsole() {}
void CloseGameMessageConsole() {}
int GetScreenMode() { return 2; } // SM_GAME
void QuickCloseCockpit() {}
void CloseCockpit() {}
void QuickOpenCockpit() {}
void OpenCockpit() {}
bool IsValidCockpit() { return false; }
void RenderCockpit() {}
float GetFPS() { return 0; }
void DemoWriteHudMessage(uint32_t, bool, char *) {}
void CallGameDLL(int, dllinfo *) {}
const char *GetStringFromTable(int) { return ""; }

// renderer touchpoints
void rend_SetFlatColor(unsigned int) {}
void rend_SetOverlayType(unsigned char) {}
void rend_SetZBufferState(signed char) {}
void rend_DrawLine(int, int, int, int) {}

// font layer is declared inside extern "C" in grtext.h
extern "C" {
void grtext_SetFont(int) {}
void grtext_SetFontScale(float) {}
int grfont_GetHeight(int) { return 8; }
void grtext_Flush() {}
void grtext_Reset() {}
} // extern "C"

// misc globals
tGameToggles Game_toggles;
MsgListConsole Game_msg_con;
MsgListConsole HUD_msg_con;
gameController *Controller = nullptr;
int bm_AllocLoadFileBitmap(const char *, int, int) { return -1; }
void bm_FreeBitmap(int) {}
const char *cfg_binding_text(ct_type, uint8_t, uint8_t) { return ""; }
int CtlFindBinding(int, bool) { return -1; }
void InitGameScreen(int, int) {}
bool AddHUDMessage(const char *, ...) { return true; }
void RenderHUDText(unsigned int, uint8_t, int, int, int, const char *, ...) {}
void RenderHUDTextFlags(int, ddgr_color, uint8_t, int, int, int, const char *, ...) {}

// Inventory ctor/dtor live in Inventory.cpp; not exercised here
Inventory::Inventory() {}
Inventory::~Inventory() {}

// pilot ctor/dtor live in pilot.cpp
pilot::pilot() {}
pilot::~pilot() {}

// object tables (only symbol presence needed)
object Objects[MAX_OBJECTS];
player Players[MAX_PLAYERS];
ship Ships[MAX_SHIPS];
poly_model Poly_models[64];

// hud render callbacks referenced by InitDefaultHUDItems & friends
void RenderHUDScore(tHUDItem *) {}
void RenderHUDTimer(tHUDItem *) {}
void HudDisplayRouter(tHUDItem *) {}
void RenderHUDMessages() {}
void RenderHUDMsgDirtyRects() {}
void RenderHUDQuad(int, int, int, int, float, float, float, float, int, uint8_t, int) {}
void RenderHUDTextFlagsNoFormat(int, ddgr_color, uint8_t, int, int, int, const char *) {}

bool WBIsBatteryReady(object *, otype_wb_info *, int) { return false; }

// dirty rect methods live in smallviews/hud display code
void t_dirty_rect::reset() {
  for (auto &rc : r)
    rc.l = rc.t = rc.r = rc.b = -1;
}
void t_dirty_rect::fill(ddgr_color) {}

// message console subclass methods used by hud.cpp
void MsgListConsole::Draw() {}
void MsgListConsole::DoInput() {}

// pilot hooks
pilot Current_pilot;
int PltWriteFile(pilot *, bool) { return 1; }
void pilot::get_hud_data(uint8_t *mode, uint16_t *stat, uint16_t *gr_stat, int *unused1, int *unused2) {
  if (mode)
    *mode = 0;
  if (stat)
    *stat = 0;
  if (gr_stat)
    *gr_stat = 0;
}
void pilot::set_hud_data(uint8_t *, uint16_t *, uint16_t *, int *, int *) {}

static void ClearHudItems() {
  InitHUD(); // zeroes stat/flags/id across all slots
}

/**
 * @test HudItems.AddItemMapsTypeToStatAndCopiesFields
 * @brief Verifies add Item Maps Type To Stat And Copies Fields.
 *
 * @details
 * Exercises the HudItems code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hud.cpp
 * @ingroup descent3_tests
 */
TEST(HudItems, AddItemMapsTypeToStatAndCopiesFields) {
  ClearHudItems();

  tHUDItem item;
  memset(&item, 0, sizeof(item));
  item.type = HUD_ITEM_SHIELD;
  item.x = 111;
  item.y = 222;
  item.color = 0x123456;

  AddHUDItem(&item);

  tHUDItem *slot0 = GetHUDItem(0);
  EXPECT_NE(slot0->stat & STAT_SHIELDS, 0);
  EXPECT_EQ(slot0->x, 111);
  EXPECT_EQ(slot0->y, 222);
  EXPECT_EQ(slot0->color, 0x123456);
  EXPECT_EQ(slot0->type, HUD_ITEM_SHIELD);
  EXPECT_EQ(slot0->id, 0); // InitHUDItem stamps the slot index

  // second add takes the next free slot
  tHUDItem item2;
  memset(&item2, 0, sizeof(item2));
  item2.type = HUD_ITEM_ENERGY;
  AddHUDItem(&item2);
  EXPECT_NE(GetHUDItem(1)->stat & STAT_ENERGY, 0);
}

/**
 * @test HudItems.StatMappingAcrossTypes
 * @brief Verifies stat Mapping Across Types.
 *
 * @details
 * Exercises the HudItems code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hud.cpp
 * @ingroup descent3_tests
 */
TEST(HudItems, StatMappingAcrossTypes) {
  ClearHudItems();
  struct TypeStat {
    int type;
    uint16_t stat;
  };
  const TypeStat cases[] = {
      {HUD_ITEM_PRIMARY, STAT_PRIMARYLOAD},
      {HUD_ITEM_SECONDARY, STAT_SECONDARYLOAD},
      {HUD_ITEM_SHIELD, STAT_SHIELDS},
      {HUD_ITEM_ENERGY, STAT_ENERGY},
      {HUD_ITEM_AFTERBURNER, STAT_AFTERBURN},
      {HUD_ITEM_INVENTORY, STAT_INVENTORY},
      {HUD_ITEM_SHIPSTATUS, STAT_SHIP},
      {HUD_ITEM_CNTRMEASURE, STAT_CNTRMEASURE},
      {HUD_ITEM_CUSTOMIMAGE, STAT_CUSTOM},
      {HUD_ITEM_WARNINGS, STAT_WARNING},
      {HUD_ITEM_GOALS, STAT_GOALS},
      {HUD_ITEM_SCORE, STAT_SCORE},
  };
  const int num_cases = (int)(sizeof(cases) / sizeof(cases[0]));
  for (int i = 0; i < num_cases; i++) {
    tHUDItem it;
    memset(&it, 0, sizeof(it));
    it.type = cases[i].type;
    AddHUDItem(&it);
    ASSERT_EQ(GetHUDItem(i)->stat, cases[i].stat) << "type " << cases[i].type;
  }
}

/**
 * @test HudItems.CustomTextLifecycleAndQuirks
 * @brief Verifies custom Text Lifecycle And Quirks.
 *
 * @details
 * Exercises the HudItems code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hud.cpp
 * @ingroup descent3_tests
 */
TEST(HudItems, CustomTextLifecycleAndQuirks) {
  ClearHudItems();

  // CUSTOMTEXT duplicates the string
  tHUDItem t1;
  memset(&t1, 0, sizeof(t1));
  char msg[] = "hello";
  t1.type = HUD_ITEM_CUSTOMTEXT;
  t1.data.text = msg;
  AddHUDItem(&t1);
  tHUDItem *slot0 = GetHUDItem(0);
  ASSERT_NE(slot0->data.text, nullptr);
  EXPECT_NE(slot0->data.text, msg); // copied, not aliased
  EXPECT_STREQ(slot0->data.text, "hello");

  // CUSTOMTEXT2 allocates buffer_size and zero-terminates
  tHUDItem t2;
  memset(&t2, 0, sizeof(t2));
  t2.type = HUD_ITEM_CUSTOMTEXT2;
  t2.buffer_size = 8;
  AddHUDItem(&t2);
  tHUDItem *slot1 = GetHUDItem(1);
  ASSERT_NE(slot1->data.text, nullptr);
  EXPECT_EQ(slot1->buffer_size, 8);
  EXPECT_EQ(slot1->data.text[0], 0);

  // find returns the customtext2 index (customtext above doesn't count)
  EXPECT_EQ(FindCustomtext2HUDItem(), 1);

  // update copies text into the buffer
  UpdateCustomtext2HUDItem((char *)"abcd");
  EXPECT_STREQ(slot1->data.text, "abcd");

  // quirk: text at least buffer_size long overflows strncpy without a
  // NUL and the trailing statement in UpdateCustomtext2HUDItem does
  // nothing, so the tail after the prefix is stale garbage
  UpdateCustomtext2HUDItem((char *)"123456789ABC");
  EXPECT_EQ(strncmp(slot1->data.text, "12345678", 8), 0);

  FreeHUDItem(1);
  EXPECT_EQ(GetHUDItem(1)->stat, 0);
  EXPECT_EQ(GetHUDItem(1)->data.text, nullptr);
  EXPECT_EQ(FindCustomtext2HUDItem(), -1);

  FreeHUDItem(0); // frees customtext copy too
  EXPECT_EQ(GetHUDItem(0)->data.text, nullptr);
}

/**
 * @test HudItems.ResetHonorsLevelAndPersistentFlags
 * @brief Verifies reset Honors Level And Persistent Flags.
 *
 * @details
 * Exercises the HudItems code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/hud.cpp
 * @ingroup descent3_tests
 */
TEST(HudItems, ResetHonorsLevelAndPersistentFlags) {
  ClearHudItems();
  tHUDItem base;
  memset(&base, 0, sizeof(base));

  base.type = HUD_ITEM_SCORE;
  AddHUDItem(&base); // plain: cleared by ResetHUD

  base.flags = HUD_FLAG_PERSISTANT;
  AddHUDItem(&base); // survives ResetHUD

  base.flags = HUD_FLAG_LEVEL;
  AddHUDItem(&base); // survives ResetHUD, cleared by level reset

  ResetHUD();
  EXPECT_EQ(GetHUDItem(0)->stat, 0); // freed
  EXPECT_NE(GetHUDItem(1)->stat, 0); // persistant kept
  EXPECT_NE(GetHUDItem(2)->stat, 0); // level kept too

  ResetHUDLevelItems();
  EXPECT_EQ(GetHUDItem(2)->stat, 0); // level item gone
  EXPECT_NE(GetHUDItem(1)->stat, 0); // persistant still there
}
