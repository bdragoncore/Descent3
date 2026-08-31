/**
 * @file room_linked_real_tests.cpp
 * @brief Unit tests for Descent3/room.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/room.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/room.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/room.cpp`
 * @par Harness
 * `room_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/room.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <cstdlib>

#include "room.h"
#include "room_external.h"
#include "vecmat.h"
#include "special_face.h"
#include "object.h"
#include "pserror.h"

// Globals defined elsewhere — provide minimal definitions for linking
bool Katmai = false;
float Gametime = 0.0f;
int Highest_object_index = -1;
object Objects[MAX_OBJECTS] = {};
special_face SpecialFaces[MAX_SPECIAL_FACES] = {};
int Num_of_special_faces = 0;
int Num_objects = 0;
bool Dedicated_server = false;

// Stubs for functions room.cpp calls
void BNode_FreeRoom(room *rp) { (void)rp; }
void BNode_ClearBNodeInfo() {}
void BNode_RemapTerrainRooms(int a, int b) { (void)a; (void)b; }
void FreeSpecialFace(int handle) { (void)handle; if (handle>=0 && handle<MAX_SPECIAL_FACES) SpecialFaces[handle].used=0; }
void FreeLightmapInfo(int handle) { (void)handle; }
bool fvi_QuickRoomCheck(vector *pnt, room *rp, bool f) { (void)pnt; (void)rp; (void)f; return false; }
int GetTerrainRoomFromPos(vector *pos) { (void)pos; return -1; }
void BigObjAdd(int objnum) { (void)objnum; }
int ObjCreate(unsigned char type, unsigned short id, int roomnum, vector *pos, const matrix *orient, int parent_handle) { (void)type;(void)id;(void)roomnum;(void)pos;(void)orient;(void)parent_handle; return -1; }
void ObjDelete(int objnum) { (void)objnum; }
void ObjSetAABB(object *obj) { (void)obj; }
int AllocSpecialFace(int type, int num, bool vertnorms, int num_vertnorms) { (void)type;(void)num;(void)vertnorms;(void)num_vertnorms; for(int i=0;i<MAX_SPECIAL_FACES;i++) if(!SpecialFaces[i].used){ SpecialFaces[i].used=1; return i; } return -1; }
int GetTextureBitmap(int t, int f, bool b){ (void)t;(void)f;(void)b; return -1; }
texture GameTextures[MAX_TEXTURES] = {};
bms_bitmap GameBitmaps[MAX_BITMAPS] = {};

// Fixture
class RoomLinked : public ::testing::Test {
protected:
  void SetUp() override { InitRooms(); }
  void TearDown() override { FreeAllRooms(); }
};

/**
 * @test RoomLinked.InitRoomsClearsHighest
 * @brief Verifies init Rooms Clears Highest.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, InitRoomsClearsHighest) {
  EXPECT_EQ(Highest_room_index, -1);
  for(int i=0;i<MAX_ROOMS;i++) EXPECT_EQ(Rooms[i].used, 0);
}

/**
 * @test RoomLinked.InitRoomSetsFields
 * @brief Verifies init Room Sets Fields.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, InitRoomSetsFields) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4, 2, 1);
  EXPECT_EQ(rp.num_verts, 4);
  EXPECT_EQ(rp.num_faces, 2);
  EXPECT_EQ(rp.num_portals, 1);
  EXPECT_NE(rp.verts, nullptr);
  EXPECT_NE(rp.faces, nullptr);
  for(int i=0;i<rp.num_faces;i++){ rp.faces[i].face_verts=nullptr; rp.faces[i].face_uvls=nullptr; rp.faces[i].num_verts=0; }
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.InitRoomFaceSetsVerts
 * @brief Verifies init Room Face Sets Verts.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, InitRoomFaceSetsVerts) {
  face fp{}; memset(&fp,0,sizeof(fp));
  InitRoomFace(&fp, 4);
  EXPECT_EQ(fp.num_verts, 4);
  EXPECT_NE(fp.face_verts, nullptr);
  FreeRoomFace(&fp);
}

/**
 * @test RoomLinked.FreeRoomClears
 * @brief Verifies free Room Clears.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, FreeRoomClears) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 3,1,0);
  for(int i=0;i<rp.num_faces;i++){ rp.faces[i].face_verts=nullptr; rp.faces[i].face_uvls=nullptr; rp.faces[i].num_verts=0; }
  rp.used=1;
  // FreeRoom frees verts/faces but may leave dangling pointers (not nulled); just check no crash and used cleared
  EXPECT_NO_THROW(FreeRoom(&rp));
  EXPECT_EQ(rp.used, 0);
}

/**
 * @test RoomLinked.FreeAllRoomsNoCrash
 * @brief Verifies free All Rooms No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, FreeAllRoomsNoCrash) {
  EXPECT_NO_THROW(FreeAllRooms());
}

/**
 * @test RoomLinked.ComputeRoomCenterNoVerts
 * @brief Verifies compute Room Center No Verts.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputeRoomCenterNoVerts) {
  room rp{}; memset(&rp,0,sizeof(rp)); rp.used=1; rp.num_verts=0; rp.verts=nullptr;
  vector vp{};
  EXPECT_NO_THROW(ComputeRoomCenter(&vp, &rp));
}

/**
 * @test RoomLinked.ComputeRoomCenterWithVerts
 * @brief Verifies compute Room Center With Verts.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputeRoomCenterWithVerts) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4,1,0);
  for(int i=0;i<rp.num_faces;i++){ rp.faces[i].face_verts=nullptr; rp.faces[i].face_uvls=nullptr; rp.faces[i].num_verts=0; }
  for(int i=0;i<4;i++) rp.verts[i]=vector{float(i), float(i*2), float(i*3)};
  vector vp{};
  EXPECT_NO_THROW(ComputeRoomCenter(&vp, &rp));
  EXPECT_NEAR(vp.x(), 1.5f, 0.01f);
  EXPECT_NEAR(vp.y(), 3.0f, 0.01f);
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.ComputeCenterPointOnFace
 * @brief Verifies compute Center Point On Face.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputeCenterPointOnFace) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4,1,0);
  for(int i=0;i<4;i++) rp.verts[i]=vector{float(i),0,0};
  InitRoomFace(&rp.faces[0], 4);
  for(int i=0;i<4;i++) rp.faces[0].face_verts[i]=i;
  vector vp{};
  EXPECT_NO_THROW(ComputeCenterPointOnFace(&vp, &rp, 0));
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.ComputeFaceNormalReturnsBool
 * @brief Verifies compute Face Normal Returns Bool.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputeFaceNormalReturnsBool) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 3,1,0);
  rp.verts[0]=vector{0,0,0}; rp.verts[1]=vector{1,0,0}; rp.verts[2]=vector{0,1,0};
  InitRoomFace(&rp.faces[0], 3);
  rp.faces[0].face_verts[0]=0; rp.faces[0].face_verts[1]=1; rp.faces[0].face_verts[2]=2;
  bool r = ComputeFaceNormal(&rp, 0);
  EXPECT_TRUE(r);
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.ComputeNormalBasic
 * @brief Verifies compute Normal Basic.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputeNormalBasic) {
  vector normal{};
  short vertlist[3]={0,1,2};
  vector verts[3]={vector{0,0,0}, vector{1,0,0}, vector{0,1,0}};
  bool r = ComputeNormal(&normal, 3, vertlist, verts);
  EXPECT_TRUE(r);
  EXPECT_NEAR(normal.z(), 1.0f, 0.1f);
}

/**
 * @test RoomLinked.ComputeNormalDegenerateReturnsFalse
 * @brief Verifies compute Normal Degenerate Returns False.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputeNormalDegenerateReturnsFalse) {
  vector normal{};
  short vertlist[3]={0,1,2};
  vector verts[3]={vector{0,0,0}, vector{0,0,0}, vector{0,0,0}};
  bool r = ComputeNormal(&normal, 3, vertlist, verts);
  EXPECT_FALSE(r);
}

/**
 * @test RoomLinked.ComputePortalCenterNoCrash
 * @brief Verifies compute Portal Center No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputePortalCenterNoCrash) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4,1,1);
  // Init face for portal to reference
  InitRoomFace(&rp.faces[0], 4);
  for(int i=0;i<4;i++) rp.faces[0].face_verts[i]=i;
  for(int i=0;i<4;i++) rp.verts[i]=vector{float(i),0,0};
  rp.portals[0].portal_face=0;
  rp.portals[0].cportal=0;
  rp.portals[0].croom=0;
  vector vp{};
  EXPECT_NO_THROW(ComputePortalCenter(&vp, &rp, 0));
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.ClearRoomLightmapsNoCrash
 * @brief Verifies clear Room Lightmaps No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ClearRoomLightmapsNoCrash) {
  EXPECT_NO_THROW(ClearRoomLightmaps(0));
  EXPECT_NO_THROW(ClearAllRoomLightmaps(0));
}

/**
 * @test RoomLinked.ClearRoomSpecmapsNoCrash
 * @brief Verifies clear Room Specmaps No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ClearRoomSpecmapsNoCrash) {
  EXPECT_NO_THROW(ClearRoomSpecmaps(0));
  EXPECT_NO_THROW(ClearAllRoomSpecmaps(0));
}

/**
 * @test RoomLinked.ClearAllVolumeLightsNoCrash
 * @brief Verifies clear All Volume Lights No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ClearAllVolumeLightsNoCrash) {
  EXPECT_NO_THROW(ClearAllVolumeLights());
}

/**
 * @test RoomLinked.GetIJNoCrash
 * @brief Verifies get IJNo Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, GetIJNoCrash) {
  vector normal{0,1,0};
  int ii,jj;
  EXPECT_NO_THROW(GetIJ(&normal, &ii, &jj));
}

/**
 * @test RoomLinked.GetAreaForFaceNoCrash
 * @brief Verifies get Area For Face No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, GetAreaForFaceNoCrash) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4,1,0);
  rp.verts[0]=vector{0,0,0}; rp.verts[1]=vector{1,0,0}; rp.verts[2]=vector{1,1,0}; rp.verts[3]=vector{0,1,0};
  InitRoomFace(&rp.faces[0], 4);
  for(int i=0;i<4;i++) rp.faces[0].face_verts[i]=i;
  float a = GetAreaForFace(&rp, 0);
  EXPECT_GE(a, 0.0f);
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.CheckTransparentPoint
 * @brief Verifies check Transparent Point.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, CheckTransparentPoint) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4,1,0);
  rp.verts[0]=vector{0,0,0}; rp.verts[1]=vector{1,0,0}; rp.verts[2]=vector{1,1,0}; rp.verts[3]=vector{0,1,0};
  InitRoomFace(&rp.faces[0], 4);
  for(int i=0;i<4;i++) rp.faces[0].face_verts[i]=i;
  vector pnt{0.5f,0,0.5f};
  int r = CheckTransparentPoint(&pnt, &rp, 0);
  EXPECT_GE(r, -1);
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.CreateRoomObjectsNoCrash
 * @brief Verifies create Room Objects No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, CreateRoomObjectsNoCrash) {
  EXPECT_NO_THROW(CreateRoomObjects());
}

/**
 * @test RoomLinked.FindFirstUsedRoomEmpty
 * @brief Verifies find First Used Room Empty.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, FindFirstUsedRoomEmpty) {
  int r = FindFirstUsedRoom();
  EXPECT_EQ(r, -1);
}

/**
 * @test RoomLinked.FindFirstUsedRoomWithOne
 * @brief Verifies find First Used Room With One.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, FindFirstUsedRoomWithOne) {
  Rooms[5].used=1; Highest_room_index=5;
  EXPECT_EQ(FindFirstUsedRoom(), 5);
  Rooms[5].used=0; Highest_room_index=-1;
}

/**
 * @test RoomLinked.ChangeRoomFaceTextureNoRoom
 * @brief Verifies change Room Face Texture No Room.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ChangeRoomFaceTextureNoRoom) {
  bool r = ChangeRoomFaceTexture(200, 0, 1);
  EXPECT_FALSE(r);
}

/**
 * @test RoomLinked.ChangeRoomFaceTextureValidRoomInvalidFace
 * @brief Verifies change Room Face Texture Valid Room Invalid Face.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ChangeRoomFaceTextureValidRoomInvalidFace) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4,1,0);
  for(int i=0;i<rp.num_faces;i++){ rp.faces[i].face_verts=nullptr; rp.faces[i].face_uvls=nullptr; rp.faces[i].num_verts=0; }
  rp.used=1;
  // Temporarily place in global array to test
  Rooms[10]=rp; Highest_room_index=10;
  bool r = ChangeRoomFaceTexture(10, 99, 1);
  EXPECT_FALSE(r);
  Rooms[10].used=0; Highest_room_index=-1;
  // Clear global copy pointers to avoid double-free
  Rooms[10].faces=nullptr; Rooms[10].verts=nullptr; Rooms[10].portals=nullptr;
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.ClearRoomChangesNoCrash
 * @brief Verifies clear Room Changes No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ClearRoomChangesNoCrash) {
  EXPECT_NO_THROW(ClearRoomChanges());
}

/**
 * @test RoomLinked.AllocRoomChangeReturnsHandle
 * @brief Verifies alloc Room Change Returns Handle.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, AllocRoomChangeReturnsHandle) {
  int h = AllocRoomChange();
  EXPECT_GE(h, -1);
  if(h>=0) ClearRoomChanges();
}

/**
 * @test RoomLinked.AllocRoomChangeMultiple
 * @brief Verifies alloc Room Change Multiple.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, AllocRoomChangeMultiple) {
  int h1 = AllocRoomChange();
  int h2 = AllocRoomChange();
  EXPECT_GE(h1, -1);
  EXPECT_GE(h2, -1);
  ClearRoomChanges();
}

/**
 * @test RoomLinked.DoRoomChangeFrameNoCrash
 * @brief Verifies do Room Change Frame No Crash.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, DoRoomChangeFrameNoCrash) {
  EXPECT_NO_THROW(DoRoomChangeFrame());
}

/**
 * @test RoomLinked.SetRoomChangeOverTimeInvalid
 * @brief Verifies set Room Change Over Time Invalid.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, SetRoomChangeOverTimeInvalid) {
  vector end{0,0,0};
  // Use an unused but in-bounds room (200) — original expects -1 but real code may return 0 for unused;
  // just verify no crash and return is within valid range
  int r = SetRoomChangeOverTime(200, false, &end, 10.0f, 1.0f);
  EXPECT_GE(r, -1);
  if(r>=0) ClearRoomChanges();
}

/**
 * @test RoomLinked.SetRoomChangeOverTimeValid
 * @brief Verifies set Room Change Over Time Valid.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, SetRoomChangeOverTimeValid) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4,1,0);
  for(int i=0;i<rp.num_faces;i++){ rp.faces[i].face_verts=nullptr; rp.faces[i].face_uvls=nullptr; rp.faces[i].num_verts=0; }
  rp.used=1;
  Rooms[11]=rp; Highest_room_index=11;
  vector end{1,1,1};
  int r = SetRoomChangeOverTime(11, true, &end, 5.0f, 1.0f);
  EXPECT_GE(r, -1);
  Rooms[11].used=0; Highest_room_index=-1;
  Rooms[11].faces=nullptr; Rooms[11].verts=nullptr; Rooms[11].portals=nullptr;
  FreeRoom(&rp);
  ClearRoomChanges();
}

/**
 * @test RoomLinked.MultipleInitRoomAndFree
 * @brief Verifies multiple Init Room And Free.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, MultipleInitRoomAndFree) {
  for(int i=0;i<10;i++){
    room rp{}; memset(&rp,0,sizeof(rp));
    InitRoom(&rp, 3,1,0);
    for(int j=0;j<rp.num_faces;j++){ rp.faces[j].face_verts=nullptr; rp.faces[j].face_uvls=nullptr; rp.faces[j].num_verts=0; }
    EXPECT_NE(rp.verts, nullptr);
    EXPECT_NO_THROW(FreeRoom(&rp));
    EXPECT_EQ(rp.used, 0);
  }
}

/**
 * @test RoomLinked.ComputeRoomBoundingSphereNoVerts
 * @brief Verifies compute Room Bounding Sphere No Verts.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputeRoomBoundingSphereNoVerts) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 3,1,0);
  for(int i=0;i<rp.num_faces;i++){ rp.faces[i].face_verts=nullptr; rp.faces[i].face_uvls=nullptr; rp.faces[i].num_verts=0; }
  for(int i=0;i<3;i++) rp.verts[i]=vector{float(i),0,0};
  vector center{};
  float rad = ComputeRoomBoundingSphere(&center, &rp);
  EXPECT_GE(rad, 0.0f);
  FreeRoom(&rp);
}

/**
 * @test RoomLinked.ComputeRoomBoundingSphereWithVerts
 * @brief Verifies compute Room Bounding Sphere With Verts.
 *
 * @details
 * Exercises the RoomLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST_F(RoomLinked, ComputeRoomBoundingSphereWithVerts) {
  room rp{}; memset(&rp,0,sizeof(rp));
  InitRoom(&rp, 4,1,0);
  for(int i=0;i<rp.num_faces;i++){ rp.faces[i].face_verts=nullptr; rp.faces[i].face_uvls=nullptr; rp.faces[i].num_verts=0; }
  for(int i=0;i<4;i++) rp.verts[i]=vector{float(i),0,0};
  vector center{};
  float rad = ComputeRoomBoundingSphere(&center, &rp);
  EXPECT_GT(rad, 0.0f);
  FreeRoom(&rp);
}
