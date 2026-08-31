/**
 * @file weapon_real_tests.cpp
 * @brief Unit tests for Descent3/weapon.cpp: weapon-table allocation above the.
 *
 * @details
 * static area, image loading dispatch, static-weapon remapping, player
 * weapon acquisition, slot selection/class toggling, autoselect lists and
 * the alpha-blend overlay.
 *
 * This harness validates the behavior of `Descent3/weapon.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/weapon.cpp`
 * @par Harness
 * `weapon_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/weapon.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "weapon.h"
#include "object.h"
#include "player.h"
#include "ship.h"
#include "vclip.h"
#include "polymodel.h"
#include "gametexture.h"
#include "hlsoundlib.h"
#include "cfile.h"
#include "renderer.h"
#include "sounds.h"

// These are file-local to weapon.cpp (declared there, not in weapon.h).
void SelectPrimaryWeapon(int slot);
void SelectSecondaryWeapon(int slot);
void SetPrimaryWeapon(int index, int slot);
void SetSecondaryWeapon(int index, int slot);

#define REC(msg) g_recorder.push_back(msg)
static std::vector<std::string> g_recorder;

// --- globals normally provided by other game modules ---
int Game_window_w = 640;
int Game_window_h = 480;
float Gametime = 0.0f;
int Player_num = 0;
object *Player_object = nullptr;
object Objects[MAX_OBJECTS];
player Players[MAX_PLAYERS];
ship Ships[MAX_SHIPS];
vclip GameVClips[MAX_VCLIPS];
poly_model Poly_models[MAX_POLY_MODELS];
object_info Object_info[MAX_OBJECT_IDS];

Inventory::Inventory() {}
Inventory::~Inventory() {}

// --- loader stubs ---
static std::vector<std::string> s_vclip_names, s_bitmap_names, s_model_names, s_texture_names;
static int s_pageinvclip_last = -1;

int AllocLoadVClip(const char *filename, int texture_size, int mipped, int pageable, int format) {
  REC(std::string("loadvclip:") + filename);
  s_vclip_names.push_back(filename);
  return 777;
}
int bm_AllocLoadFileBitmap(const char *filename, int local, int format) {
  REC(std::string("loadbitmap:") + filename);
  s_bitmap_names.push_back(filename);
  return 555;
}
int LoadPolyModel(const std::filesystem::path &path, int pageable) {
  REC(std::string("loadmodel:") + path.string());
  s_model_names.push_back(path.string());
  return 888;
}
int LoadTextureImage(const char *filename, int *type, int texture_size, int mipped, int pageable, int format) {
  REC(std::string("loadtexture:") + filename);
  s_texture_names.push_back(filename);
  return 999;
}
void PageInVClip(int n) { s_pageinvclip_last = n; }

// --- HUD / AI / reticle / firing stubs ---
static int s_hud_count = 0;
static int s_ainotify_count = 0;
static int s_clearfire_count = 0;
static int s_reticle_count = 0;
static int s_play2d_last = -1;

void AddHUDMessage(const char *, ...) { s_hud_count++; }
void AINotify(object *, unsigned char, void *) { s_ainotify_count++; }
void ClearPlayerFiring(object *, int) { s_clearfire_count++; }
void ResetReticle() { s_reticle_count++; }

// --- sound system ---
hlsSystem::hlsSystem() {}
int hlsSystem::Play2dSound(int id, float volume, float pan, uint16_t frequency) {
  s_play2d_last = id;
  return 0;
}
void hlsSystem::KillSoundLib(bool) {}
hlsSystem Sound_system;

// --- renderer ---
static std::vector<std::string> s_rend_calls;
void rend_SetZBufferState(int8_t state) { s_rend_calls.push_back(std::string("zbuf:") + std::to_string(state)); }
void rend_SetTextureType(texture_type t) {
  const char *n = (t == TT_FLAT) ? "flat" : "other";
  s_rend_calls.push_back(std::string("tex:") + n);
}
void rend_SetAlphaType(int8_t t) { s_rend_calls.push_back(std::string("alphatype:") + std::to_string(t)); }
void rend_SetAlphaValue(uint8_t v) { s_rend_calls.push_back(std::string("alphaval:") + std::to_string(v)); }
void rend_SetLighting(light_state l) { s_rend_calls.push_back(std::string("light:") + std::to_string(l)); }
void rend_SetFlatColor(ddgr_color c) { s_rend_calls.push_back(std::string("flat:") + std::to_string(c)); }
void rend_DrawPolygon2D(int texnum, g3Point **p, int nvtx) {
  s_rend_calls.push_back(std::string("drawpoly:") + std::to_string(nvtx));
}

// --- string table ---
const char *GetStringFromTable(int id) {
  static std::string buf;
  buf = "STR_" + std::to_string(id);
  return buf.c_str();
}

// --- CFILE short read/write backed by a real FILE* ---
int16_t cf_ReadShort(CFILE *c, bool little_endian) {
  uint8_t b[2];
  if (fread(b, 1, 2, c->file) != 2)
    return 0;
  if (little_endian)
    return (int16_t)(b[0] | (b[1] << 8));
  return (int16_t)(((uint16_t)b[0] << 8) | b[1]);
}
void cf_WriteShort(CFILE *c, int16_t s) {
  uint8_t b[2] = {(uint8_t)s, (uint8_t)((uint16_t)s >> 8)};
  fwrite(b, 1, 2, c->file);
}

// otype_wb_info buffer for Object_info[i].static_wb pointers
static otype_wb_info s_objinfo_wb[MAX_WBS_PER_OBJ];

// NUM_PRIMARY_SLOTS / NUM_SECONDARY_SLOTS are file-local in weapon.cpp.
#define NUM_PRIMARY_SLOTS 5
#define NUM_SECONDARY_SLOTS 5

/**
 * @brief GTest fixture for WeaponTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class WeaponTest : public ::testing::Test {
protected:
  void SetUp() override {
    memset(Objects, 0, sizeof(Objects));
    memset(Players, 0, sizeof(Players));
    memset(Ships, 0, sizeof(Ships));
    memset(Object_info, 0, sizeof(Object_info));
    memset(GameVClips, 0, sizeof(GameVClips));
    memset(Poly_models, 0, sizeof(Poly_models));
    memset(s_objinfo_wb, 0, sizeof(s_objinfo_wb));
    InitWeapons();
    Player_num = 0;
    Player_object = nullptr;
    Gametime = 0.0f;
    Game_window_w = 640;
    Game_window_h = 480;
    s_vclip_names.clear();
    s_bitmap_names.clear();
    s_model_names.clear();
    s_texture_names.clear();
    s_pageinvclip_last = -1;
    s_hud_count = 0;
    s_ainotify_count = 0;
    s_clearfire_count = 0;
    s_reticle_count = 0;
    s_play2d_last = -1;
    s_rend_calls.clear();
    g_recorder.clear();
  }

  // Builds a minimal player setup: player 0 flying ship 0, pilot object objnum.
  // object::dynamic_wb is a pointer, so it must be backed by real storage.
  object *MakeLocalPlayer(int objnum) {
    static dynamic_wb_info dwb_storage[MAX_WBS_PER_OBJ];
    memset(dwb_storage, 0, sizeof(dwb_storage));
    Players[0].objnum = objnum;
    Players[0].ship_index = 0;
    Objects[objnum].type = OBJ_PLAYER;
    Objects[objnum].dynamic_wb = dwb_storage;
    Player_object = &Objects[objnum];
    return &Objects[objnum];
  }

  // Saves Weapon_slot_mask through the real save function and returns raw bytes.
  uint16_t SavedSlotMask() {
    FILE *f = tmpfile();
    CFILE cf = {};
    cf.file = f;
    SaveWeaponSelectStates(&cf);
    fflush(f);
    rewind(f);
    uint8_t b[2] = {0, 0};
    fread(b, 1, 2, f);
    fclose(f);
    return (uint16_t)(b[0] | (b[1] << 8));
  }
};

/**
 * @test WeaponTest.AllocStartsAboveStaticAreaWithDefaults
 * @brief Verifies alloc Starts Above Static Area With Defaults.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, AllocStartsAboveStaticAreaWithDefaults) {
  int a = AllocWeapon();
  EXPECT_EQ(a, MAX_STATIC_WEAPONS);
  EXPECT_EQ(Num_weapons, 1);
  EXPECT_EQ(Weapons[a].used, 1);
  EXPECT_EQ(Weapons[a].alpha, 1.0f);
  EXPECT_EQ(Weapons[a].spawn_handle, -1);
  EXPECT_EQ(Weapons[a].alternate_spawn_handle, -1);
  EXPECT_EQ(Weapons[a].icon_handle, -1);
  EXPECT_EQ(Weapons[a].scorch_handle, -1);
  EXPECT_EQ(Weapons[a].particle_handle, -1);
  EXPECT_EQ(Weapons[a].robot_spawn_handle, -1);
  for (int t = 0; t < MAX_WEAPON_SOUNDS; t++)
    EXPECT_EQ(Weapons[a].sounds[t], SOUND_NONE_INDEX);
  EXPECT_EQ(Weapons[a].gravity_size, 1.0f);
  EXPECT_EQ(Weapons[a].gravity_time, 1.0f);
  EXPECT_EQ(Weapons[a].explode_size, 1.0f);
  EXPECT_EQ(Weapons[a].explode_time, 1.0f);
}

/**
 * @test WeaponTest.FreeClearsSlotAndCount
 * @brief Verifies free Clears Slot And Count.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, FreeClearsSlotAndCount) {
  int a = AllocWeapon();
  FreeWeapon(a);
  EXPECT_EQ(Num_weapons, 0);
  EXPECT_EQ(Weapons[a].used, 0);
  EXPECT_EQ(Weapons[a].name[0], 0);
}

/**
 * @test WeaponTest.AllocReusesFreedDynamicSlot
 * @brief Verifies alloc Reuses Freed Dynamic Slot.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, AllocReusesFreedDynamicSlot) {
  int a = AllocWeapon();
  int b = AllocWeapon();
  EXPECT_EQ(b, a + 1);
  FreeWeapon(a);
  EXPECT_EQ(AllocWeapon(), a);
  EXPECT_EQ(Num_weapons, 2);
}

/**
 * @test WeaponTest.NextAndPrevWrapAroundDynamicSlots
 * @brief Verifies next And Prev Wrap Around Dynamic Slots.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, NextAndPrevWrapAroundDynamicSlots) {
  int a = AllocWeapon(); // 21
  int b = AllocWeapon(); // 22
  int c = AllocWeapon(); // 23
  (void)b;
  EXPECT_EQ(GetNextWeapon(c), a); // wraps past end
  EXPECT_EQ(GetPrevWeapon(a), c); // wraps below static area
  FreeWeapon(b);
  EXPECT_EQ(GetNextWeapon(a), c);
  // single weapon returns itself
  InitWeapons();
  int only = AllocWeapon();
  EXPECT_EQ(GetNextWeapon(only), only);
  EXPECT_EQ(GetPrevWeapon(only), only);
}

/**
 * @test WeaponTest.FindNameIsCaseInsensitiveAndSkipsUnused
 * @brief Verifies find Name Is Case Insensitive And Skips Unused.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, FindNameIsCaseInsensitiveAndSkipsUnused) {
  int a = AllocWeapon();
  strcpy(Weapons[a].name, "Laser");
  EXPECT_EQ(FindWeaponName("laser"), a);
  EXPECT_EQ(FindWeaponName("LAser"), a);

  int b = AllocWeapon();
  strcpy(Weapons[b].name, "Ghost");
  FreeWeapon(b);
  EXPECT_EQ(FindWeaponName("ghost"), -1);
  EXPECT_EQ(FindWeaponName("missing"), -1);
}

/**
 * @test WeaponTest.HudImageNonAnimatedReturnsHandleDirectly
 * @brief Verifies hud Image Non Animated Returns Handle Directly.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, HudImageNonAnimatedReturnsHandleDirectly) {
  int h = AllocWeapon();
  Weapons[h].flags &= ~WF_HUD_ANIMATED;
  Weapons[h].hud_image_handle = 42;
  EXPECT_EQ(GetWeaponHudImage(h, 7), 42);
  EXPECT_EQ(GetWeaponHudImage(h, 0), 42);
}

/**
 * @test WeaponTest.HudImageAnimatedIndexesVClipFrames
 * @brief Verifies hud Image Animated Indexes VClip Frames.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, HudImageAnimatedIndexesVClipFrames) {
  int h = AllocWeapon();
  Weapons[h].flags |= WF_HUD_ANIMATED;
  Weapons[h].hud_image_handle = 5;
  static int16_t frames[3] = {10, 11, 12};
  GameVClips[5].used = 1;
  GameVClips[5].num_frames = 3;
  GameVClips[5].frame_time = 1.0f;
  GameVClips[5].frames = frames;

  Gametime = 2.5f; // int_frame = 2
  EXPECT_EQ(GetWeaponHudImage(h, 0), 12);
  EXPECT_EQ(GetWeaponHudImage(h, 1), 10); // (2+1)%3
  Gametime = 0.5f;                        // int_frame = 0 + framenum
  EXPECT_EQ(GetWeaponHudImage(h, 2), 12); // (0+2)%3
}

/**
 * @test WeaponTest.LoadHudImageDispatchesByExtension
 * @brief Verifies load Hud Image Dispatches By Extension.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, LoadHudImageDispatchesByExtension) {
  int type = -1;
  EXPECT_EQ(LoadWeaponHudImage((char *)"gun.oaf", &type), 777);
  EXPECT_EQ(type, 1);
  ASSERT_FALSE(s_vclip_names.empty());
  EXPECT_EQ(s_vclip_names.back(), "gun.oaf");

  type = -1;
  EXPECT_EQ(LoadWeaponHudImage((char *)"gun.bmp", &type), 555);
  EXPECT_EQ(type, 0);
  ASSERT_FALSE(s_bitmap_names.empty());
  EXPECT_EQ(s_bitmap_names.back(), "gun.bmp");

  // .ifl and .abm also count as animations
  type = -1;
  EXPECT_EQ(LoadWeaponHudImage((char *)"seq.ifl", &type), 777);
  EXPECT_EQ(type, 1);
  type = -1;
  EXPECT_EQ(LoadWeaponHudImage((char *)"tex.abm", &type), 777);
  EXPECT_EQ(type, 1);

  // too short: no extension possible
  EXPECT_EQ(LoadWeaponHudImage((char *)"ab", &type), -1);
}

/**
 * @test WeaponTest.LoadFireImageDispatchesModelVsTexture
 * @brief Verifies load Fire Image Dispatches Model Vs Texture.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, LoadFireImageDispatchesModelVsTexture) {
  int type = -1, anim = -1;
  EXPECT_EQ(LoadWeaponFireImage((char *)"ship.pof", &type, &anim), 888);
  EXPECT_EQ(type, 1);
  EXPECT_EQ(anim, 0);
  ASSERT_FALSE(s_model_names.empty());
  EXPECT_EQ(s_model_names.back(), "ship.pof");

  type = anim = -1;
  EXPECT_EQ(LoadWeaponFireImage((char *)"flash.gif", &type, &anim), 999);
  EXPECT_EQ(type, 0);
  EXPECT_EQ(anim, 0);
  ASSERT_FALSE(s_texture_names.empty());
  EXPECT_EQ(s_texture_names.back(), "flash.gif");

  type = -1;
  EXPECT_EQ(LoadWeaponFireImage((char *)"ab", &type, &anim), -1);
}

/**
 * @test WeaponTest.FireImageVclipPathPagesInAndFrames
 * @brief Verifies fire Image Vclip Path Pages In And Frames.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, FireImageVclipPathPagesInAndFrames) {
  int h = AllocWeapon();
  Weapons[h].flags |= WF_IMAGE_VCLIP;
  Weapons[h].fire_image_handle = 3;
  static int16_t frames[2] = {70, 71};
  GameVClips[3].used = 1;
  GameVClips[3].num_frames = 2;
  GameVClips[3].frame_time = 0.5f;
  GameVClips[3].frames = frames;

  Gametime = 1.0f; // cur_frametime = 2 -> frame index 2%2=0
  EXPECT_EQ(GetWeaponFireImage(h, 0), 70);
  EXPECT_EQ(s_pageinvclip_last, 3);

  Gametime = 0.75f; // cur_frametime = 1 -> frame index 1
  EXPECT_EQ(GetWeaponFireImage(h, 0), 71);
}

/**
 * @test WeaponTest.FireImageNonVclipReturnsHandle
 * @brief Verifies fire Image Non Vclip Returns Handle.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, FireImageNonVclipReturnsHandle) {
  int h = AllocWeapon();
  Weapons[h].flags &= ~WF_IMAGE_VCLIP;
  Weapons[h].fire_image_handle = 9;
  EXPECT_EQ(GetWeaponFireImage(h, 4), 9);
  EXPECT_EQ(s_pageinvclip_last, -1);
}

/**
 * @test WeaponTest.MatchWeaponToIndexVariants
 * @brief Verifies match Weapon To Index Variants.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, MatchWeaponToIndexVariants) {
  EXPECT_EQ(MatchWeaponToIndex((char *)"Nope", 5), -1);

  // already matched in place
  Weapons[5].used = 1;
  strcpy(Weapons[5].name, "InPlace");
  Num_weapons = 1;
  EXPECT_EQ(MatchWeaponToIndex((char *)"inplace", 5), 0);
  EXPECT_EQ(Num_weapons, 1);

  // destination free: weapon simply moves
  int src = AllocWeapon();
  strcpy(Weapons[src].name, "Mover");
  EXPECT_EQ(MatchWeaponToIndex((char *)"mover", 6), 0);
  EXPECT_STREQ(Weapons[6].name, "Mover");
  EXPECT_EQ(Weapons[src].used, 0);
  EXPECT_EQ(Num_weapons, 2); // InPlace + Mover

  // destination used: old occupant is preserved in a new dynamic slot
  int other = AllocWeapon(); // occupies next slot so dest is used
  strcpy(Weapons[other].name, "Occupant");
  EXPECT_GT(MatchWeaponToIndex((char *)"mover", other), 0);
  // the occupant was relocated above the static area with its data intact
  bool found_occupant = false;
  for (int i = MAX_STATIC_WEAPONS; i < MAX_WEAPONS; i++)
    if (Weapons[i].used && strcmp(Weapons[i].name, "Occupant") == 0)
      found_occupant = true;
  EXPECT_TRUE(found_occupant);
  EXPECT_STREQ(Weapons[other].name, "Mover");
}

/**
 * @test WeaponTest.MoveWeaponFromIndexRelocatesToDynamicArea
 * @brief Verifies move Weapon From Index Relocates To Dynamic Area.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, MoveWeaponFromIndexRelocatesToDynamicArea) {
  Weapons[0].used = 1;
  strcpy(Weapons[0].name, "StaticOne");
  Num_weapons = 1;

  int ni = MoveWeaponFromIndex(0);
  EXPECT_EQ(ni, MAX_STATIC_WEAPONS);
  EXPECT_STREQ(Weapons[ni].name, "StaticOne");
  EXPECT_EQ(Weapons[0].used, 0);
  EXPECT_EQ(Num_weapons, 1);
}

/**
 * @test WeaponTest.RemapWeaponsMovesUnknownOutOfStaticArea
 * @brief Verifies remap Weapons Moves Unknown Out Of Static Area.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, RemapWeaponsMovesUnknownOutOfStaticArea) {
  Weapons[3].used = 1;
  strcpy(Weapons[3].name, "CustomGun");
  Num_weapons = 1;

  RemapWeapons();
  EXPECT_EQ(Weapons[3].used, 0);
  int ni = -1;
  for (int i = MAX_STATIC_WEAPONS; i < MAX_WEAPONS; i++)
    if (Weapons[i].used)
      ni = i;
  ASSERT_GE(ni, MAX_STATIC_WEAPONS);
  EXPECT_STREQ(Weapons[ni].name, "CustomGun");
}

/**
 * @test WeaponTest.RemapWeaponsPullsStrayStaticHome
 * @brief Verifies remap Weapons Pulls Stray Static Home.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, RemapWeaponsPullsStrayStaticHome) {
  // A weapon named after static entry 0 sits in the dynamic area.
  const char *static_name = Static_weapon_names[0];
  Weapons[MAX_STATIC_WEAPONS + 5].used = 1;
  strcpy(Weapons[MAX_STATIC_WEAPONS + 5].name, static_name);
  Num_weapons = 1;

  RemapWeapons();
  EXPECT_EQ(Weapons[0].used, 1);
  EXPECT_STREQ(Weapons[0].name, static_name);
  EXPECT_EQ(Weapons[MAX_STATIC_WEAPONS + 5].used, 0);
  EXPECT_EQ(Num_weapons, 1);
}

/**
 * @test WeaponTest.RemapAllWeaponObjectsRewiresEverything
 * @brief Verifies remap All Weapon Objects Rewires Everything.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, RemapAllWeaponObjectsRewiresEverything) {
  const int old_idx = 31, new_idx = 32;

  Objects[7].type = OBJ_WEAPON;
  Objects[7].id = old_idx;

  Object_info[9].static_wb = s_objinfo_wb;
  s_objinfo_wb[2].gp_weapon_index[3] = old_idx;

  Ships[1].static_wb[4].gp_weapon_index[5] = old_idx;

  int w = AllocWeapon();
  Weapons[w].spawn_handle = old_idx;
  Weapons[w].alternate_spawn_handle = old_idx;

  RemapAllWeaponObjects(old_idx, new_idx);

  EXPECT_EQ(Objects[7].id, new_idx);
  EXPECT_EQ(Object_info[9].static_wb[2].gp_weapon_index[3], new_idx);
  EXPECT_EQ(Ships[1].static_wb[4].gp_weapon_index[5], new_idx);
  EXPECT_EQ(Weapons[w].spawn_handle, new_idx);
  EXPECT_EQ(Weapons[w].alternate_spawn_handle, new_idx);

  // untouched entries stay put
  Objects[8].type = OBJ_WEAPON;
  Objects[8].id = old_idx;
  RemapAllWeaponObjects(old_idx, new_idx); // second call remaps again
  EXPECT_EQ(Objects[8].id, new_idx);
}

/**
 * @test WeaponTest.IsWeaponSecondaryBoundary
 * @brief Verifies is Weapon Secondary Boundary.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, IsWeaponSecondaryBoundary) {
  EXPECT_FALSE(IsWeaponSecondary(0));
  EXPECT_FALSE(IsWeaponSecondary(SECONDARY_INDEX - 1));
  EXPECT_TRUE(IsWeaponSecondary(SECONDARY_INDEX));
  EXPECT_TRUE(IsWeaponSecondary(19));
}

/**
 * @test WeaponTest.AddWeaponGrantsFlagAndClampsAmmo
 * @brief Verifies add Weapon Grants Flag And Clamps Ammo.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, AddWeaponGrantsFlagAndClampsAmmo) {
  Players[1].ship_index = 0;
  Ships[0].max_ammo[15] = 100;
  Players[1].weapon_ammo[15] = 90;
  Ships[0].static_wb[15].ammo_usage = 1;

  EXPECT_EQ(AddWeaponToPlayer(1, 15, 500), 1);
  EXPECT_NE(Players[1].weapon_flags & HAS_FLAG(15), 0u);
  EXPECT_EQ(Players[1].weapon_ammo[15], 100); // clamped to max

  // ammo under max: partial grant
  Players[1].weapon_flags = 0;
  Players[1].weapon_ammo[15] = 10;
  AddWeaponToPlayer(1, 15, 25);
  EXPECT_EQ(Players[1].weapon_ammo[15], 35);
}

/**
 * @test WeaponTest.AddWeaponForLocalPlayerAutoSelectsWhenCurrentUnusable
 * @brief Verifies add Weapon For Local Player Auto Selects When Current Unusable.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, AddWeaponForLocalPlayerAutoSelectsWhenCurrentUnusable) {
  MakeLocalPlayer(5);
  Ships[0].static_wb[0].ammo_usage = 0; // laser: pure energy
  Ships[0].static_wb[0].energy_usage = 10.0f;
  Ships[0].static_wb[1].ammo_usage = 0; // vauss: cheap energy
  Ships[0].static_wb[1].energy_usage = 1.0f;

  // current primary (laser) is unusable with no energy; vauss becomes available.
  Players[0].weapon[PW_PRIMARY].index = LASER_INDEX;
  Players[0].energy = 2.0f;
  EXPECT_EQ(AddWeaponToPlayer(Player_num, VAUSS_INDEX, 0), 1);
  EXPECT_EQ(Players[0].weapon[PW_PRIMARY].index, VAUSS_INDEX);
}

/**
 * @test WeaponTest.SelectStateRoundTripThroughCfile
 * @brief Verifies select State Round Trip Through Cfile.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SelectStateRoundTripThroughCfile) {
  ResetWeaponSelectStates(0x1234);
  EXPECT_EQ(SavedSlotMask(), 0x1234);

  FILE *f = tmpfile();
  CFILE out = {};
  out.file = f;
  SaveWeaponSelectStates(&out);
  fflush(f);

  ResetWeaponSelectStates();
  EXPECT_EQ(SavedSlotMask(), 0);

  rewind(f);
  CFILE in = {};
  in.file = f;
  LoadWeaponSelectStates(&in);
  fclose(f);

  EXPECT_EQ(SavedSlotMask(), 0x1234);
}

/**
 * @test WeaponTest.AutoSelectIndexAccessorsRoundTrip
 * @brief Verifies auto Select Index Accessors Round Trip.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, AutoSelectIndexAccessorsRoundTrip) {
  EXPECT_EQ(GetAutoSelectPrimaryWpnIdx(0), LASER_INDEX);
  EXPECT_EQ(GetAutoSelectPrimaryWpnIdx(1), VAUSS_INDEX);
  EXPECT_EQ(GetAutoSelectPrimaryWpnIdx(9), OMEGA_INDEX);
  EXPECT_EQ(GetAutoSelectPrimaryWpnIdx(50), WPNSEL_INVALID);

  EXPECT_EQ(GetAutoSelectSecondaryWpnIdx(0), CONCUSSION_INDEX);
  EXPECT_EQ(GetAutoSelectSecondaryWpnIdx(50), WPNSEL_INVALID);

  SetAutoSelectPrimaryWpnIdx(3, MASSDRIVER_INDEX);
  EXPECT_EQ(GetAutoSelectPrimaryWpnIdx(3), MASSDRIVER_INDEX);
  SetAutoSelectPrimaryWpnIdx(3, FUSION_INDEX); // restore original order
  EXPECT_EQ(GetAutoSelectPrimaryWpnIdx(3), FUSION_INDEX);

  SetAutoSelectSecondaryWpnIdx(2, MEGA_INDEX);
  EXPECT_EQ(GetAutoSelectSecondaryWpnIdx(2), MEGA_INDEX);
  SetAutoSelectSecondaryWpnIdx(2, IMPACTMORTAR_INDEX); // restore
}

/**
 * @test WeaponTest.SetPrimarySwitchesClassAndNotifies
 * @brief Verifies set Primary Switches Class And Notifies.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SetPrimarySwitchesClassAndNotifies) {
  object *pobj = MakeLocalPlayer(5);
  Players[0].weapon[PW_PRIMARY].index = LASER_INDEX;
  ResetWeaponSelectStates(0xFFFF);
  Gametime = 33.0f;

  SetPrimaryWeapon(VAUSS_INDEX, 1);

  EXPECT_EQ(Players[0].weapon[PW_PRIMARY].index, VAUSS_INDEX);
  // low class in slot 1 clears the slot bit
  EXPECT_EQ(SavedSlotMask(), (uint16_t)(0xFFFF & ~(1 << 1)));
  EXPECT_EQ(s_hud_count, 1);
  EXPECT_EQ(s_play2d_last, SOUND_CHANGE_PRIMARY);
  EXPECT_EQ(s_clearfire_count, 1);
  EXPECT_EQ(s_reticle_count, 1);
  EXPECT_EQ(s_ainotify_count, 1);
  EXPECT_FLOAT_EQ(pobj->dynamic_wb[VAUSS_INDEX].last_fire_time, 33.0f);

  // selecting the same weapon again is a no-op
  SetPrimaryWeapon(VAUSS_INDEX, 1);
  EXPECT_EQ(s_hud_count, 1);
}

/**
 * @test WeaponTest.SetSecondarySetsHighClassBitAndAlwaysClearsFiring
 * @brief Verifies set Secondary Sets High Class Bit And Always Clears Firing.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SetSecondarySetsHighClassBitAndAlwaysClearsFiring) {
  MakeLocalPlayer(5);
  Players[0].weapon[PW_SECONDARY].index = CONCUSSION_INDEX;
  ResetWeaponSelectStates(0x0000);

  // index 15 == NUM_SECONDARY_SLOTS + SECONDARY_INDEX -> high class -> set bit
  SetSecondaryWeapon(SECONDARY_INDEX + NUM_SECONDARY_SLOTS, 6);
  EXPECT_EQ(SavedSlotMask(), (uint16_t)(1 << 6));
  // ClearPlayerFiring fires unconditionally, then again inside the change branch
  EXPECT_EQ(s_clearfire_count, 2);
  EXPECT_EQ(s_hud_count, 1);

  // same index again: firing still cleared, but no duplicate message
  SetSecondaryWeapon(SECONDARY_INDEX + NUM_SECONDARY_SLOTS, 6);
  EXPECT_EQ(s_clearfire_count, 3);
  EXPECT_EQ(s_hud_count, 1);
}

/**
 * @test WeaponTest.SelectPrimaryNewSlotPicksLowClassWithoutMask
 * @brief Verifies select Primary New Slot Picks Low Class Without Mask.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SelectPrimaryNewSlotPicksLowClassWithoutMask) {
  MakeLocalPlayer(5);
  Players[0].weapon_flags = HAS_FLAG(LASER_INDEX) | HAS_FLAG(VAUSS_INDEX);
  Players[0].weapon[PW_PRIMARY].index = LASER_INDEX;
  ResetWeaponSelectStates(0x0000);

  SelectPrimaryWeapon(1);

  EXPECT_EQ(Players[0].weapon[PW_PRIMARY].index, VAUSS_INDEX);
  // the weapon changed, so the selection message fired
  EXPECT_EQ(s_hud_count, 1);
  EXPECT_EQ(s_play2d_last, SOUND_CHANGE_PRIMARY);
}

/**
 * @test WeaponTest.SelectPrimarySameSlotTogglesClassUp
 * @brief Verifies select Primary Same Slot Toggles Class Up.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SelectPrimarySameSlotTogglesClassUp) {
  MakeLocalPlayer(5);
  Players[0].weapon_flags = HAS_FLAG(LASER_INDEX) | HAS_FLAG(SUPER_LASER_INDEX);
  Players[0].weapon[PW_PRIMARY].index = LASER_INDEX; // slot 0
  ResetWeaponSelectStates(0x0000);

  SelectPrimaryWeapon(0);

  EXPECT_EQ(Players[0].weapon[PW_PRIMARY].index, SUPER_LASER_INDEX);
  EXPECT_EQ(SavedSlotMask() & (1 << 0), 1 << 0); // high-class bit recorded
}

/**
 * @test WeaponTest.SelectPrimaryWithoutWeaponGivesFeedback
 * @brief Verifies select Primary Without Weapon Gives Feedback.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SelectPrimaryWithoutWeaponGivesFeedback) {
  Players[0].weapon_flags = HAS_FLAG(LASER_INDEX);
  Players[0].weapon[PW_PRIMARY].index = LASER_INDEX;
  Players[0].objnum = 5;
  Objects[5].type = OBJ_PLAYER;
  ResetWeaponSelectStates(0x0000);

  SelectPrimaryWeapon(1);

  EXPECT_EQ(Players[0].weapon[PW_PRIMARY].index, LASER_INDEX);
  EXPECT_EQ(s_hud_count, 1);
  EXPECT_EQ(s_play2d_last, SOUND_DO_NOT_HAVE_IT);
  EXPECT_EQ(s_ainotify_count, 1);
}

/**
 * @test WeaponTest.SelectSecondarySameSlotRequiresAmmoForNextLevel
 * @brief Verifies select Secondary Same Slot Requires Ammo For Next Level.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SelectSecondarySameSlotRequiresAmmoForNextLevel) {
  MakeLocalPlayer(5);
  // cycling from concussion (slot 5) lands on FRAG_INDEX (10 + 5 % 10)
  Players[0].weapon_flags = HAS_FLAG(CONCUSSION_INDEX) | HAS_FLAG(FRAG_INDEX);
  Players[0].weapon[PW_SECONDARY].index = CONCUSSION_INDEX; // oldslot 5
  Players[0].weapon_ammo[FRAG_INDEX] = 0;
  ResetWeaponSelectStates(0x0000);

  // same slot without ammo for the next level -> feedback only
  SelectSecondaryWeapon(5);
  EXPECT_EQ(Players[0].weapon[PW_SECONDARY].index, CONCUSSION_INDEX);
  EXPECT_EQ(s_hud_count, 1);
  EXPECT_EQ(s_play2d_last, SOUND_DO_NOT_HAVE_IT);

  // now with ammo the cycle succeeds
  Players[0].weapon_ammo[FRAG_INDEX] = 4;
  s_hud_count = 0;
  SelectSecondaryWeapon(5);
  EXPECT_EQ(Players[0].weapon[PW_SECONDARY].index, FRAG_INDEX);
  EXPECT_EQ(s_hud_count, 1); // weapon-change message
}

/**
 * @test WeaponTest.SwitchPlayerWeaponCyclesToNextUsableSecondary
 * @brief Verifies switch Player Weapon Cycles To Next Usable Secondary.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SwitchPlayerWeaponCyclesToNextUsableSecondary) {
  MakeLocalPlayer(5);
  Players[0].weapon_flags =
      HAS_FLAG(CONCUSSION_INDEX) | HAS_FLAG(HOMING_INDEX) | HAS_FLAG(IMPACTMORTAR_INDEX);
  Players[0].weapon[PW_SECONDARY].index = CONCUSSION_INDEX;
  Players[0].weapon_ammo[HOMING_INDEX] = 5;
  Players[0].weapon_ammo[IMPACTMORTAR_INDEX] = 0;
  Ships[0].static_wb[HOMING_INDEX].ammo_usage = 1;
  Ships[0].static_wb[IMPACTMORTAR_INDEX].ammo_usage = 1;

  int ret = SwitchPlayerWeapon(PW_SECONDARY);

  EXPECT_EQ(ret, HOMING_INDEX);
  EXPECT_EQ(Players[0].weapon[PW_SECONDARY].index, HOMING_INDEX);
}

/**
 * @test WeaponTest.SwitchPlayerWeaponFallsBackToEnergyWeapon
 * @brief Verifies switch Player Weapon Falls Back To Energy Weapon.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, SwitchPlayerWeaponFallsBackToEnergyWeapon) {
  MakeLocalPlayer(5);
  Players[0].weapon_flags = HAS_FLAG(LASER_INDEX) | HAS_FLAG(VAUSS_INDEX);
  Players[0].weapon[PW_PRIMARY].index = LASER_INDEX;
  Players[0].energy = 10.0f;
  Ships[0].static_wb[LASER_INDEX].ammo_usage = 0;
  Ships[0].static_wb[LASER_INDEX].energy_usage = 1.0f;
  Ships[0].static_wb[VAUSS_INDEX].ammo_usage = 0;
  Ships[0].static_wb[VAUSS_INDEX].energy_usage = 2.0f;

  int ret = SwitchPlayerWeapon(PW_PRIMARY);

  EXPECT_EQ(ret, VAUSS_INDEX);
  EXPECT_EQ(Players[0].weapon[PW_PRIMARY].index, VAUSS_INDEX);
}

/**
 * @test WeaponTest.AutoSelectKeepsUsableCurrentOverNewWeapon
 * @brief Verifies auto Select Keeps Usable Current Over New Weapon.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, AutoSelectKeepsUsableCurrentOverNewWeapon) {
  MakeLocalPlayer(5);
  Players[0].weapon_flags = HAS_FLAG(LASER_INDEX) | HAS_FLAG(VAUSS_INDEX);
  Players[0].weapon[PW_PRIMARY].index = VAUSS_INDEX;
  Players[0].energy = 10.0f;
  Ships[0].static_wb[VAUSS_INDEX].ammo_usage = 0;
  Ships[0].static_wb[VAUSS_INDEX].energy_usage = 1.0f;

  bool switched = AutoSelectWeapon(PW_PRIMARY, LASER_INDEX);

  EXPECT_FALSE(switched);
  EXPECT_EQ(Players[0].weapon[PW_PRIMARY].index, VAUSS_INDEX);
}

/**
 * @test WeaponTest.AutoSelectWithoutNewPicksBestUsable
 * @brief Verifies auto Select Without New Picks Best Usable.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, AutoSelectWithoutNewPicksBestUsable) {
  MakeLocalPlayer(5);
  // Only vauss is owned; current index points at unowned laser.
  Players[0].weapon_flags = HAS_FLAG(VAUSS_INDEX);
  Players[0].weapon[PW_PRIMARY].index = LASER_INDEX;
  Players[0].energy = 10.0f;
  Ships[0].static_wb[VAUSS_INDEX].ammo_usage = 0;
  Ships[0].static_wb[VAUSS_INDEX].energy_usage = 1.0f;

  bool switched = AutoSelectWeapon(PW_PRIMARY);

  EXPECT_FALSE(switched); // sel_new_wpn only true when a new_wpn was passed
  EXPECT_EQ(Players[0].weapon[PW_PRIMARY].index, VAUSS_INDEX);
}

/**
 * @test WeaponTest.DrawAlphaBlendedScreenIssuesFullStateSequence
 * @brief Verifies draw Alpha Blended Screen Issues Full State Sequence.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, DrawAlphaBlendedScreenIssuesFullStateSequence) {
  DrawAlphaBlendedScreen(1.0f, 0.0f, 0.0f, 0.5f);

  ASSERT_GE(s_rend_calls.size(), 8u);
  EXPECT_EQ(s_rend_calls.front(), "zbuf:0");
  EXPECT_EQ(s_rend_calls.back(), "zbuf:1");
  EXPECT_NE(std::find(s_rend_calls.begin(), s_rend_calls.end(), std::string("tex:flat")),
            s_rend_calls.end());
  EXPECT_NE(std::find(s_rend_calls.begin(), s_rend_calls.end(),
                      std::string("alphaval:") + std::to_string((int)(uint8_t)(0.5f * 255))),
            s_rend_calls.end());
  bool drew = false;
  for (auto &c : s_rend_calls)
    if (c.rfind("drawpoly:", 0) == 0 && c == "drawpoly:4")
      drew = true;
  EXPECT_TRUE(drew);
}

/**
 * @test WeaponTest.GetWeaponFromIndexResolvesViaGunpointMasks
 * @brief Verifies get Weapon From Index Resolves Via Gunpoint Masks.
 *
 * @details
 * Exercises the WeaponTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon.cpp
 * @ingroup descent3_tests
 */
TEST_F(WeaponTest, GetWeaponFromIndexResolvesViaGunpointMasks) {
  object *pobj = MakeLocalPlayer(5);
  pobj->rtype.pobj_info.model_num = 7;
  pobj->dynamic_wb[1].cur_firing_mask = 0;

  // poly_model::poly_wb is a pointer; back it with real storage
  static poly_wb_info pwb_storage[2];
  memset(pwb_storage, 0, sizeof(pwb_storage));
  Poly_models[7].poly_wb = pwb_storage;
  pwb_storage[0].num_gps = 2;

  Ships[0].static_wb[1].gp_fire_masks[0] = 0x01;
  Ships[0].static_wb[1].gp_weapon_index[0] = 44;

  weapon *w = GetWeaponFromIndex(0, 1);
  EXPECT_EQ(w, &Weapons[44]);

  // no matching mask bits -> NULL
  Ships[0].static_wb[1].gp_fire_masks[0] = 0x00;
  EXPECT_EQ(GetWeaponFromIndex(0, 1), nullptr);
}
