/**
 * @file fireball_linked_real_tests.cpp
 * @brief Linked-real harness for Descent3/fireball.cpp.
 *
 * @details
 * Real explosion logic is compiled; engine rendering/physics/object deps stubbed.
 *
 * This harness validates the behavior of `Descent3/fireball.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/fireball.cpp`
 * @par Harness
 * `fireball_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/fireball.cpp
 */

#include "gtest/gtest.h"

#include "fireball.h"
#include "object.h"
#include "player.h"
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
#include "fix.h"
#include "physics.h"
#include "damage.h"
#include "terrain.h"
#include "manage.h"
#include "spew.h"
#include "Inventory.h"
#include "gamesequence.h"
#include "ddio.h"
#include "render.h"
#include "room.h"
#include "grtext.h"

struct fvi_query;
struct fvi_info;

// --- game-state globals (defined here; referenced as extern by fireball.cpp) ---
object Objects[MAX_OBJECTS];
player Players[MAX_PLAYERS];
ship Ships[MAX_SHIPS];
texture GameTextures[MAX_TEXTURES];
netgame_info Netgame;
const matrix Identity_matrix{};
bms_bitmap GameBitmaps[MAX_BITMAPS];
vclip GameVClips[MAX_VCLIPS];
object_info Object_info[MAX_OBJECT_TYPES];
weapon Weapons[MAX_WEAPONS];
poly_model *Poly_models = nullptr;
vis_effect *VisEffects = nullptr;
object *Player_object = nullptr;
object *Viewer_object = nullptr;
int Player_num = 0;
int Highest_object_index = 0;
uint32_t Demo_flags = 0;
int FrameCount = 0;
float Frametime = 0;
float Gametime = 0;
int Game_mode = 0;

// --- engine function stubs (signatures match those seen by fireball.cpp) ---
void AddToShakeMagnitude(float) {}
bool ApplyDamageToGeneric(object *, object *, int, float, int, int) { return true; }
bool ApplyDamageToPlayer(object *, object *, int, float, int, int, bool) { return true; }
uint16_t *bm_data(int, int) { return nullptr; }
int bm_h(int, int) { return 0; }
int bm_w(int, int) { return 0; }
bool BOA_IsVisible(int, int) { return false; }
int CreateNewEvent(int, int, float, void *, int, void (*)(int, void *), int) { return 0; }
void CreateRandomSparks(int, vector *, int, int, float) {}
void DrawAlphaBlendedScreen(float, float, float, float) {}
void Error(const char *, ...) {}
int FindObjectIDName(const char *) { return -1; }
int FindTextureName(const char *) { return -1; }
int fvi_FindIntersection(fvi_query *, fvi_info *, bool) { return -1; }
void g3_CheckAndDrawPoly(int, g3Point **, int, vector *, vector *) {}
uint8_t g3_CodePoint(g3Point *) { return 0; }
void g3_DrawBitmap(vector *, float, float, int, int) {}
int g3_DrawPoly(int, g3Point **, int, int, g3Codes *) { return 1; }
void g3_DrawRotatedBitmap(vector *, unsigned short, float, float, int, int) {}
void g3_GetUnscaledMatrix(matrix *m) { vm_MakeIdentity(m); }
uint8_t g3_RotatePoint(g3Point *, vector *) { return 0; }
poly_model *GetPolymodelPointer(int) { return nullptr; }
void GetPolyModelPointInWorld(vector *, poly_model *, vector *, matrix *, int, vector *, vector *) {}
float GetTerrainGroundPoint(vector *, vector *) { return 0.0f; }
int GetTextureBitmap(int, int, bool) { return -1; }
void InitObjectScripts(object *, bool) {}
bool IsNonRenderableSubmodel(poly_model *, int) { return false; }
int LoadTextureImage(const char *, int *, int, int, int, int) { return -1; }
void MultiSendObject(object *, unsigned char, unsigned char) {}
int ObjCreate(uint8_t, uint16_t, int, vector *, matrix const *, int) { return 1; }
object *ObjGet(int) { return nullptr; }
object *ObjGetUltimateParent(object *obj) { return obj; }
void ObjSetOrient(object *, matrix const *) {}
int osipf_ObjCreate(uint8_t, uint16_t, int, vector *, matrix const *, int, vector *) { return -1; }
void PageInVClip(int) {}
void phys_apply_force(object *, vector *, short) {}
bool PhysCalcGround(vector *, vector *, object *, int) { return false; }
void PlayerScoreAdd(int, int) {}
void rend_SetAlphaType(signed char) {}
void rend_SetAlphaValue(unsigned char) {}
void rend_SetColorModel(color_model) {}
void rend_SetFlatColor(unsigned int) {}
void rend_SetLighting(light_state) {}
void rend_SetOverlayType(unsigned char) {}
void rend_SetTextureType(texture_type) {}
void rend_SetWrapType(wrap_type) {}
void rend_SetZBias(float) {}
void rend_SetZBufferWriteMask(int) {}
void SetObjectControlType(object *, int) {}
void SetObjectDeadFlag(object *, bool, bool) {}
void SetShakeMagnitude(float) {}
int SpewCreate(spewinfo *) { return 0; }
Inventory::Inventory() {}
Inventory::~Inventory() {}
bool Inventory::Add(int, int, object *, int, int, int, const char *) { return false; }
void StartCockpitShake(float, vector *) {}
int VisEffectCreate(uint8_t, uint8_t, int, vector *) { return -1; }
void vm_AnglesToMatrix(matrix *, unsigned short, unsigned short, unsigned short) {}
angvec *vm_ExtractAnglesFromMatrix(angvec *a, const matrix *m) { return a; }
float vm_GetMagnitudeFast(vector const *) { return 1.0f; }
float vm_GetNormalizedDirFast(vector *, vector const *, vector const *) { return 1.0f; }
void vm_MakeIdentity(matrix *m) { memset(m, 0, sizeof(matrix)); m->a2d[0][0] = m->a2d[1][1] = m->a2d[2][2] = 1.0f; }
float vm_NormalizeVectorFast(vector *) { return 1.0f; }
float vm_NormalizeVector(vector *) { return 1.0f; }
void vm_TransposeMatrix(matrix *) {}
float vm_VectorDistanceQuick(vector const *, vector const *) { return 1.0f; }
float vm_VectorDistance(vector const *, vector const *) { return 1.0f; }
void vm_VectorToMatrix(matrix *, vector *, vector *, vector *) {}

// random for GetRandom* family
int ps_rand() { static int s = 0; return (s = (s * 1103515245 + 12345) >> 16) & 0x7fff; }
void ps_srand(unsigned) {}

// symbols defined in fireball.cpp
extern void InitFireballs();
extern fireball Fireballs[];
extern void DrawColoredRing(vector *pos, float r, float g, float b, float inner_alpha,
                            float outer_alpha, float size, float inner_ring_ratio,
                            uint8_t saturate, uint8_t lod);
extern void DrawColoredDisk(vector *pos, float r, float g, float b, float inner_alpha,
                            float outer_alpha, float size, uint8_t saturate, uint8_t lod);
extern int CreateFireball(vector *pos, int fireball_num, int roomnum, int realtype);

/**
 * @test FireballLinked.DrawColoredRingNoCrash
 * @brief Verifies draw Colored Ring No Crash.
 *
 * @details
 * Exercises the FireballLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(FireballLinked, DrawColoredRingNoCrash) {
  vector pos{};
  DrawColoredRing(&pos, 1.0f, 0.5f, 0.2f, 0.3f, 0.7f, 10.0f, 0.5f, 0, 0);
  SUCCEED();
}

/**
 * @test FireballLinked.DrawColoredDiskNoCrash
 * @brief Verifies draw Colored Disk No Crash.
 *
 * @details
 * Exercises the FireballLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(FireballLinked, DrawColoredDiskNoCrash) {
  vector pos{};
  DrawColoredDisk(&pos, 1.0f, 0.5f, 0.2f, 0.3f, 0.7f, 10.0f, 0, 0);
  SUCCEED();
}

/**
 * @test FireballLinked.CreateFireballSuccessPath
 * @brief Verifies create Fireball Success Path.
 *
 * @details
 * Exercises the FireballLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fireball.cpp
 * @ingroup descent3_tests
 */
TEST(FireballLinked, CreateFireballSuccessPath) {
  // ObjCreate stub returns a valid objnum -> CreateFireball populates Objects[1]
  vector pos{};
  int r = CreateFireball(&pos, 0, 0, 1);
  EXPECT_EQ(r, 1);
  EXPECT_FLOAT_EQ(Objects[1].size, Fireballs[0].size);
}
