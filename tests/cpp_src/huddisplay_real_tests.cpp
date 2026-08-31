/**
 * @file huddisplay_real_tests.cpp
 * @brief Tests for huddisplay.cpp 1037 lines — the per-item HUD renderers.
 *
 * @details
 * The REAL source is compiled in; text/bitmap output goes through
 * recording stubs so formatting, thresholds, gauge math, and the
 * router dispatch are all observable without a renderer.
 *
 * This harness validates the behavior of `Descent3/huddisplay.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/huddisplay.cpp`
 * @par Harness
 * `huddisplay_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/huddisplay.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <string>

#include "hud.h"
#include "player.h"
#include "object.h"
#include "ship.h"
#include "weapon.h"
#include "Inventory.h"
#include "gamefont.h"
#include "game.h"

// public in huddisplay.cpp but missing from hud.h
void HudDisplayRouter(tHUDItem *item);
void RenderHUDShieldValue(tHUDItem *item);
void RenderHUDEnergyValue(tHUDItem *item);
void RenderHUDPrimary(tHUDItem *item);

// ---- recorded output ----
// The real RenderHUDText/RenderHUDQuad funnel into grtext_Puts and
// rend_DrawScaledBitmap; those are the capture points.
struct PutCall {
  int x, y;
  std::string str;
  ddgr_color col;
  uint8_t alpha;
  bool saturate;
};
struct DrawCall {
  int x1, y1, x2, y2, bm;
};
static std::vector<PutCall> g_puts;
static std::vector<DrawCall> g_draws;
static ddgr_color g_cur_col = 0;
static uint8_t g_cur_alpha = 255;
static int g_cur_flags = 0;

void grtext_SetAlpha(uint8_t a) { g_cur_alpha = a; }
void grtext_SetFlags(int f) { g_cur_flags = f; }
uint8_t grtext_GetAlpha() { return g_cur_alpha; }
void grtext_SetColor(ddgr_color c) { g_cur_col = c; }
ddgr_color grtext_GetColor() { return g_cur_col; }
void grtext_SetFont(int) {}
int grtext_GetFont() { return 0; } // not the HUD font -> aspect 1.0 paths
int grtext_GetTextHeight(const char *) { return 20; }
int grtext_GetTextLineWidth(const char *) { return 40; }
int grtext_GetTextHeightTemplate(tFontTemplate *, const char *) { return 20; }
int grtext_GetTextLineWidthTemplate(const tFontTemplate *, const char *) { return 40; }
void grtext_CenteredPrintf(int, int, const char *, ...) {}
void grtext_Puts(int x, int y, const char *str) {
  g_puts.push_back({x, y, str, g_cur_col, g_cur_alpha, (g_cur_flags & GRTEXTFLAG_SATURATE) != 0});
}
extern "C" {
int grfont_GetHeight(int) { return 20; }
}

// ---- world state ----
player Players[MAX_PLAYERS];
int Player_num = 0;
object Objects[MAX_OBJECTS];
ship Ships[MAX_SHIPS];
const int Static_weapon_names_msg[8] = {10, 11, 12, 13, 14, 15, 16, 17};
sHUDResources HUD_resources;
bool Small_hud_flag = false;
float Hud_aspect_x = 1.0f, Hud_aspect_y = 1.0f;
tFontTemplate Hud_font_template;
int Game_fonts[8] = {};
bms_bitmap GameBitmaps[MAX_BITMAPS];
int Game_mode = 0;
float Gametime = 0.0f, Frametime = 0.0f;
int Game_window_w = 640, Max_window_w = 640;
int Score_added = 0;
float Score_added_timer = 0.0f;
netgame_info Netgame;

const char *GetStringFromTable(int index) {
  switch (index) {
  case 120:
    return "inv";
  case 121:
    return "clk";
  case 313:
    return "LOCK";
  case 550:
    return "Shields";
  case 551:
    return "Energy";
  case 552:
    return "Afterburner";
  case 10:
    return "Laser";
  case 11:
    return "Vulcan";
  case 12:
    return "Mass";
  case 13:
    return "Fusion";
  default:
    return "";
  }
}

// fake bitmap metrics
static int g_bm_w_val = 64, g_bm_h_val = 32;
int bm_w(int, int) { return g_bm_w_val; }
int bm_h(int, int) { return g_bm_h_val; }
int GetTextureBitmap(int, int, bool) { return -1; }
weapon *GetWeaponFromIndex(int, int) { return nullptr; }

void rend_SetZBufferState(signed char) {}
void rend_SetTextureType(texture_type) {}
void rend_SetAlphaValue(unsigned char) {}
void rend_SetLighting(light_state) {}
void rend_SetWrapType(wrap_type) {}
void rend_SetAlphaType(signed char) {}
void rend_FillRect(unsigned int, int, int, int, int) {}
void rend_DrawScaledBitmap(int x1, int y1, int x2, int y2, int bm, float u0, float v0, float u1, float v1, int color,
                           const float *alphas) {
  g_draws.push_back({x1, y1, x2, y2, bm});
}

static effect_info_s g_status_effect_info;

// controlled inventory for the player under test
static tInvenList g_inven[4];
static int g_inven_count = 0;
static int g_inven_cur_sel = -1;
int Inventory::GetInventoryItemList(tInvenList *list, int max_amount, int *cur_sel) {
  int n = (g_inven_count < max_amount) ? g_inven_count : max_amount;
  memcpy(list, g_inven, n * sizeof(tInvenList));
  *cur_sel = g_inven_cur_sel;
  return n;
}
float Osiris_TimerTimeRemaining(int) { return -1.0f; }

// pulled in via player.h; not exercised here
Inventory::Inventory() {}
Inventory::~Inventory() {}

static void ResetRecorders() {
  g_puts.clear();
  g_draws.clear();
  g_cur_col = 0;
  g_cur_alpha = 255;
  g_cur_flags = 0;
}

/**
 * @test HudDisplay.ShieldGaugeMathAndLowWarningDot
 * @brief Verifies shield Gauge Math And Low Warning Dot.
 *
 * @details
 * Exercises the HudDisplay code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/huddisplay.cpp
 * @ingroup descent3_tests
 */
TEST(HudDisplay, ShieldGaugeMathAndLowWarningDot) {
  ResetRecorders();
  Player_num = 0;
  Players[0].objnum = 5;
  Objects[5].shields = INITIAL_SHIELDS; // full
  HUD_resources.dot_bmp = 90;
  HUD_resources.shield_bmp[0] = 51;
  HUD_resources.shield_bmp[1] = 52;
  HUD_resources.shield_bmp[2] = 53;
  HUD_resources.shield_bmp[3] = 54;
  HUD_resources.shield_bmp[4] = 55;

  tHUDItem item;
  memset(&item, 0, sizeof(item));
  item.type = HUD_ITEM_SHIELD;
  item.stat = STAT_GRAPHICAL;
  item.alpha = 200;
  item.saturation_count = 2;

  HudDisplayRouter(&item);

  // numeric readout uses %03d; sat_count=2 means the string is pushed
  // three times (base pass + two saturates). Gauge frame for full
  // shields: ceil((1-1-0.1)*5) < 0 -> clamped to frame 0, also drawn
  // three times. No warning dot at alpha_mod 1.0.
  ASSERT_EQ(g_puts.size(), 3u);
  EXPECT_EQ(g_puts[0].str, "100");
  ASSERT_EQ(g_draws.size(), 3u);
  EXPECT_EQ(g_draws[0].bm, HUD_resources.shield_bmp[0]);
  EXPECT_TRUE(std::none_of(g_draws.begin(), g_draws.end(),
                           [&](const DrawCall &d) { return d.bm == HUD_resources.dot_bmp; }));

  // exactly at the 20% line: dot drawn (sat=1 -> twice), gauge frame
  // ceil((1-0.2-0.1)*5)=ceil(3.5)=4
  ResetRecorders();
  Objects[5].shields = 20;
  HudDisplayRouter(&item);
  int dots = 0, gauge4 = 0;
  for (auto &d : g_draws) {
    if (d.bm == HUD_resources.dot_bmp)
      dots++;
    if (d.bm == HUD_resources.shield_bmp[4])
      gauge4++;
  }
  EXPECT_EQ(dots, 2);
  EXPECT_EQ(gauge4, 3);

  // just above the 20% line: dot suppressed
  ResetRecorders();
  Objects[5].shields = 21;
  HudDisplayRouter(&item);
  dots = 0;
  for (auto &d : g_draws)
    if (d.bm == HUD_resources.dot_bmp)
      dots++;
  EXPECT_EQ(dots, 0);
}

/**
 * @test HudDisplay.ShieldTextVariantBoostsSaturationWhenCritical
 * @brief Verifies shield Text Variant Boosts Saturation When Critical.
 *
 * @details
 * Exercises the HudDisplay code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/huddisplay.cpp
 * @ingroup descent3_tests
 */
TEST(HudDisplay, ShieldTextVariantBoostsSaturationWhenCritical) {
  ResetRecorders();
  Player_num = 0;
  Players[0].objnum = 5;

  tHUDItem item;
  memset(&item, 0, sizeof(item));
  item.type = HUD_ITEM_SHIELD;
  item.saturation_count = 3;
  item.tx = 7;
  item.ty = 9;

  Objects[5].shields = 15; // critical
  HudDisplayRouter(&item);
  // critical -> sat 4 -> 5 puts of "Shields: 015"
  ASSERT_EQ(g_puts.size(), 5u);
  EXPECT_NE(g_puts[0].str.find("Shields"), std::string::npos);
  EXPECT_NE(g_puts[0].str.find("015"), std::string::npos);
  EXPECT_TRUE(g_puts[0].saturate);
  EXPECT_EQ(g_puts[0].x, 7);
  EXPECT_EQ(g_puts[0].y, 9);

  // healthy again: base saturation 3 -> 4 puts
  ResetRecorders();
  Objects[5].shields = 90;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.size(), 4u); // sat 3 + base pass
}

/**
 * @test HudDisplay.EnergyGaugeSplitsSpentVsRemainingQuads
 * @brief Verifies energy Gauge Splits Spent Vs Remaining Quads.
 *
 * @details
 * Exercises the HudDisplay code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/huddisplay.cpp
 * @ingroup descent3_tests
 */
TEST(HudDisplay, EnergyGaugeSplitsSpentVsRemainingQuads) {
  ResetRecorders();
  Player_num = 0;
  Players[0].energy = INITIAL_ENERGY / 2; // exactly half

  tHUDItem item;
  memset(&item, 0, sizeof(item));
  item.type = HUD_ITEM_ENERGY;
  item.stat = STAT_GRAPHICAL;
  item.alpha = 100;
  item.xa = 30;
  item.ya = 40;
  item.xb = 100;
  item.yb = 50;

  HUD_resources.energy_bmp = 77;
  g_bm_w_val = 64; // scale .5 -> img_w 32
  g_bm_h_val = 32; // scale .5 -> img_h 16

  HudDisplayRouter(&item);

  // half energy: img_energy_h = floor(0.5*16+0.5) = 8, spent height
  // img_h - img_energy_h = 8. Four quads, each drawn once (sat 0).
  int rem_h = 8;
  ASSERT_EQ(g_draws.size(), 4u);
  ASSERT_EQ(g_puts.size(), 1u);
  EXPECT_EQ(g_puts[0].str, "050");

  // left spent strip at (x+xa, y+ya), faded alpha via alpha/4 quad call
  EXPECT_EQ(g_draws[0].bm, HUD_resources.energy_bmp);
  EXPECT_EQ(g_draws[0].x2 - g_draws[0].x1, 32);
  EXPECT_EQ(g_draws[0].y2 - g_draws[0].y1, 16 - rem_h);
  // right spent strip mirrors at xb - width
  EXPECT_EQ(g_draws[1].x2 - g_draws[1].x1, 32);
  EXPECT_EQ(g_draws[1].y2 - g_draws[1].y1, 16 - rem_h);
  // remaining strips sit below the spent ones and are img_energy_h tall
  EXPECT_EQ(g_draws[2].y1, g_draws[0].y2);      // ya + spent_h
  EXPECT_EQ(g_draws[2].y2 - g_draws[2].y1, rem_h);
  EXPECT_EQ(g_draws[3].y2 - g_draws[3].y1, rem_h);
}

/**
 * @test HudDisplay.AfterburnerPercentTruncatesAndBoostsSaturationAtLow
 * @brief Verifies afterburner Percent Truncates And Boosts Saturation At Low.
 *
 * @details
 * Exercises the HudDisplay code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/huddisplay.cpp
 * @ingroup descent3_tests
 */
TEST(HudDisplay, AfterburnerPercentTruncatesAndBoostsSaturationAtLow) {
  ResetRecorders();
  Player_num = 0;

  tHUDItem item;
  memset(&item, 0, sizeof(item));
  item.type = HUD_ITEM_AFTERBURNER;
  item.stat = STAT_GRAPHICAL;
  item.alpha = 100;
  item.saturation_count = 1;

  HUD_resources.afterburn_bmp = 44;
  g_bm_w_val = 128; // scale .5 -> img_w 64
  g_bm_h_val = 32;  // scale .5 -> img_h 16

  Players[0].afterburn_time_left = AFTERBURN_TIME; // 100%
  HudDisplayRouter(&item);
  // QUIRK: snprintf builds "%d%%" -> "100%", then RenderHUDText feeds
  // that string through vsnprintf again as a format string, so the
  // trailing lone '%' is dropped: displayed text is just "100".
  ASSERT_EQ(g_puts.size(), 2u);
  EXPECT_EQ(g_puts[0].str, "100");
  // two gauge quads; at full burn the spent quad is degenerate (w=0)
  // but still issued -> 1 + sat+1 = 3 draws
  ASSERT_EQ(g_draws.size(), 3u);
  for (auto &d : g_draws)
    EXPECT_EQ(d.bm, HUD_resources.afterburn_bmp);

  // ~28.9% fuel -> "28" (int cast truncates; rounding would say 29)
  ResetRecorders();
  Players[0].afterburn_time_left = AFTERBURN_TIME * 0.289f;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.size(), 2u);
  EXPECT_EQ(g_puts[0].str, "28");
}

/**
 * @test HudDisplay.PrimaryAmmoFormattingTenthsAndPlain
 * @brief Verifies primary Ammo Formatting Tenths And Plain.
 *
 * @details
 * Exercises the HudDisplay code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/huddisplay.cpp
 * @ingroup descent3_tests
 */
TEST(HudDisplay, PrimaryAmmoFormattingTenthsAndPlain) {
  ResetRecorders();
  Player_num = 0;
  Players[0].weapon[PW_PRIMARY].index = 2;
  Players[0].ship_index = 1;
  ship *s = &Ships[1];
  memset(s, 0, sizeof(*s));
  HUD_resources.wpn_bmp = 0; // GetWeaponFromIndex stub returns null -> icon == wpn_bmp -> no icon quad

  tHUDItem item;
  memset(&item, 0, sizeof(item));
  item.type = HUD_ITEM_PRIMARY;
  item.stat = STAT_GRAPHICAL;
  item.alpha = 90;
  item.tx = 3;
  item.ty = 4;

  // plain integer ammo: ammo line + weapon-name line
  s->static_wb[2].ammo_usage = 1;
  s->fire_flags[2] = 0;
  Players[0].weapon_ammo[2] = 250;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.size(), 2u);
  EXPECT_NE(g_puts[0].str.find("250"), std::string::npos);
  EXPECT_NE(g_puts[1].str.find("Mass"), std::string::npos); // table name for msg 12
  EXPECT_TRUE(g_draws.empty());                             // icon suppressed when == wpn_bmp

  // tenths mode splits into N.M
  ResetRecorders();
  s->fire_flags[2] |= SFF_TENTHS;
  Players[0].weapon_ammo[2] = 47;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.size(), 2u);
  EXPECT_NE(g_puts[0].str.find("4.7"), std::string::npos);

  // weapons without ammo draw name only, shifted down half a line
  ResetRecorders();
  s->static_wb[2].ammo_usage = 0;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.size(), 1u);
  EXPECT_NE(g_puts[0].str.find("Mass"), std::string::npos);
}

/**
 * @test HudDisplay.RouterGuardsCustomTextAndSkipsUnknownTypes
 * @brief Verifies router Guards Custom Text And Skips Unknown Types.
 *
 * @details
 * Exercises the HudDisplay code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/huddisplay.cpp
 * @ingroup descent3_tests
 */
TEST(HudDisplay, RouterGuardsCustomTextAndSkipsUnknownTypes) {
  ResetRecorders();

  // CUSTOMTEXT with no string renders nothing at all
  tHUDItem item;
  memset(&item, 0, sizeof(item));
  item.type = HUD_ITEM_CUSTOMTEXT;
  item.data.text = nullptr;
  item.color = GR_RGB(1, 2, 3);
  item.alpha = 99;
  item.saturation_count = 5;
  item.x = 11;
  item.y = 22;
  HudDisplayRouter(&item);
  EXPECT_TRUE(g_puts.empty());
  EXPECT_TRUE(g_draws.empty());

  // with text it forwards the exact styling fields; saturation_count=5
  // means six puts of the same string
  static char msg[] = "hello";
  item.data.text = msg;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.size(), 6u);
  EXPECT_EQ(g_puts[0].str, "hello");
  EXPECT_EQ(g_puts[0].col, item.color);
  EXPECT_EQ(static_cast<int>(g_puts[0].alpha), 99);
  EXPECT_EQ(g_puts[0].x, 11);
  EXPECT_EQ(g_puts[0].y, 22);

  // CUSTOMTEXT2 pins to the fixed bottom slot coordinates regardless of
  // the item's own x/y
  item.type = HUD_ITEM_CUSTOMTEXT2;
  ResetRecorders();
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.size(), 6u);
  EXPECT_EQ(g_puts[0].x, 2);
  EXPECT_EQ(g_puts[0].y, 240);

  // CUSTOMIMAGE is accepted as an intentional no-op
  ResetRecorders();
  item.type = HUD_ITEM_CUSTOMIMAGE;
  HudDisplayRouter(&item);
  EXPECT_TRUE(g_puts.empty());

  // unknown types hit the release Int3() no-op: silent, nothing drawn
  item.type = 99;
  HudDisplayRouter(&item);
  EXPECT_TRUE(g_puts.empty());
  EXPECT_TRUE(g_draws.empty());
}

/**
 * @test HudDisplay.ShipStatusCloakFadeAlphaFollowsFixCosCurve
 * @brief Verifies ship Status Cloak Fade Alpha Follows Fix Cos Curve.
 *
 * @details
 * Exercises the HudDisplay code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/huddisplay.cpp
 * @ingroup descent3_tests
 */
TEST(HudDisplay, ShipStatusCloakFadeAlphaFollowsFixCosCurve) {
  ResetRecorders();
  Player_num = 0;
  Players[0].objnum = 5;
  Players[0].flags = 0;
  Objects[5].effect_info = &g_status_effect_info;

  HUD_resources.ship_bmp = 31;

  tHUDItem item;
  memset(&item, 0, sizeof(item));
  item.type = HUD_ITEM_SHIPSTATUS;
  item.stat = STAT_GRAPHICAL;
  item.alpha = 180;

  // cloak ending soon (< CLOAKEND 3.0): clk_alpha =
  // 128 - 127*FixCos(65536*frac); fraction .5 -> cos(pi) -> -1 -> 255.
  // Cloaked ships draw the quad plus a "clk" label at alpha 255-clk.
  g_status_effect_info.type_flags = EF_CLOAKED;
  g_status_effect_info.cloak_time = 1.5f;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_draws.size(), 1u);
  EXPECT_EQ(g_draws[0].bm, HUD_resources.ship_bmp);
  ASSERT_EQ(g_puts.size(), 1u); // sat 0
  EXPECT_EQ(g_puts[0].str, "clk");

  // plenty of cloak time left: ship invisible (alpha 0)
  ResetRecorders();
  g_status_effect_info.cloak_time = 9.0f;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.size(), 1u);
  EXPECT_EQ(static_cast<int>(g_puts[0].alpha), 255); // 255 - 0

  // uncloaked: plain item alpha, no status label
  ResetRecorders();
  g_status_effect_info.type_flags = 0;
  HudDisplayRouter(&item);
  ASSERT_EQ(g_puts.empty(), true);
}
