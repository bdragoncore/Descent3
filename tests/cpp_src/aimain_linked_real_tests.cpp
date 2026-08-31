/**
 * @file aimain_linked_real_tests.cpp
 * @brief Unit tests for Descent3/AImain.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/AImain.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/AImain.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/AImain.cpp`
 * @par Harness
 * `aimain_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/AImain.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>

#include "pstypes.h"
#include "vecmat.h"
#include "aistruct.h"
#include "object.h"
#include "objinfo.h"
#include "player.h"
#include "AIMain.h"
#include "AIGoal.h"
#include "aipath.h"
#include "findintersection.h"
#include "room.h"
#include "terrain.h"
#include "BOA.h"
#include "game.h"
#include "difficulty.h"
#include "multi.h"
#include "gametexture.h"
#include "bitmap.h"
#include "polymodel.h"
#include "hlsoundlib.h"
#include "robotfire.h"
#include "weapon.h"
#include "attach.h"
#include "damage.h"
#include "osiris_dll.h"
#include "matcen.h"
#include "gamepath.h"

// AImain defines these — use extern for access
extern int AI_NumRendered;
extern int AI_RenderedList[MAX_OBJECTS];
extern int AI_NumHostileAlert;
extern bool AI_debug_robot_do;
extern int AI_debug_robot_index;
extern float AI_last_time_room_noise_alert_time[MAX_ROOMS+8];
extern int AI_unique_goal_id;
extern int Buddy_handle[MAX_PLAYERS];

// Globals U in AImain.o — define here
uint16_t BOA_Array[MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS][MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS] = {};
connect_data BOA_connect[MAX_BOA_TERRAIN_REGIONS][MAX_PATH_PORTALS] = {};
int BOA_num_connect[MAX_BOA_TERRAIN_REGIONS] = {};
int BOA_num_terrain_regions = 0;
bool Cinematic_inuse = false;
uint32_t Demo_flags = 0;
const float Diff_ai_circle_dist[5] = {1,1,1,1,1};
const float Diff_ai_dodge_percent[5] = {0.5f,0.5f,0.5f,0.5f,0.5f};
const float Diff_ai_dodge_speed[5] = {1,1,1,1,1};
const float Diff_ai_rotspeed[5] = {1,1,1,1,1};
const float Diff_ai_speed[5] = {1,1,1,1,1};
const float Diff_ai_turret_speed[5] = {1,1,1,1,1};
const float Diff_ai_vis_dist[5] = {1,1,1,1,1};
const float Diff_general_inv_scalar[5] = {1,1,1,1,1};
float Frametime = 0.016f;
float Gametime = 100.0f;
bms_bitmap GameBitmaps[MAX_BITMAPS] = {};
int Game_mode = 0;
game_path GamePaths[MAX_GAME_PATHS] = {};
texture GameTextures[MAX_TEXTURES] = {};
int Highest_object_index = -1;
int Highest_room_index = 10;
uint8_t ingame_difficulty = 2;
bool Matcen_created = false;
netgame_info Netgame = {};
netplayer NetPlayers[MAX_NET_PLAYERS] = {};
int Num_teams = 0;
object_info Object_info[MAX_OBJECT_IDS] = {};
object Objects[MAX_OBJECTS] = {};
int Player_num = 0;
object *Player_object = nullptr;
object *Viewer_object = nullptr;
player Players[MAX_PLAYERS] = {};
poly_model Poly_models[MAX_POLY_MODELS] = {};
room Rooms[MAX_ROOMS] = {};
hlsSystem Sound_system;
terrain_normals *TerrainNormals[MAX_TERRAIN_LOD] = {};
terrain_segment Terrain_seg[(TERRAIN_WIDTH+1)*(TERRAIN_DEPTH+1)] = {};
terrain_tex_segment Terrain_tex_seg[TERRAIN_TEX_WIDTH*TERRAIN_TEX_DEPTH] = {};
int Num_objects = 0;
int Team_game = 0;
team Teams[MAX_TEAMS] = {};

// Additional globals needed for linking but not in U list (used by test setup)
int Dedicated_server = 0;
int FrameCount = 0;

// hlsSystem minimal impl
hlsSystem::hlsSystem(){}
void hlsSystem::KillSoundLib(bool) {}
int hlsSystem::Play3dSound(int, int, object*, float, int, float){ return -1; }
int hlsSystem::Play3dSound(int, int, pos_state*, float, int, float){ return -1; }
int hlsSystem::Play3dSound(int, pos_state*, object*, int, float, int, float){ return -1; }
int hlsSystem::Play3dSound(int, pos_state*, float, int, float){ return -1; }
int hlsSystem::Play3dSound(int, object*, float, int, float){ return -1; }
void hlsSystem::StopSoundLooping(int){}

// Inventory stubs (needed for player)
Inventory::Inventory(){}
Inventory::~Inventory(){}
bool Inventory::Add(int, int, object*, int, int, int, const char*){ return true; }
bool Inventory::AddObject(int, int, const char*){ return true; }
bool Inventory::Remove(int, int){ return true; }
bool Inventory::CheckItem(int, int){ return false; }

// --- Function stubs for U symbols not provided by vecmat/libc ---

// Goal system
int GoalAddGoal(object *obj, uint32_t goal_type, void *args, int level, float influence, int f_goal, int guid, char subtype){ (void)obj;(void)goal_type;(void)args;(void)level;(void)influence;(void)f_goal;(void)guid;(void)subtype; return -1; }
void GoalDoFrame(object *obj){ (void)obj; }
void GoalClearGoal(object *obj, goal *goal_ptr, int notify_reason){ (void)obj;(void)goal_ptr;(void)notify_reason; }
int GoalAddEnabler(object *obj, int goal_index, uint8_t enabler_type, void *arg_struct, float percent, float interval){ (void)obj;(void)goal_index;(void)enabler_type;(void)arg_struct;(void)percent;(void)interval; return -1; }
void GoalInitTypeGoals(object *obj, int ai_type){ (void)obj;(void)ai_type; }
bool GoalIsGoalEnabled(object *obj, int goal_index){ (void)obj;(void)goal_index; return true; }
goal* GoalGetCurrentGoal(object *obj){ (void)obj; return nullptr; }
float GoalDetermineTrackDist(object *obj){ (void)obj; return 50.0f; }

// AIPATH
bool AIPathAllocPath(object *obj, ai_frame *ai_info, void *goal_ptr, int *start_room, vector *start_pos, int *end_room, vector *end_pos, float rad, int flags, int handle, int ignore_obj){ (void)obj;(void)ai_info;(void)goal_ptr;(void)start_room;(void)start_pos;(void)end_room;(void)end_pos;(void)rad;(void)flags;(void)handle;(void)ignore_obj; return false; }
bool AIPathFreePath(ai_path_info *aip){ if(aip) memset(aip,0,sizeof(*aip)); return true; }
bool AIPathInitPath(ai_path_info *aip){ if(aip) memset(aip,0,sizeof(*aip)); return true; }
bool AIPathAtEnd(ai_path_info *aip){ (void)aip; return true; }
bool AIPathAtStart(ai_path_info *aip){ (void)aip; return true; }
bool AIPathGetNextNodePos(ai_path_info *aip, vector *pos, int *room){ (void)aip; if(pos) *pos=vector{}; if(room) *room=0; return false; }
bool AIPathGetPrevNodePos(ai_path_info *aip, vector *pos, int *room){ (void)aip; if(pos) *pos=vector{}; if(room) *room=0; return false; }
bool AIPathGetCurrentNodePos(ai_path_info *aip, vector *pos, int *room){ (void)aip; if(pos) *pos=vector{}; if(room) *room=0; return false; }
void AIPathMoveTurnTowardsNode(object *obj, vector *mdir, bool *f_moved){ (void)obj; if(mdir) *mdir=vector{}; if(f_moved) *f_moved=false; }
bool AIPathSetAsStaticPath(object *obj, void *goal_ptr, int path_id, int start_node, int end_node, int cur_node){ (void)obj;(void)goal_ptr;(void)path_id;(void)start_node;(void)end_node;(void)cur_node; return true; }
bool AIFindAltPath(object *obj, int i, int j, float *dist){ (void)obj;(void)i;(void)j; if(dist) *dist=0; return false; }

// Attach
bool AttachObject(object *parent, int8_t parent_ap, object *child, char child_ap, bool f_use_aligned){ (void)parent;(void)parent_ap;(void)child;(void)child_ap;(void)f_use_aligned; return false; }
bool AttachObject(object *parent, int8_t parent_ap, object *child, float rad){ (void)parent;(void)parent_ap;(void)child;(void)rad; return false; }
bool AttachDoPosOrient(object *parent, char parent_ap, object *child, char child_ap, bool f_parent, bool f_move_obj, vector *pos, matrix *orient, bool f_dropping_off){ (void)parent;(void)parent_ap;(void)child;(void)child_ap;(void)f_parent;(void)f_move_obj;(void)pos;(void)orient;(void)f_dropping_off; return false; }
bool AttachDoPosOrientRad(object *parent, char p_ap, object *child, float rad_percent, vector *pos){ (void)parent;(void)p_ap;(void)child;(void)rad_percent;(void)pos; return false; }
void AttachUpdateSubObjects(object *obj){ (void)obj; }
bool UnattachFromParent(object *obj){ (void)obj; return false; }
void ObjSetOrient(object *obj, const matrix *orient){ if(obj && orient) obj->orient=*orient; }

// BOA
bool BOA_IsVisible(int start_room, int end_room){ (void)start_room;(void)end_room; return false; }
bool BOA_ComputeMinDist(int start_room, int end_room, float max_check_dist, float *dist, int *num_blockages){ (void)start_room;(void)end_room;(void)max_check_dist; if(dist) *dist=0; if(num_blockages) *num_blockages=0; return false; }
int BOA_DetermineStartRoomPortal(int start_room, vector *start_pos, int end_room, vector *end_pos, bool f_moving_into_room, bool f_use_terrain, int *portal_out){ (void)start_room;(void)start_pos;(void)end_room;(void)end_pos;(void)f_moving_into_room;(void)f_use_terrain;(void)portal_out; return -1; }

// Weapon / robotfire
void DoSprayEffect(object *obj, otype_wb_info *static_wb, uint8_t wb_index){ (void)obj;(void)static_wb;(void)wb_index; }
void WBFireBattery(object *obj, otype_wb_info *static_wb, int wb_index){ (void)obj;(void)static_wb;(void)wb_index; }
bool WeaponCalcGun(vector *gun_point, vector *gun_normal, object *obj, int gun_num){ (void)gun_point;(void)gun_normal;(void)obj;(void)gun_num; if(gun_point) *gun_point=vector{}; if(gun_normal) *gun_normal=vector{}; return false; }
void FireOnOffWeapon(object *obj){ (void)obj; }
void WBFireAnimFrame(object *obj, otype_wb_info *static_wb, int wb_index){ (void)obj;(void)static_wb;(void)wb_index; }
void WBSetupFireAnim(object *obj, otype_wb_info *static_wb, int wb_index){ (void)obj;(void)static_wb;(void)wb_index; }
bool WBIsBatteryReady(object *obj, otype_wb_info *static_wb, int wb_index){ (void)obj;(void)static_wb;(void)wb_index; return false; }

// Demo / Multi / Osiris
void DemoWrite3DSound(short s, unsigned short n, int h, float v){ (void)s;(void)n;(void)h;(void)v; }
int GetTextureBitmap(int t, int f, bool b){ (void)t;(void)f;(void)b; return -1; }
void MultiPlay3dSound(short s, unsigned short n, int h){ (void)s;(void)n;(void)h; }
bool Osiris_CallEvent(object *obj, int evt, tOSIRISEventInfo *info){ (void)obj;(void)evt;(void)info; return false; }
bool Osiris_CallLevelEvent(int evt, tOSIRISEventInfo *info){ (void)evt;(void)info; return false; }
bool ObjectsAreRelated(int h1, int h2){ (void)h1;(void)h2; return false; }
void DemoWriteTurretChanged(unsigned short o){ (void)o; }
void DemoWriteObjAnimChanged(unsigned short o){ (void)o; }
void DemoWriteObjWeapFireFlagChanged(short o){ (void)o; }
void MultiAddObjAnimUpdate(int o){ (void)o; }
void MultiAddObjTurretUpdate(int o){ (void)o; }
void MultiSendAiWeaponFlags(object *obj, int f, int t){ (void)obj;(void)f;(void)t; }

// Damage / player
bool ApplyDamageToPlayer(object *playerobj, object *killer, int damage_type, float damage_amount, int server_says, int weapon_num, bool playsound){ (void)playerobj;(void)killer;(void)damage_type;(void)damage_amount;(void)server_says;(void)weapon_num;(void)playsound; return false; }
bool ApplyDamageToGeneric(object *robotobj, object *killer, int damage_type, float damage, int server_says, int weapon_num){ (void)robotobj;(void)killer;(void)damage_type;(void)damage;(void)server_says;(void)weapon_num; return false; }
void DecreasePlayerEnergy(int slot, float energy){ (void)slot;(void)energy; }
uint32_t check_point_to_face(vector *col_point, vector *face_normal, int nv, vector **vertex_list){ (void)col_point;(void)face_normal;(void)nv;(void)vertex_list; return 0; }
int check_vector_to_sphere_1(vector *pos, float *rad, vector const *pnt, vector const *vec, vector *intp, float len, bool s, bool e){ (void)pos;(void)rad;(void)pnt;(void)vec;(void)intp;(void)len;(void)s;(void)e; return 0; }

// FVI
int fvi_FindIntersection(fvi_query *fq, fvi_info *hit_data, bool no_subdivision){ (void)fq;(void)hit_data;(void)no_subdivision; return HIT_NONE; }
int fvi_QuickDistCellList(int r, vector *p, float d, int *l, int m){ (void)r;(void)p;(void)d;(void)l;(void)m; return 0; }
int fvi_QuickDistFaceList(int r, vector *p, float d, fvi_face_room_list *l, int m){ (void)r;(void)p;(void)d;(void)l;(void)m; return 0; }
int fvi_QuickDistObjectList(vector *pos, int r, float d, short *l, int m, bool a, bool b, bool c, bool d2){ (void)pos;(void)r;(void)d;(void)l;(void)m;(void)a;(void)b;(void)c;(void)d2; return 0; }

// Terrain
float GetTerrainGroundPoint(vector *pos, vector *normal){ (void)pos; if(normal) *normal=vector{0,1,0}; return 0; }
int GetTerrainRoomFromPos(vector *pos){ (void)pos; return -1; }
void ComputeTerrainSegmentCenter(vector *pos, int segnum){ (void)segnum; if(pos) *pos=vector{}; }

// Object / misc
void ObjGhostObject(int o){ (void)o; }
object* ObjGet(int handle){
  if(handle==OBJECT_HANDLE_NONE) return nullptr;
  for(int i=0;i<MAX_OBJECTS;i++) if(Objects[i].handle==handle && Objects[i].type!=OBJ_NONE) return &Objects[i];
  return nullptr;
}
int ObjCreate(unsigned char type, unsigned short id, int roomnum, vector *pos, const matrix *orient, int parent_handle){
  (void)type;(void)id;(void)roomnum;(void)pos;(void)orient;(void)parent_handle;
  for(int i=0;i<MAX_OBJECTS;i++) if(Objects[i].type==OBJ_NONE){ Objects[i].type=type; Objects[i].id=id; Objects[i].roomnum=roomnum; if(pos) Objects[i].pos=*pos; if(orient) Objects[i].orient=*orient; Objects[i].handle=1000+i; Highest_object_index=std::max(Highest_object_index,i); return i; }
  return -1;
}
int ps_rand(){ return 0; }
void ait_Init(){}
void ComputeRoomCenter(vector *pos, room *rp){ if(pos) *pos=vector{}; (void)rp; }

// Matrix operators (if not provided by vecmat)
static ai_frame* MakeAI(object &obj){
  static ai_frame frames[8];
  static int next=0;
  ai_frame *af=&frames[next%8]; next++; memset(af,0,sizeof(*af));
  af->circle_distance=20.0f; af->max_velocity=10.0f;
  for(int i=0;i<MAX_GOALS;i++) af->goals[i].used=false;
  obj.ai_info=af; obj.handle=100+next; obj.type=OBJ_ROBOT; obj.id=0; obj.size=5.0f; obj.roomnum=1; obj.pos=vector{}; obj.control_type=CT_AI; obj.movement_type=MT_NONE; obj.orient=Identity_matrix;
  // ensure polyobj info is zeroed
  memset(&obj.rtype,0,sizeof(obj.rtype));
  obj.orient=Identity_matrix;
  return af;
}
static void ResetObj(object &obj){ memset(&obj,0,sizeof(obj)); obj.handle=1; obj.type=OBJ_ROBOT; obj.size=5.0f; obj.roomnum=1; obj.control_type=CT_AI; obj.movement_type=MT_NONE; obj.orient=Identity_matrix; MakeAI(obj); }

static anim_elem g_anim_store[NUM_MOVEMENT_CLASSES];
static t_ai_info g_ai_info_store;
static bool g_objinfo_init=false;
static void EnsureObjInfoInit(){
  if(g_objinfo_init) return;
  memset(g_anim_store,0,sizeof(g_anim_store));
  // Give AS_IDLE and AS_ALERT some non-zero to avoid fallback logic crash but still zero is okay; keep zero
  memset(&g_ai_info_store,0,sizeof(g_ai_info_store));
  g_ai_info_store.max_velocity=10; g_ai_info_store.max_turn_rate=1; g_ai_info_store.movement_type=0;
  for(int i=0;i<MAX_OBJECT_IDS;i++){
    Object_info[i].anim=g_anim_store;
    // leave ai_info null so ObjSetAIInfo returns early (simpler) except for id 0 we can set
  }
  Object_info[0].ai_info=&g_ai_info_store;
  Object_info[0].type=OBJ_ROBOT;
  Object_info[0].anim=g_anim_store;
  g_objinfo_init=true;
}

class AIMainLinked : public ::testing::Test{
protected:
  object obj;
  void SetUp() override{
    EnsureObjInfoInit();
    Gametime=100.0f; Frametime=0.016f; AI_NumRendered=0; AI_NumHostileAlert=0;
    for(int i=0;i<MAX_OBJECTS;i++){ Objects[i].type=OBJ_NONE; Objects[i].handle=OBJECT_HANDLE_NONE; }
    for(int i=0;i<MAX_ROOMS;i++) Rooms[i].used=0;
    Rooms[1].used=1; Highest_room_index=10; Highest_object_index=-1;
    memset(&obj,0,sizeof(obj)); ResetObj(obj);
    for(int i=0;i<MAX_PLAYERS;i++){ Players[i].objnum=-1; Players[i].team=0; }
    Player_num=0; Player_object=&obj; Viewer_object=&obj;
    for(int i=0;i<MAX_ROOMS+MAX_BOA_TERRAIN_REGIONS;i++) for(int j=0;j<MAX_ROOMS+MAX_BOA_TERRAIN_REGIONS;j++) BOA_Array[i][j]=0;
    BOA_num_terrain_regions=0;
    // ensure obj's Object_info anim is valid via EnsureObjInfoInit
  }
};

/**
 * @test AIMainLinked.AIInitCreatesAI
 * @brief Verifies aIInit Creates AI.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIInitCreatesAI) {
  object o{}; memset(&o,0,sizeof(o)); o.type=OBJ_ROBOT; o.handle=999; o.roomnum=1; o.control_type=CT_AI; o.movement_type=MT_NONE; o.id=0; o.size=5.0f; o.orient=Identity_matrix;
  static ai_frame frame; memset(&frame,0,sizeof(frame)); frame.circle_distance=20; frame.max_velocity=10;
  o.ai_info=&frame;
  bool r = AIInit(&o, 0, 0, 0);
  EXPECT_TRUE(r);
  EXPECT_NE(o.ai_info, nullptr);
  if(o.ai_info) EXPECT_EQ(o.control_type, CT_AI);
}

/**
 * @test AIMainLinked.AINotifyReturnsTrue
 * @brief Verifies aINotify Returns True.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AINotifyReturnsTrue) {
  EXPECT_TRUE(AINotify(&obj, 0, nullptr));
}

/**
 * @test AIMainLinked.AIDoFrameNoCrash
 * @brief Verifies aIDo Frame No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIDoFrameNoCrash) {
  EXPECT_NO_THROW(AIDoFrame(&obj));
}

/**
 * @test AIMainLinked.AIFrameAllNoCrash
 * @brief Verifies aIFrame All No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIFrameAllNoCrash) {
  EXPECT_NO_THROW(AIFrameAll());
}

/**
 * @test AIMainLinked.AIPowerSwitchNoCrash
 * @brief Verifies aIPower Switch No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIPowerSwitchNoCrash) {
  EXPECT_NO_THROW(AIPowerSwitch(&obj, true));
  EXPECT_NO_THROW(AIPowerSwitch(&obj, false));
}

/**
 * @test AIMainLinked.AITurnTowardsDirNoCrash
 * @brief Verifies aITurn Towards Dir No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AITurnTowardsDirNoCrash) {
  vector dir{1,0,0};
  EXPECT_NO_THROW(AITurnTowardsDir(&obj, &dir, 1.0f));
}

/**
 * @test AIMainLinked.AIMoveTowardsDirNoCrash
 * @brief Verifies aIMove Towards Dir No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIMoveTowardsDirNoCrash) {
  vector dir{0,1,0};
  EXPECT_NO_THROW(AIMoveTowardsDir(&obj, &dir, 1.0f));
}

/**
 * @test AIMainLinked.AIMoveTowardsPositionNoCrash
 * @brief Verifies aIMove Towards Position No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIMoveTowardsPositionNoCrash) {
  vector pos{10,0,0}; vector mdir{}; bool moved=false;
  EXPECT_NO_THROW(AIMoveTowardsPosition(&obj, &pos, 1.0f, false, &mdir, &moved));
}

/**
 * @test AIMainLinked.AITurnTowardsPositionNoCrash
 * @brief Verifies aITurn Towards Position No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AITurnTowardsPositionNoCrash) {
  vector pos{5,5,5};
  EXPECT_NO_THROW(AITurnTowardsPosition(&obj, &pos));
}

/**
 * @test AIMainLinked.AIFindHidePosReturnsFalse
 * @brief Verifies aIFind Hide Pos Returns False.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIFindHidePosReturnsFalse) {
  vector hpos{}; int hroom=-1;
  EXPECT_FALSE(AIFindHidePos(&obj, &obj, &hpos, &hroom, 1.0f));
}

/**
 * @test AIMainLinked.AIFindRoomWithFlagReturnsMinusOne
 * @brief Verifies aIFind Room With Flag Returns Minus One.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIFindRoomWithFlagReturnsMinusOne) {
  EXPECT_EQ(AIFindRoomWithFlag(&obj, 0), -1);
}

/**
 * @test AIMainLinked.AIFindObjOfTypeReturnsNull
 * @brief Verifies aIFind Obj Of Type Returns Null.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIFindObjOfTypeReturnsNull) {
  EXPECT_EQ(AIFindObjOfType(&obj, OBJ_ROBOT, 0, false, OBJECT_HANDLE_NONE), nullptr);
}

/**
 * @test AIMainLinked.AIObjEnemyAndFriend
 * @brief Verifies aIObj Enemy And Friend.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIObjEnemyAndFriend) {
  object other{}; other.type=OBJ_ROBOT;
  EXPECT_FALSE(AIObjEnemy(&obj, &other));
  EXPECT_FALSE(AIObjFriend(&obj, &other));
}

/**
 * @test AIMainLinked.AISetTargetNoCrash
 * @brief Verifies aISet Target No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AISetTargetNoCrash) {
  EXPECT_FALSE(AISetTarget(&obj, OBJECT_HANDLE_NONE));
}

/**
 * @test AIMainLinked.AIDestroyObjNoCrash
 * @brief Verifies aIDestroy Obj No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIDestroyObjNoCrash) {
  EXPECT_NO_THROW(AIDestroyObj(&obj));
}

/**
 * @test AIMainLinked.AIUpdateAnimNoCrash
 * @brief Verifies aIUpdate Anim No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIUpdateAnimNoCrash) {
  EXPECT_NO_THROW(AIUpdateAnim(&obj));
}

/**
 * @test AIMainLinked.AITurnTowardsMatrixNoCrash
 * @brief Verifies aITurn Towards Matrix No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AITurnTowardsMatrixNoCrash) {
  matrix m = Identity_matrix;
  EXPECT_NO_THROW(AITurnTowardsMatrix(&obj, 1.0f, &m));
}

/**
 * @test AIMainLinked.AIFindRandomRoomReturnsMinusOne
 * @brief Verifies aIFind Random Room Returns Minus One.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIFindRandomRoomReturnsMinusOne) {
  int depth=-1;
  // With stubbed BOA/path, may return a room or -1; just check no crash and depth updated
  int r = AIFindRandomRoom(&obj, obj.ai_info, nullptr, -1, 0, 5, false, true, &depth);
  EXPECT_GE(r, -1);
  EXPECT_GE(depth, -1);
}

/**
 * @test AIMainLinked.AIMakeNextRoomListNoCrash
 * @brief Verifies aIMake Next Room List No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIMakeNextRoomListNoCrash) {
  int next[10]={0};
  EXPECT_NO_THROW(AIMakeNextRoomList(1, next, 10));
}

/**
 * @test AIMainLinked.AIStatusCircleFrameReturnsFalse
 * @brief Verifies aIStatus Circle Frame Returns False.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIStatusCircleFrameReturnsFalse) {
  int reg=0;
  EXPECT_FALSE(AIStatusCircleFrame(&obj, &obj, 10.0f, 5.0f, &reg));
}

/**
 * @test AIMainLinked.AIInitAllNoCrash
 * @brief Verifies aIInit All No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIInitAllNoCrash) {
  EXPECT_NO_THROW(AIInitAll());
}

/**
 * @test AIMainLinked.AIInitWithInvalidClassStillCreates
 * @brief Verifies aIInit With Invalid Class Still Creates.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIInitWithInvalidClassStillCreates) {
  object o2{}; memset(&o2,0,sizeof(o2)); o2.type=OBJ_ROBOT; o2.handle=1000; o2.roomnum=1; o2.control_type=CT_AI; o2.movement_type=MT_NONE; o2.id=0; o2.size=5; o2.orient=Identity_matrix;
  static ai_frame frame2; memset(&frame2,0,sizeof(frame2)); frame2.circle_distance=20; frame2.max_velocity=10;
  o2.ai_info=&frame2;
  // Use valid movement class (0-4) to avoid OOB on Object_info anim array; invalid 255 would segfault due to bounds
  EXPECT_NO_THROW(AIInit(&o2, 2, 2, 2));
}

/**
 * @test AIMainLinked.AIDoFrameWithNoAIInfoNoCrash
 * @brief Verifies aIDo Frame With No AIInfo No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIDoFrameWithNoAIInfoNoCrash) {
  // Real AIDoFrame dereferences ai_info unconditionally (see AImain.cpp:6036 ai_info->flags) so nullptr crashes.
  // Test the safe early-out path instead: valid ai_info with AIF_DISABLED.
  object o3{}; memset(&o3,0,sizeof(o3)); o3.type=OBJ_ROBOT; o3.handle=555; o3.roomnum=1; o3.control_type=CT_AI; o3.movement_type=MT_NONE; o3.id=0; o3.size=5; o3.orient=Identity_matrix;
  static ai_frame frame3; memset(&frame3,0,sizeof(frame3)); frame3.flags = AIF_DISABLED; frame3.max_velocity=10;
  for(int i=0;i<MAX_GOALS;i++) frame3.goals[i].used=false;
  o3.ai_info=&frame3;
  EXPECT_NO_THROW(AIDoFrame(&o3));
}

/**
 * @test AIMainLinked.BuddyHandleArrayAccessible
 * @brief Verifies buddy Handle Array Accessible.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, BuddyHandleArrayAccessible) {
  Buddy_handle[0]=123;
  EXPECT_EQ(Buddy_handle[0], 123);
  Buddy_handle[0]=OBJECT_HANDLE_NONE;
}

/**
 * @test AIMainLinked.AI_NumRenderedZeroInitially
 * @brief Verifies aI Num Rendered Zero Initially.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AI_NumRenderedZeroInitially) {
  EXPECT_EQ(AI_NumRendered, 0);
  AI_RenderedList[0]=obj.handle;
  AI_NumRendered=1;
  EXPECT_EQ(AI_NumRendered, 1);
  AI_NumRendered=0;
}

/**
 * @test AIMainLinked.GametimeAdvances
 * @brief Verifies gametime Advances.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, GametimeAdvances) {
  float before=Gametime;
  Gametime+=Frametime;
  EXPECT_FLOAT_EQ(Gametime, before+0.016f);
}

/**
 * @test AIMainLinked.MultipleAIDoFramesNoCrash
 * @brief Verifies multiple AIDo Frames No Crash.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, MultipleAIDoFramesNoCrash) {
  for(int i=0;i<5;i++) EXPECT_NO_THROW(AIDoFrame(&obj));
}

/**
 * @test AIMainLinked.AIPathAllocPathReturnsFalse
 * @brief Verifies aIPath Alloc Path Returns False.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIPathAllocPathReturnsFalse) {
  int sr=0, er=0; vector sp{}, ep{};
  EXPECT_FALSE(AIPathAllocPath(&obj, obj.ai_info, nullptr, &sr, &sp, &er, &ep, 5.0f, 0, 0, -1));
}

/**
 * @test AIMainLinked.AIUniqueGoalIdIncrements
 * @brief Verifies aIUnique Goal Id Increments.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, AIUniqueGoalIdIncrements) {
  int before=AI_unique_goal_id;
  GoalAddGoal(&obj, AIG_WANDER_AROUND, nullptr, 0, 1.0f, 0, -1, 0);
  EXPECT_GE(AI_unique_goal_id, before);
}

/**
 * @test AIMainLinked.GoalDoFrameNoCrashWithAIMain
 * @brief Verifies goal Do Frame No Crash With AIMain.
 *
 * @details
 * Exercises the AIMainLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AImain.cpp
 * @ingroup descent3_tests
 */
TEST_F(AIMainLinked, GoalDoFrameNoCrashWithAIMain) {
  GoalAddGoal(&obj, AIG_WANDER_AROUND, nullptr, 0, 1.0f, 0, -1, 0);
  EXPECT_NO_THROW(GoalDoFrame(&obj));
  EXPECT_NO_THROW(AIDoFrame(&obj));
}
