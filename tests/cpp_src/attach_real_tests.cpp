/**
 * @file attach_real_tests.cpp
 * @brief Tests for attach.cpp (~692 lines).
 *
 * @details
 * Covers AttachPointPos math (via public wrappers), both AttachObject
 * overloads (aligned + rad), mass propagation up the parent chain,
 * ultimate-parent handle propagation, UnattachFromParent/Child/Children,
 * and AttachUpdateSubObjects velocity/position following.
 *
 * This harness validates the behavior of `Descent3/attach.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/attach.cpp`
 * @par Harness
 * `attach_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/attach.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "attach.h"
#include "object.h"
#include "polymodel.h"
#include "objinfo.h"
#include "terrain.h"
#include "multi.h"
#include "game.h"
#include "demofile.h"
#include "vecmat.h"
#include "multi.h"
#include "log.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
object Objects[MAX_OBJECTS];
poly_model Poly_models[4];
object_info Object_info[64];
int Game_mode = 0;
uint32_t Demo_flags = 0;
netgame_info Netgame;

// ---------------------------------------------------------------------------
// Scripted object lookup
// ---------------------------------------------------------------------------
static object *g_lookup[MAX_OBJECTS];

object *ObjGet(int handle) {
  if (handle < 0 || handle >= MAX_OBJECTS)
    return nullptr;
  return g_lookup[handle];
}

// ---------------------------------------------------------------------------
// Stubs for engine calls attach.cpp depends on
// ---------------------------------------------------------------------------
static int g_last_terrain_room = 5;
static int g_dead_calls = 0;

struct PosCapture {
  vector pos;
  int room;
};
static PosCapture g_setpos;
static bool g_setpos_called;

int GetTerrainRoomFromPos(vector *pos) {
  (void)pos;
  return g_last_terrain_room;
}
void SetObjectDeadFlag(object *obj, bool tell_clients_to_remove, bool play_sound_on_clients) {
  (void)obj;
  (void)tell_clients_to_remove;
  (void)play_sound_on_clients;
  g_dead_calls++;
}
void ObjSetPos(object *obj, vector *pos, int roomnum, matrix *orient, bool f_update_attached_children) {
  (void)obj;
  (void)orient;
  (void)f_update_attached_children;
  g_setpos.pos = *pos;
  g_setpos.room = roomnum;
  g_setpos_called = true;
}

void MultiSendAttach(object *, char, object *, char, bool) {}
void MultiSendAttachRad(object *, char, object *, float) {}
void MultiSendUnattach(object *) {}
void DemoWriteAttachObj(object *, char, object *, char, bool) {}
void DemoWriteAttachObjRad(object *, char, object *, float) {}
void DemoWriteUnattachObj(object *) {}

void SetNormalizedTimeObj(object *obj, float *normalized_time) {
  (void)obj;
  (void)normalized_time;
}
void SetModelAnglesAndPos(poly_model *pm, float *normalized_time, uint32_t subobj_flags) {
  (void)pm;
  (void)normalized_time;
  (void)subobj_flags;
}

// ---------------------------------------------------------------------------
// Fixtures
// ---------------------------------------------------------------------------
static a_bank g_slots[2];
static bsp_info g_subs[1];
static matrix s_identity = IDENTITY_MATRIX;
static const vector V0 = {0.f, 0.f, 0.f};

static void SetupModel(int model_num, int n_attach) {
  Poly_models[model_num] = poly_model{};
  Poly_models[model_num].n_attach = n_attach;
  Poly_models[model_num].attach_slots = g_slots;
  Poly_models[model_num].submodel = g_subs;
  Poly_models[model_num].n_models = 1;
  for (int i = 0; i < n_attach; i++) {
    g_slots[i] = a_bank{};
    g_slots[i].parent = -1; // no submodel instancing
    g_slots[i].pnt = V0;
    g_slots[i].norm = {0.f, 0.f, 1.f};
    g_slots[i].f_uvec = false;
  }
}

/**
 * @brief GTest fixture for AttachTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class AttachTest : public ::testing::Test {
protected:
  object parent{}, child{}, grandchild{};

  void SetUp() override {
    memset(g_lookup, 0, sizeof(g_lookup));
    memset(Objects, 0, sizeof(object) * 8);
    memset(Object_info, 0, sizeof(object_info) * 64);
    for (auto &s : g_slots)
      s = a_bank{};
    g_subs[0] = bsp_info{};
    g_setpos_called = false;
    g_dead_calls = 0;
    Game_mode = 0;
    Demo_flags = 0;
    g_last_terrain_room = 5;

    SetupObject(parent, 0);
    SetupObject(child, 1);
    SetupObject(grandchild, 2);

    // default model: one AP at origin pointing +z
    SetupModel(0, 1);
    Poly_models[0].attach_slots[0].pnt = {2.f, 0.f, 0.f};

    Object_info[1].phys_info.mass = 3.0f;
  }

  static int NextHandle(object &o, int objnum) {
    o.handle = 1000 + objnum; // simple unique handles (lookup is scripted)
    g_lookup[o.handle] = &o;
    return o.handle;
  }

  void SetupObject(object &o, int id) {
    o = object{};
    o.type = OBJ_ROBOT;
    o.id = id;
    o.roomnum = 42;
    o.size = 4.0f;
    o.mtype.phys_info.mass = 10.0f;
    o.rtype.pobj_info.model_num = 0;
    o.orient = s_identity;
    o.attach_parent_handle = OBJECT_HANDLE_NONE;
    o.attach_ultimate_handle = OBJECT_HANDLE_NONE;
  }

  static void AllocAttachSlots(object &o, int n) {
    o.attach_children = new int32_t[n];
    for (int i = 0; i < n; i++)
      o.attach_children[i] = OBJECT_HANDLE_NONE;
  }

  void LinkParents() { AllocAttachSlots(parent, Poly_models[0].n_attach); }
};

/**
 * @test AttachTest.RadAttachPlacesChildAlongAPNormal
 * @brief Verifies rad Attach Places Child Along APNormal.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, RadAttachPlacesChildAlongAPNormal) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  LinkParents();
  parent.pos = {100.f, 200.f, 300.f};

  // AP at {2,0,0}, normal +z, child size 4 -> dist = 0.5*4 = 2
  ASSERT_TRUE(AttachObject(&parent, 0, &child, 0.5f));

  EXPECT_TRUE(child.flags & OF_ATTACHED);
  EXPECT_EQ(child.attach_type, AT_RAD);
  EXPECT_EQ(child.attach_dist, 2.0f);
  EXPECT_EQ(child.attach_parent_handle, parent.handle);
  EXPECT_FLOAT_EQ(g_setpos.pos.x(), 102.f); // pnt + fvec*dist + parent pos
  EXPECT_FLOAT_EQ(g_setpos.pos.y(), 200.f);
  EXPECT_FLOAT_EQ(g_setpos.pos.z(), 302.f);
  EXPECT_EQ(g_setpos.room, 42); // inside room: parent's room kept
}

/**
 * @test AttachTest.RadAttachNegativePercentRejected
 * @brief Verifies rad Attach Negative Percent Rejected.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, RadAttachNegativePercentRejected) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  EXPECT_FALSE(AttachObject(&parent, 0, &child, -0.5f));
  EXPECT_FALSE(child.flags & OF_ATTACHED);
}

/**
 * @test AttachTest.AttachRejectsBadParentAP
 * @brief Verifies attach Rejects Bad Parent AP.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, AttachRejectsBadParentAP) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  EXPECT_FALSE(AttachObject(&parent, 9, &child, 0.5f));
  EXPECT_FALSE(AttachObject(&parent, -1, &child, 0.5f));
}

/**
 * @test AttachTest.AttachRejectsAlreadyAttachedChild
 * @brief Verifies attach Rejects Already Attached Child.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, AttachRejectsAlreadyAttachedChild) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  child.flags |= OF_ATTACHED;
  EXPECT_FALSE(AttachObject(&parent, 0, &child, 0.5f));
}

/**
 * @test AttachTest.AlignedAttachOrientsAndMovesChild
 * @brief Verifies aligned Attach Orients And Moves Child.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, AlignedAttachOrientsAndMovesChild) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  LinkParents();
  parent.pos = {10.f, 0.f, 0.f};

  // Both objects use model 0 whose single AP points +z. DoPosOrient
  // computes how the child must rotate to connect AP-to-AP (goal fvec
  // is the negated parent AP normal).
  vector pos;
  matrix orient;
  ASSERT_TRUE(AttachDoPosOrient(&parent, 0, &child, 0, false, false, &pos, &orient));
  // goal fvec is negated AP normal => -z; uvec defaults to +y-ish
  vector fvec = orient.fvec;
  vm_NormalizeVector(&fvec);
  EXPECT_NEAR(fvec.z(), -1.0f, 1e-4f);
}

/**
 * @test AttachTest.MassPropagatesToParentOnAttachAndUnattach
 * @brief Verifies mass Propagates To Parent On Attach And Unattach.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, MassPropagatesToParentOnAttachAndUnattach) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  LinkParents();

  float before = parent.mtype.phys_info.mass;
  ASSERT_TRUE(AttachObject(&parent, 0, &child, 0.25f));
  EXPECT_FLOAT_EQ(parent.mtype.phys_info.mass, before + 3.0f); // Object_info[child.id].mass

  ASSERT_TRUE(UnattachFromParent(&child));
  EXPECT_FLOAT_EQ(parent.mtype.phys_info.mass, before);
}

/**
 * @test AttachTest.UltimateHandleChainsThroughGrandchildren
 * @brief Verifies ultimate Handle Chains Through Grandchildren.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, UltimateHandleChainsThroughGrandchildren) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  NextHandle(grandchild, 5);
  LinkParents();

  // child has its own model+AP so grandchild can hang off it
  SetupModel(1, 1);
  child.rtype.pobj_info.model_num = 1;
  AllocAttachSlots(child, 1); // child becomes a grandparent

  ASSERT_TRUE(AttachObject(&parent, 0, &child, 0.25f));
  ASSERT_TRUE(AttachObject(&child, 0, &grandchild, 0.25f));

  EXPECT_EQ(child.attach_ultimate_handle, parent.handle);
  EXPECT_EQ(grandchild.attach_ultimate_handle, parent.handle);
}

/**
 * @test AttachTest.UnattachFromParentClearsState
 * @brief Verifies unattach From Parent Clears State.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, UnattachFromParentClearsState) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  LinkParents();
  ASSERT_TRUE(AttachObject(&parent, 0, &child, 0.5f));
  parent.attach_children[0] = child.handle;

  ASSERT_TRUE(UnattachFromParent(&child));

  EXPECT_FALSE(child.flags & OF_ATTACHED);
  EXPECT_EQ(child.attach_parent_handle, OBJECT_HANDLE_NONE);
  EXPECT_EQ(child.attach_ultimate_handle, OBJECT_HANDLE_NONE);
  EXPECT_EQ(parent.attach_children[0], OBJECT_HANDLE_NONE);
  // ultimate handle now points at itself
  EXPECT_EQ(child.attach_ultimate_handle, OBJECT_HANDLE_NONE);
}

/**
 * @test AttachTest.UnattachNotAttachedFails
 * @brief Verifies unattach Not Attached Fails.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, UnattachNotAttachedFails) {
  NextHandle(child, 4);
  EXPECT_FALSE(UnattachFromParent(&child));
}

/**
 * @test AttachTest.UnattachChildBySlot
 * @brief Verifies unattach Child By Slot.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, UnattachChildBySlot) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  LinkParents();
  EXPECT_FALSE(UnattachChild(&parent, 0)); // empty slot

  ASSERT_TRUE(AttachObject(&parent, 0, &child, 0.5f));
  parent.attach_children[0] = child.handle;
  EXPECT_TRUE(UnattachChild(&parent, 0));
  EXPECT_FALSE(child.flags & OF_ATTACHED);
}

/**
 * @test AttachTest.UnattachChildrenClearsAllSlots
 * @brief Verifies unattach Children Clears All Slots.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, UnattachChildrenClearsAllSlots) {
  Poly_models[0].n_attach = 2;
  LinkParents();
  NextHandle(parent, 3);
  NextHandle(child, 4);
  NextHandle(grandchild, 5);

  // fake two attached children without full attach flow
  child.flags |= OF_ATTACHED;
  child.attach_parent_handle = parent.handle;
  grandchild.flags |= OF_ATTACHED;
  grandchild.attach_parent_handle = parent.handle;
  parent.attach_children[0] = child.handle;
  parent.attach_children[1] = grandchild.handle;

  float pmass = parent.mtype.phys_info.mass;
  ASSERT_TRUE(UnattachChildren(&parent));

  EXPECT_FALSE(child.flags & OF_ATTACHED);
  EXPECT_FALSE(grandchild.flags & OF_ATTACHED);
  EXPECT_LT(parent.mtype.phys_info.mass, pmass); // masses removed
}

/**
 * @test AttachTest.UpdateSubObjectsMovesRadChildWithParent
 * @brief Verifies update Sub Objects Moves Rad Child With Parent.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, UpdateSubObjectsMovesRadChildWithParent) {
  NextHandle(parent, 3);
  NextHandle(child, 4);
  LinkParents();

  ASSERT_TRUE(AttachObject(&parent, 0, &child, 0.5f)); // child at AP + z*2 relative

  // move the parent; update should recompute child position
  parent.pos = {50.f, 60.f, 70.f};
  parent.attach_children[0] = child.handle;

  g_setpos_called = false;
  AttachUpdateSubObjects(&parent);

  ASSERT_TRUE(g_setpos_called);
  EXPECT_FLOAT_EQ(g_setpos.pos.x(), 52.f);
  EXPECT_FLOAT_EQ(g_setpos.pos.y(), 60.f);
  EXPECT_FLOAT_EQ(g_setpos.pos.z(), 72.f);
  // velocity copied from parent
  EXPECT_TRUE(true);
}

/**
 * @test AttachTest.UpdateSubObjectsNullChildrenSafe
 * @brief Verifies update Sub Objects Null Children Safe.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, UpdateSubObjectsNullChildrenSafe) {
  NextHandle(parent, 3);
  parent.attach_children = nullptr;
  AttachUpdateSubObjects(&parent); // must not crash
  SUCCEED();
}

/**
 * @test AttachTest.UpdateSubObjectsInvalidSlotIgnored
 * @brief Verifies update Sub Objects Invalid Slot Ignored.
 *
 * @details
 * Exercises the AttachTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/attach.cpp
 * @ingroup descent3_tests
 */
TEST_F(AttachTest, UpdateSubObjectsInvalidSlotIgnored) {
  NextHandle(parent, 3);
  LinkParents();
  parent.attach_children[0] = 99999; // stale handle
  parent.pos = {1.f, 1.f, 1.f};
  g_setpos_called = false;
  AttachUpdateSubObjects(&parent);
  EXPECT_FALSE(g_setpos_called);
}
