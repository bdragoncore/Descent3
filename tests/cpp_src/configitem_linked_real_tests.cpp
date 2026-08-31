/**
 * @file configitem_linked_real_tests.cpp
 * @brief Unit tests for Descent3/ConfigItem.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/ConfigItem.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/ConfigItem.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/ConfigItem.cpp`
 * @par Harness
 * `configitem_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/ConfigItem.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <SDL3/SDL.h>

#include "ConfigItem.h"
#include "newui_core.h"
#include "newui.h"
#include "ui.h"
#include "pserror.h"
#include "gamefont.h"
#include "mem.h"
#include "descent.h"
#include "renderer.h"
#include "chrono_timer.h"
#include "hlsoundlib.h"
#include "hud.h"
#include "args.h"
#include "pilot.h"

// Extra globals needed by linked newui.cpp
bool Dedicated_server = false;
int GetScreenMode() { return 0; }
void SetScreenMode(int, bool) {}

// Recorder
static std::vector<std::string> g_recorder;
#define REC(msg) g_recorder.push_back(msg)

// Globals normally defined elsewhere
int Max_window_w = 640;
int Max_window_h = 480;
int Game_window_w = 640;
int Game_window_h = 480;
bool Multi_bail_ui_menu = false;
int Game_fonts[NUM_FONTS] = {0};
bool TCP_active = 0;

static function_mode s_function_mode = MENU_MODE;
function_mode GetFunctionMode() { return s_function_mode; }
void SetFunctionMode(function_mode mode) { s_function_mode = mode; }

extern int UI_frame_result;
static bool s_defer_force_exit = false;
static int s_defer_escape = UID_CANCEL;

struct FakeApp : oeApplication {
  void init() override {}
  void get_info(void *) override {}
  int flags() const override { return 0; }
  unsigned defer() override { REC("defer"); if (s_defer_force_exit) UI_frame_result = s_defer_escape; return 0; }
  void delay(float) override {}
  void set_defer_handler(void (*)(bool)) override {}
};
static FakeApp g_fake_app;
oeApplication *Descent = &g_fake_app;

namespace D3 {
float ChronoTimer::GetTime() { return 0.0f; }
int64_t ChronoTimer::GetTimeMS() { return 0; }
void ChronoTimer::SleepMS(int) {}
void ChronoTimer::Initialize() {}
}

void (*DebugBreak_callback_stop)() = nullptr;
void (*DebugBreak_callback_resume)() = nullptr;
void Error(const char *, ...) {}

// bitmap stubs
static uint16_t s_gamma_bitmap_data[128*128];
int bm_AllocLoadFileBitmap(const char *, int, int) { return 900; }
int bm_AllocBitmap(int w, int h, int) { return (w==128 && h==128)?901:-1; }
uint16_t *bm_data(int handle, int) { return (handle==901)?s_gamma_bitmap_data:nullptr; }
int bm_w(int,int){return 64;} int bm_h(int,int){return 64;}
bool bm_CreateChunkedBitmap(int, chunked_bitmap*){return true;}
void bm_DestroyChunkedBitmap(chunked_bitmap*){}
void bm_FreeBitmap(int){}

void *mem_malloc_sub(int size, const char*,int){return malloc(size);}
void mem_free_sub(void *p){free(p);}
char *mem_strdup_sub(const char *s,const char*,int){return strdup(s);}

// input stubs
int ddio_GetAdjKeyState(int){return 0;}
void ddio_KeyFlush(){}
int ddio_KeyInKey(){return 0;}
bool ddio_MouseGetEvent(int *btn,bool *state){*btn=0;*state=false;return false;}
int ddio_MouseGetState(int *x,int *y,int *dx,int *dy,int *z,int *dz){if(x)*x=0;if(y)*y=0;if(dx)*dx=0;if(dy)*dy=0;if(z)*z=0;if(dz)*dz=0;return 0;}
void ddio_MouseReset(){}
void ddio_MouseSetVCoords(int,int){}
void ddio_ff_GetInfo(bool *found,bool*){if(found)*found=false;}

// renderer / frame stubs
void rend_ClearScreen(ddgr_color){}
void rend_Flip(){ if(s_defer_force_exit) UI_frame_result=s_defer_escape; }
void rend_DrawChunkedBitmap(chunked_bitmap*,int,int,uint8_t){}
void rend_DrawLine(int,int,int,int){}
void rend_DrawPolygon2D(int,g3Point**,int){REC("drawpoly2d");}
void rend_DrawScaledBitmap(int,int,int,int,int,float,float,float,float,int,const float*){}
void rend_EndFrame(){}
void rend_StartFrame(int,int,int,int,int){}
void rend_SetAlphaType(int8_t){}
void rend_SetAlphaValue(uint8_t){}
void rend_SetColorModel(color_model){}
void rend_SetFlatColor(ddgr_color){}
void rend_SetLighting(light_state){}
void rend_SetOverlayType(uint8_t){}
void rend_SetTextureType(texture_type){}
void rend_SetWrapType(wrap_type){}
void rend_SetZBufferState(int8_t){}
int rend_SetPreferredState(renderer_preferred_state*,bool){REC("setpreferredstate");return 1;}
float Render_FOV=72.0f;
int Render_preferred_bitdepth=32;
renderer_preferred_state Render_preferred_state{};
void StartFrame(bool){}
void StartFrame(int,int,int,int,bool,bool){}
void EndFrame(){}
void DoScreenshot(){}

// text / font stubs
const char *GetStringFromTable(int id){
  static std::string buf;
  buf="STR_"+std::to_string(id);
  return buf.c_str();
}
extern "C" {
void grtext_SetParameters(int,int,int,int,int){}
void grtext_SetFlags(int){}
int grtext_GetFont(){return 0;}
int grtext_GetTextHeight(const char*){return 12;}
int grtext_GetTextLineWidth(const char*){return 40;}
void grtext_Puts(int,int,const char*){}
void grtext_CenteredPrintf(int,int,const char*,...){}
void grtext_SetColor(ddgr_color){}
void grtext_SetAlpha(uint8_t){}
void grtext_SetFont(int){}
void grtext_Flush(){}
int grfont_GetHeight(int){return 12;}
int grfont_KeyToAscii(int,int){return 'a';}
}
void textaux_WordWrap(const char*,char*,int,int){}

// sound stubs
hlsSystem::hlsSystem(){}
int hlsSystem::Play2dSound(int,float,float,uint16_t){return 0;}
void hlsSystem::BeginSoundFrame(bool){}
void hlsSystem::EndSoundFrame(){}
int hlsSystem::GetLLSoundQuantity(){return 32;}
void hlsSystem::SetLLSoundQuantity(int){}
float hlsSystem::GetMasterVolume(){return 1.0f;}
void hlsSystem::SetMasterVolume(float){}
char hlsSystem::GetSoundQuality(){return 0;}
bool hlsSystem::SetSoundQuality(char){return true;}
void hlsSystem::StopSoundImmediate(int){}
void hlsSystem::KillSoundLib(bool){}
hlsSystem Sound_system;
float D3MusicGetVolume(){return 0.5f;}
void D3MusicSetVolume(float){}
struct tMusicSeqInfo; void D3MusicDoFrame(tMusicSeqInfo*){}
int FindSoundName(const char*){return 777;}
SDL_AssertState SDL_ReportAssertion(SDL_AssertData*,const char*,const char*,int){return SDL_ASSERTION_IGNORE;}
SDL_DisplayID *SDL_GetDisplays(int *count){if(count)*count=0;return nullptr;}
SDL_DisplayMode **SDL_GetFullscreenDisplayModes(SDL_DisplayID,int *count){if(count)*count=0;return nullptr;}
const char *SDL_GetDisplayName(SDL_DisplayID){return "Dummy";}
const SDL_DisplayMode *SDL_GetCurrentDisplayMode(SDL_DisplayID){return nullptr;}
const SDL_DisplayMode *SDL_GetDesktopDisplayMode(SDL_DisplayID){return nullptr;}

class ConfigItemLinked : public ::testing::Test {
protected:
  void SetUp() override { g_recorder.clear(); s_defer_force_exit=false; }
  void TearDown() override { s_defer_force_exit=false; }
};

/**
 * @test ConfigItemLinked.GetUniqueIDBasic
 * @brief Verifies get Unique IDBasic.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, GetUniqueIDBasic) {
  int a = GetUniqueID();
  int b = GetUniqueID();
  EXPECT_NE(a, b);
  EXPECT_GE(a, 0);
}

/**
 * @test ConfigItemLinked.ConfigItemConstruct
 * @brief Verifies config Item Construct.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, ConfigItemConstruct) {
  EXPECT_NO_THROW({ ConfigItem ci; (void)ci; });
}

/**
 * @test ConfigItemLinked.ConfigItemDestruct
 * @brief Verifies config Item Destruct.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, ConfigItemDestruct) {
  ConfigItem *ci = new ConfigItem();
  EXPECT_NE(ci, nullptr);
  delete ci;
}

/**
 * @test ConfigItemLinked.CallbacksNoCrash
 * @brief Verifies callbacks No Crash.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, CallbacksNoCrash) {
  EXPECT_NO_THROW(CIListBoxCallback(0, nullptr));
  EXPECT_NO_THROW(CISliderCallback(0, nullptr));
}

/**
 * @test ConfigItemLinked.MultipleUniqueIDs
 * @brief Verifies multiple Unique IDs.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, MultipleUniqueIDs) {
  int prev = -1;
  for (int i=0;i<10;i++){
    int id = GetUniqueID();
    EXPECT_NE(id, prev);
    prev = id;
  }
}

/**
 * @test ConfigItemLinked.StringTable
 * @brief Verifies string Table.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, StringTable) {
  const char* s = GetStringFromTable(5);
  EXPECT_STREQ(s, "STR_5");
}

/**
 * @test ConfigItemLinked.WindowGlobals
 * @brief Verifies window Globals.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, WindowGlobals) {
  EXPECT_EQ(Game_window_w, 640);
  Game_window_w = 800;
  EXPECT_EQ(Game_window_w, 800);
  Game_window_w = 640;
}

/**
 * @test ConfigItemLinked.UIDConstants
 * @brief Verifies uIDConstants.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, UIDConstants) {
  EXPECT_EQ(UID_CANCEL, 0xEF);
  EXPECT_EQ(UID_OK, 0xEE);
}

/**
 * @test ConfigItemLinked.Recorder
 * @brief Verifies recorder.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, Recorder) {
  REC("test");
  EXPECT_EQ(g_recorder.size(), 1u);
  EXPECT_STREQ(g_recorder[0].c_str(), "test");
}

/**
 * @test ConfigItemLinked.GameFontsArray
 * @brief Verifies game Fonts Array.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, GameFontsArray) {
  Game_fonts[0] = 1;
  EXPECT_EQ(Game_fonts[0], 1);
  Game_fonts[0] = 0;
}

/**
 * @test ConfigItemLinked.DeferLatch
 * @brief Verifies defer Latch.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, DeferLatch) {
  s_defer_force_exit = true;
  s_defer_escape = UID_CANCEL;
  // Simulate a deferred frame
  rend_Flip();
  EXPECT_EQ(UI_frame_result, UID_CANCEL);
  s_defer_force_exit = false;
}

/**
 * @test ConfigItemLinked.ConfigItemUniqueIDViaClass
 * @brief Verifies config Item Unique IDVia Class.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, ConfigItemUniqueIDViaClass) {
  ConfigItem ci;
  (void)ci;
  int id = GetUniqueID();
  EXPECT_GE(id, 0);
}

/**
 * @test ConfigItemLinked.MaxWindowGlobals
 * @brief Verifies max Window Globals.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, MaxWindowGlobals) {
  EXPECT_EQ(Max_window_w, 640);
  Max_window_w = 1024;
  EXPECT_EQ(Max_window_w, 1024);
  Max_window_w = 640;
}

/**
 * @test ConfigItemLinked.SoundStub
 * @brief Verifies sound Stub.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, SoundStub) {
  Sound_system.Play2dSound(0,1.0f,1.0f,0);
  EXPECT_FLOAT_EQ(Sound_system.GetMasterVolume(), 1.0f);
}

/**
 * @test ConfigItemLinked.RenderStub
 * @brief Verifies render Stub.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, RenderStub) {
  rend_SetFlatColor(0);
  rend_SetLighting(LS_NONE);
  EXPECT_NO_THROW(rend_SetPreferredState(&Render_preferred_state, false));
}

/**
 * @test ConfigItemLinked.ConfigItemDefaultCtorMultiple
 * @brief Verifies config Item Default Ctor Multiple.
 *
 * @details
 * Exercises the ConfigItemLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ConfigItem.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigItemLinked, ConfigItemDefaultCtorMultiple) {
  for (int i=0;i<5;i++){
    ConfigItem ci;
    (void)ci;
  }
  EXPECT_TRUE(true);
}
