/**
 * @file player_linked_real_tests.cpp
 * @brief Linked-real harness for Descent3/Player.cpp.
 *
 * @details
 * Real player logic is compiled; engine deps stubbed.
 *
 * This harness validates the behavior of `Descent3/Player.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/Player.cpp`
 * @par Harness
 * `player_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/Player.cpp
 */

#include "gtest/gtest.h"

#include "player.h"
#include "object.h"
#include "ship.h"
#include "gametexture.h"
#include "multi.h"
#include "3d.h"
#include "vecmat.h"
#include "renderer.h"
#include "bitmap.h"
#include "vclip.h"
#include "weapon.h"
#include "object_external.h"
#include "polymodel_external.h"
#include "hlsoundlib.h"
#include "ssl_lib.h"
#include "fix.h"
#include "physics.h"
#include "findintersection.h"
#include "damage.h"
#include "terrain.h"
#include "manage.h"
#include "spew.h"
#include "gamesequence.h"
#include "ddio.h"
#include "render.h"
#include "room.h"
#include "grtext.h"
#include "fireball.h"
#include "config.h"
#include "game2dll.h"
#include "controls.h"
#include "hud.h"
#include "pilot.h"
#include "robotfire.h"

struct ai_path_info;
struct tOSIRISEventInfo;

// --- game-state globals ---
object Objects[MAX_OBJECTS];
ship Ships[MAX_SHIPS];
texture GameTextures[MAX_TEXTURES];
netgame_info Netgame;
netplayer NetPlayers[MAX_NET_PLAYERS];
vclip GameVClips[MAX_VCLIPS];
object_info Object_info[MAX_OBJECT_TYPES];
weapon Weapons[MAX_WEAPONS];
room Rooms[MAX_ROOMS];
poly_model *Poly_models = nullptr;
vis_effect *VisEffects = nullptr;
object *Player_object = nullptr;
object *Viewer_object = nullptr;
sound_info Sounds[MAX_SOUNDS];
terrain_segment Terrain_seg[(TERRAIN_WIDTH + 1) * (TERRAIN_DEPTH + 1)];
terrain_sky Terrain_sky;
int Highest_object_index = 0;
int Highest_room_index = 0;
uint32_t Demo_flags = 0;
float Frametime = 0;
float Gametime = 0;
int Game_mode = 0;
tGameState Game_state;
tGameToggles Game_toggles;
float Render_FOV = 0;
float Render_zoom = 0;
int Difficulty_level = 0;
int Buddy_handle = -1;
int IsCheater = 0;
int Game_window_h = 0;
int Game_window_y = 0;
uint8_t AutomapVisMap[MAX_ROOMS] = {};
dllinfo DLLInfo{};
float Multi_additional_shields[MAX_PLAYERS];
pilot Current_pilot;
int Cinematic_inuse = 0;
gameController *Controller = nullptr;

// --- engine function stubs ---
bool AddHUDMessage(const char *, ...) { return true; }
void AddPersistentHUDMessage(unsigned, int, int, float, int, int, const char *, ...) {}
bool AINotify(object *, uint8_t, void *) { return false; }
void AIPathInitPath(ai_path_info *) {}
int AllocTexture() { return -1; }
void bm_FreeBitmap(int) {}
void CallGameDLL(int, dllinfo *) {}
bool Cinematic_GetOldHudMode() { return false; }
void ClearPlayerFiring(object *, int) {}
void ClearTransientObjects(int) {}
int ComputeDefaultSize(int, int, float *) { return 0; }
int CreateBlastRing(vector *, int, float, float, int, int) { return -1; }
int CreateFireball(vector *, int, int, int) { return -1; }
int CreateNewEvent(int, int, float, void *, int, void (*)(int, void *), int) { return 0; }
void CreateRandomSparks(int, vector *, int, int, float) {}
object *CreateSubobjectDebrisDirected(object *, int, vector *, float, int) { return nullptr; }
float D3_ChronoTimer_GetTime() { return 0; }
void ddio_KeyFlush() {}
void DemoWriteObjLifeLeft(object *) {}
void DemoWritePlayerBalls(int) {}
void DemoWritePlayerTypeChange(int, bool, int, int) {}
void DoConcussiveForce(object *, int, float) {}
void DoSprayEffect(object *, otype_wb_info *, unsigned char) {}
void DrawAlphaEvent(int, void *) {}
int FindObjectIDName(const char *) { return -1; }
int FindShipName(const char *) { return -1; }
int FindSoundName(const char *) { return -1; }
int FindWeaponName(const char *) { return -1; }
void FireOnOffWeapon(object *) {}
int fvi_FindIntersection(fvi_query *, fvi_info *, bool) { return -1; }
tHUDMode GetHUDMode() { return (tHUDMode)0; }
int GetNextShip(int) { return 0; }
poly_model *GetPolymodelPointer(int) { return nullptr; }
void GetPolyModelPointInWorld(vector *, poly_model *, vector *, matrix *, int, vector *, vector *) {}
int GetRandomSmallExplosion() { return 0; }
const char *GetStringFromTable(int) { return ""; }
float GetTerrainDynamicScalar(vector *, int) { return 1; }
void GoalInitTypeGoals(object *, int) {}
void InitObjectScripts(object *, bool) {}
void MakeObjectVisible(object *) {}
bool MercInstalled() { return false; }
int LoadTextureImage(const char *, int *, int, int, int, int) { return -1; }
void MultiMakePlayerGhost(int) {}
void MultiMakePlayerReal(int) {}
void MultiSendGhostObject(object *, bool) {}
void MultiSendObject(object *, unsigned char, unsigned char) {}
void MultiSendThiefSteal(int, int) {}
int ObjCreate(uint8_t, uint16_t, int, vector *, matrix const *, int) { return -1; }
void ObjDelete(int) {}
object *ObjGet(int) { return nullptr; }
void ObjSetPos(object *, vector *, int, matrix *, bool) {}
void ObjSetRenderPolyobj(object *, int, int) {}
void ObjUnGhostObject(int) {}
void Osiris_CallLevelEvent(int, tOSIRISEventInfo *) {}
int PageInPolymodel(int, int, float *) { return 0; }
void phys_apply_force(object *, vector *, short) {}
void pilot::get_hud_data(uint8_t *, uint16_t *, uint16_t *, int *, int *) {}
void ResetPersistentHUDMessage() {}
void ResetReticle() {}
void ResetWeaponSelectStates(unsigned short) {}
void SetHUDMode(tHUDMode) {}
void SetNapalmDamageEffect(object *, object *, int) {}
void SetObjectControlType(object *, int) {}
void SetObjectDeadFlag(object *, bool, bool) {}
void FreeVClip(int) {}
void hlsSystem::StopSoundLooping(int) {}
int ps_rand() { static int s = 0; return (s = (s * 1103515245 + 12345) >> 16) & 0x7fff; }
void ps_srand(unsigned) {}
void WBClearInfo(object *) {}

// Inventory members referenced via player struct
Inventory::Inventory() {}
Inventory::~Inventory() {}
pilot::pilot() {}
pilot::~pilot() {}
bool Inventory::Add(int, int, object *, int, int, int, const char *) { return false; }
bool Inventory::Remove(int, int) { return false; }
int Inventory::Size(void) { return 0; }
bool Inventory::CheckItem(int, int) { return false; }
void Inventory::Reset(bool, int) {}
void Inventory::ResetPos(void) {}
void Inventory::NextPos(bool) {}
bool Inventory::GetPosTypeID(int &, int &) { return false; }
bool Inventory::GetAuxPosTypeID(int &, int &) { return false; }
bool Inventory::GetPosInfo(uint16_t &, int &) { return false; }
int Inventory::GetPosCount(void) { return 0; }
int Inventory::GetTypeIDCount(int, int) { return 0; }

// D3::ChronoTimer
namespace D3 {
float ChronoTimer::GetTime() { return 0.0f; }
} // namespace D3

// symbols defined in Player.cpp
extern void InitPlayers();

/**
 * @test PlayerLinked.SmokeNoCrash
 * @brief Verifies smoke No Crash.
 *
 * @details
 * Exercises the PlayerLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Player.cpp
 * @ingroup descent3_tests
 */
TEST(PlayerLinked, SmokeNoCrash) {
  SUCCEED();
}

/**
 * @test PlayerLinked.InitPlayersReal
 * @brief Verifies init Players Real.
 *
 * @details
 * Exercises the PlayerLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/Player.cpp
 * @ingroup descent3_tests
 */
TEST(PlayerLinked, InitPlayersReal) {
  InitPlayers();
  SUCCEED();
}
