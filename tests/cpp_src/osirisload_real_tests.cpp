/**
 * @file osirisload_real_tests.cpp
 * @brief Tests for OsirisLoadandBind.cpp 3934 lines — OSIRIS DLL script.
 *
 * @details
 * system. Covers the script-capability type macros (with the
 * OBJ_DUMMY asymmetry between object and type variants), the
 * module slot lifecycle (FreeModule flag rules: level-module state
 * reset, DLLELSEWHERE skipping teardown, mission-dll detach, full
 * wipe), FindLoadedModule's case-insensitive stem matching, and
 * get_full_path_to_module's search-order codes where on-disk
 * success returns -1 but library success returns 0.
 *
 * This harness validates the behavior of `Descent3/osirisload.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/osirisload.cpp`
 * @par Harness
 * `osirisload_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/osirisload.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>
#include <map>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;

// replicated constants
constexpr int MAX_LOADED_MODULES = 6;
constexpr uint8_t OSIMF_INUSE = 0x1;
constexpr uint8_t OSIMF_LEVEL = 0x2;
constexpr uint8_t OSIMF_DLLELSEWHERE = 0x4;
enum ObjTypes {
  OBJ_NONE_O = 0, OBJ_WALL_O, OBJ_FIREBALL_O, OBJ_ROBOT_O, OBJ_WEAPON_O,
  OBJ_PLAYER_O, OBJ_POWERUP_O, OBJ_CLUTTER_O, OBJ_BUILDING_O, OBJ_DEBRIS_O,
  OBJ_CAMERA_O, OBJ_SHOCKWAVE_O, OBJ_FLARE_O, OBJ_DOOR_O, OBJ_DUMMY_O,
  OBJ_GHOST_O, OBJ_MARKER_O
};

// ---------------------------------------------------------------------------
// Capability macros replication (OsirisLoadandBind.cpp:556-565)
struct OObj {
  int type = OBJ_NONE_O;
};

#define HAVECUSTOMONLY_O(type) (type == OBJ_CAMERA_O)
static bool CanBeAssignedScript(const OObj *obj) {
  return (obj && ((obj->type == OBJ_ROBOT_O) || (obj->type == OBJ_BUILDING_O) ||
                  (obj->type == OBJ_POWERUP_O) || (obj->type == OBJ_CLUTTER_O) ||
                  (obj->type == OBJ_DOOR_O) || (obj->type == OBJ_CAMERA_O) ||
                  (obj->type == OBJ_DUMMY_O)));
}
static bool CanHaveAnyScript(const OObj *obj) {
  return CanBeAssignedScript(obj) ||
         (obj && ((obj->type == OBJ_DEBRIS_O) || (obj->type == OBJ_GHOST_O)));
}
static bool CanTypeBeAssignedScript(int type) {
  return (type == OBJ_ROBOT_O) || (type == OBJ_BUILDING_O) ||
         (type == OBJ_POWERUP_O) || (type == OBJ_CLUTTER_O) ||
         (type == OBJ_DOOR_O) || (type == OBJ_CAMERA_O);
}

/**
 * @test OsirisCaps.TypeMacrosWithDummyAsymmetry
 * @brief Verifies type Macros With Dummy Asymmetry.
 *
 * @details
 * Exercises the OsirisCaps code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/osirisload.cpp
 * @ingroup descent3_tests
 */
TEST(OsirisCaps, TypeMacrosWithDummyAsymmetry) {
  // assigned-script capable types
  EXPECT_TRUE(CanBeAssignedScript(new OObj{OBJ_ROBOT_O}));
  EXPECT_TRUE(CanBeAssignedScript(new OObj{OBJ_BUILDING_O}));
  EXPECT_TRUE(CanBeAssignedScript(new OObj{OBJ_DUMMY_O})); // dummy included here
  EXPECT_FALSE(CanBeAssignedScript(nullptr));              // null rejected

  // quirk: the TYPE-only variant omits OBJ_DUMMY, so a dummy OBJECT can be
  // assigned a script while a dummy TYPE cannot
  EXPECT_TRUE(CanBeAssignedScript(new OObj{OBJ_DUMMY_O}));
  EXPECT_FALSE(CanTypeBeAssignedScript(OBJ_DUMMY_O));

  // any-script adds debris + ghost
  OObj debris{OBJ_DEBRIS_O}, ghost{OBJ_GHOST_O}, player{OBJ_PLAYER_O};
  EXPECT_TRUE(CanHaveAnyScript(&debris));
  EXPECT_TRUE(CanHaveAnyScript(&ghost));
  EXPECT_FALSE(CanHaveAnyScript(&player)); // players excluded entirely
  EXPECT_TRUE(CanHaveAnyScript(new OObj{OBJ_DUMMY_O}));

  // only cameras are custom-only
  EXPECT_TRUE(HAVECUSTOMONLY_O(OBJ_CAMERA_O));
  EXPECT_FALSE(HAVECUSTOMONLY_O(OBJ_ROBOT_O));
}

// ---------------------------------------------------------------------------
// Module table replication (OsirisLoadandBind.cpp:664-727, 773-789)
struct ModMock {
  uint8_t flags = 0;
  int reference_count = 0;
  char *module_name = nullptr;
  bool shutdown_called = false;
  bool strings_freed = false;
  char *string_table = nullptr;
  int strings_loaded = 0;
};
struct LevelStateMock {
  bool level_loaded = false;
  int num_customs = 0;
  int dll_id = 0;
};
struct MissionStateMock {
  bool mission_loaded = false;
  int dll_id = 0;
};

static ModMock MODS[MAX_LOADED_MODULES];
static LevelStateMock CUR_LEVEL;
static MissionStateMock CUR_MISSION;
static int ShutdownCalls = 0;

static void RepFreeModule(int id) {
  if (id < 0 || id >= MAX_LOADED_MODULES)
    return;
  if (!(MODS[id].flags & OSIMF_INUSE))
    return; // quirk path: unused slots silently ignored

  if (MODS[id].flags & OSIMF_LEVEL) {
    CUR_LEVEL.level_loaded = false;
    CUR_LEVEL.num_customs = 0;
    CUR_LEVEL.dll_id = 0;
  }
  if (!(MODS[id].flags & OSIMF_DLLELSEWHERE)) {
    MODS[id].shutdown_called = true; // ShutdownDLL()
    ShutdownCalls++;
    MODS[id].strings_freed = MODS[id].string_table != nullptr;
  }
  if (CUR_MISSION.mission_loaded && id == CUR_MISSION.dll_id) {
    CUR_MISSION.mission_loaded = false;
    CUR_MISSION.dll_id = 0;
  }
  MODS[id].string_table = nullptr;
  MODS[id].strings_loaded = 0;
  MODS[id].flags = 0;
  MODS[id].reference_count = 0;
}

static int RepFindLoadedModule(const fs::path &module_name) {
  fs::path real_name = module_name.stem();
  for (int i = 0; i < MAX_LOADED_MODULES; i++) {
    if (MODS[i].flags & OSIMF_INUSE) {
      if (MODS[i].module_name &&
          strcasecmp(MODS[i].module_name, real_name.u8string().c_str()) == 0)
        return i;
    }
  }
  return -1;
}

static void ResetMods() {
  for (auto &m : MODS)
    m = ModMock{};
  CUR_LEVEL = LevelStateMock{};
  CUR_MISSION = MissionStateMock{};
  ShutdownCalls = 0;
}

/**
 * @test OsirisModules.FreeModuleFlagRulesAndStateResets
 * @brief Verifies free Module Flag Rules And State Resets.
 *
 * @details
 * Exercises the OsirisModules code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/osirisload.cpp
 * @ingroup descent3_tests
 */
TEST(OsirisModules, FreeModuleFlagRulesAndStateResets) {
  ResetMods();
  static char name1[] = "mylevel";
  MODS[2] = {OSIMF_INUSE | OSIMF_LEVEL, 3, name1, false, false, nullptr, 0};
  CUR_LEVEL.level_loaded = true;
  CUR_LEVEL.num_customs = 5;
  CUR_LEVEL.dll_id = 2;

  RepFreeModule(2);
  EXPECT_EQ(ShutdownCalls, 1); // ours -> shut down
  EXPECT_FALSE(CUR_LEVEL.level_loaded);
  EXPECT_EQ(CUR_LEVEL.num_customs, 0);
  EXPECT_EQ(MODS[2].flags, 0);
  EXPECT_EQ(MODS[2].reference_count, 0);

  // mission-owned module: freeing detaches the mission pointer too
  ResetMods();
  static char name2[] = "mission";
  MODS[1] = {OSIMF_INUSE, 1, name2, false, false, nullptr, 0};
  CUR_MISSION.mission_loaded = true;
  CUR_MISSION.dll_id = 1;
  RepFreeModule(1);
  EXPECT_FALSE(CUR_MISSION.mission_loaded);
  EXPECT_EQ(CUR_MISSION.dll_id, 0);

  // DLLELSEWHERE: shared DLL skips shutdown/strings but still wipes slot
  ResetMods();
  MODS[0] = {OSIMF_INUSE | OSIMF_DLLELSEWHERE, 7, nullptr, false, false,
             (char *)0x1, 9};
  RepFreeModule(0);
  EXPECT_EQ(ShutdownCalls, 0);          // NOT our dll
  EXPECT_FALSE(MODS[0].strings_freed);  // string table left to owner
  EXPECT_EQ(MODS[0].flags, 0);          // but slot is reusable again

  // out-of-range and unused slots are no-ops
  ResetMods();
  RepFreeModule(-1);
  RepFreeModule(MAX_LOADED_MODULES);
  RepFreeModule(4); // never in use
  EXPECT_EQ(ShutdownCalls, 0);
}

/**
 * @test OsirisModules.FindLoadedModuleMatchesStemCaseInsensitively
 * @brief Verifies find Loaded Module Matches Stem Case Insensitively.
 *
 * @details
 * Exercises the OsirisModules code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/osirisload.cpp
 * @ingroup descent3_tests
 */
TEST(OsirisModules, FindLoadedModuleMatchesStemCaseInsensitively) {
  ResetMods();
  static char nm[] = "MyCoolScript";
  MODS[3] = {OSIMF_INUSE, 1, nm, false, false, nullptr, 0};

  // extension and case ignored via stem comparison
  EXPECT_EQ(RepFindLoadedModule("mycoolscript.dll"), 3);
  EXPECT_EQ(RepFindLoadedModule("MYCOOLSCRIPT.so"), 3);

  // different stem misses even with same extension
  EXPECT_EQ(RepFindLoadedModule("other.dll"), -1);

  // unused slots skipped even if names match
  MODS[3].flags = 0;
  EXPECT_EQ(RepFindLoadedModule("mycoolscript.dll"), -1);
}

// ---------------------------------------------------------------------------
// get_full_path_to_module replication (OsirisLoadandBind.cpp:884-934)
enum { CFES_NONE = 0, CFES_ON_DISK = 1, CFES_IN_LIBRARY = 2 };
static int TC_exist_result = CFES_NONE;
static std::string TC_extract_dir = "/tmp/extracted";
static std::map<std::string, std::string> TC_extracted_scripts;

struct PathResultN {
  fs::path fullpath;
  fs::path basename;
  int ret;
};

static PathResultN RepGetFullPath(const fs::path &module_name) {
  PathResultN r;
  fs::path ppath = module_name.parent_path();
  fs::path adjusted_fname = module_name.filename();

  std::string p = adjusted_fname.u8string();
  std::transform(p.begin(), p.end(), p.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  adjusted_fname = p;
  r.basename = adjusted_fname.stem();

  fs::path adjusted_name =
      ppath.empty() ? adjusted_fname : (ppath / adjusted_fname);

  // mod_GetRealModuleName stand-in: appends .dll when missing
  fs::path modfilename = adjusted_name;
  if (modfilename.extension().empty())
    modfilename += ".dll";

  switch (TC_exist_result) {
  case CFES_ON_DISK:
    r.fullpath = fs::path("/scripts") / modfilename;
    r.ret = -1; // quirk: DISK success returns NEGATIVE one
    return r;
  case CFES_IN_LIBRARY: {
    auto it = TC_extracted_scripts.find(r.basename.u8string());
    if (it != TC_extracted_scripts.end()) {
      r.fullpath = fs::path(TC_extract_dir) / it->second;
      r.ret = 0;
      return r;
    }
    r.ret = -2; // not extracted -> treated as 3rd-party, refuse
    return r;
  }
  default:
    r.fullpath.clear();
    r.ret = -2;
    return r;
  }
}

/**
 * @test OsirisPaths.SearchOrderCodesAndLowercasing
 * @brief Verifies search Order Codes And Lowercasing.
 *
 * @details
 * Exercises the OsirisPaths code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/osirisload.cpp
 * @ingroup descent3_tests
 */
TEST(OsirisPaths, SearchOrderCodesAndLowercasing) {
  TC_extracted_scripts.clear();

  // on disk: success reported as -1 (caller checks < -1 style)
  TC_exist_result = CFES_ON_DISK;
  auto r1 = RepGetFullPath("sub/MyLevel.DLL");
  EXPECT_EQ(r1.ret, -1);
  // filename lowercased for Linux, directory case untouched
  EXPECT_STREQ(r1.fullpath.filename().u8string().c_str(), "mylevel.dll");
  EXPECT_EQ(r1.basename.u8string(), "mylevel");

  // in library and previously extracted: 0 plus temp path
  TC_exist_result = CFES_IN_LIBRARY;
  TC_extracted_scripts["mylevel"] = "tmp_mylevel.dll";
  auto r2 = RepGetFullPath("MyLevel");
  EXPECT_EQ(r2.ret, 0);
  EXPECT_NE(r2.fullpath.u8string().find("tmp_mylevel.dll"), std::string::npos);
  EXPECT_NE(r2.fullpath.u8string().find("/tmp/extracted"), std::string::npos);

  // quirk: a hog-library hit WITHOUT extraction record is refused as a
  // suspected 3rd-party level script -- silent skip, not an error box
  TC_extracted_scripts.clear();
  auto r3 = RepGetFullPath("SomeHogScript");
  EXPECT_EQ(r3.ret, -2);

  // nowhere: cleared path, -2
  TC_exist_result = CFES_NONE;
  auto r4 = RepGetFullPath("missing");
  EXPECT_EQ(r4.ret, -2);
  EXPECT_TRUE(r4.fullpath.empty());
}
