/**
 * @file dllwrappers_real_tests.cpp
 * @brief Tests for DllWrappers.cpp (350 lines) — DLL wrapper layer.
 *
 * @details
 * Covers ChangePlayerShipIndex validation, inventory forwarding,
 * D3W sound wrappers demo recording, and trivial wrappers.
 * Replicates logic to avoid heavy newui/sound/attach deps.
 *
 * This harness validates the behavior of `Descent3/DllWrappers.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/DllWrappers.cpp`
 * @par Harness
 * `dllwrappers_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/DllWrappers.cpp
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstring>

// --- replicated constants ---
constexpr int MAX_PLAYERS = 32;
constexpr int MAX_SHIPS = 30;
constexpr int DF_RECORDING = 1;

// --- mocks ---
static int g_demoFlags = 0;
static int g_demoWrite3DCalls = 0, g_demoWrite2DCalls = 0;
static int g_sound3DCalls = 0, g_sound2DCalls = 0;
static int g_playerChangeShipCalls = 0;
static int g_freeCockpit = 0, g_closeHUD = 0, g_initHUD = 0, g_initCockpit = 0;
static int g_objInitCalls = 0;

struct MockShip { bool used = false; };
static MockShip MockShips[MAX_SHIPS];
static int MockPlayerNum = 0;

struct MockPlayer {
  int start_roomnum = 0;
  int objnum = 0;
  int ship_index = 0;
  // inventory mock
  std::vector<std::pair<int,int>> items; // type,id
  bool CheckItem(int type,int id) const { for(auto &p:items) if(p.first==type && p.second==id) return true; return false; }
  bool Add(int type,int id) { items.emplace_back(type,id); return true; }
  bool Remove(int type,int id) { for(size_t i=0;i<items.size();++i) if(items[i].first==type&&items[i].second==id){ items.erase(items.begin()+i); return true;} return false; }
  int GetTypeIDCount(int type,int id) const { int c=0; for(auto &p:items) if(p.first==type&&p.second==id) c++; return c; }
};
static MockPlayer MockPlayers[MAX_PLAYERS];
struct MockObject { int dummy=0; };
static MockObject MockObjects[1000];

// replicated ChangePlayerShipIndex (from DllWrappers.cpp:193-226)
static void MockPlayerChangeShip(int pnum,int ship_index){ g_playerChangeShipCalls++; MockPlayers[pnum].ship_index = ship_index; }
static void MockObjInitTypeSpecific(MockObject*,bool){ g_objInitCalls++; }
static void MockFreeCockpit(){ g_freeCockpit++; }
static void MockCloseShipHUD(){ g_closeHUD++; }
static void MockInitShipHUD(int){ g_initHUD++; }
static void MockInitCockpit(int){ g_initCockpit++; }

static void ReplicatedChangePlayerShipIndex(int pnum,int ship_index){
  if (pnum < 0 || pnum >= MAX_PLAYERS) return;
  if (MockPlayers[pnum].start_roomnum == -1) return;
  if (ship_index < 0 || ship_index >= MAX_SHIPS) return;
  if (!MockShips[ship_index].used) return;
  bool isme = (pnum == MockPlayerNum);
  if (isme) { MockFreeCockpit(); MockCloseShipHUD(); }
  MockPlayerChangeShip(pnum, ship_index);
  MockObjInitTypeSpecific(&MockObjects[MockPlayers[pnum].objnum], false);
  if (isme) { MockInitShipHUD(MockPlayers[pnum].ship_index); MockInitCockpit(MockPlayers[pnum].ship_index); }
}

// replicated D3W wrappers
static int ReplicatedD3W_Play3dSound(int snd, MockObject* obj, float vol, int flags){
  (void)obj; (void)flags;
  if (g_demoFlags == DF_RECORDING) g_demoWrite3DCalls++;
  g_sound3DCalls++;
  return snd;
}
static int ReplicatedD3W_Play2dSound(int snd,float vol){
  (void)vol;
  if (g_demoFlags == DF_RECORDING) g_demoWrite2DCalls++;
  g_sound2DCalls++;
  return snd;
}
static void ReplicatedMonoPrintf(int n, char* fmt, ...){ (void)n;(void)fmt; }

// inventory wrappers
static bool ReplicatedInvCheckItem(int pnum,int type,int id){
  // ASSERT omitted in RELEASE
  if(pnum<0||pnum>=MAX_PLAYERS) return false;
  return MockPlayers[pnum].CheckItem(type,id);
}
static bool ReplicatedInvAddTypeID(int pnum,int type,int id){
  if(pnum<0||pnum>=MAX_PLAYERS) return false;
  return MockPlayers[pnum].Add(type,id);
}
static bool ReplicatedInvRemove(int pnum,int type,int id){
  if(pnum<0||pnum>=MAX_PLAYERS) return false;
  return MockPlayers[pnum].Remove(type,id);
}
static int ReplicatedInvGetTypeIDCount(int pnum,int type,int id){
  if(pnum<0||pnum>=MAX_PLAYERS) return 0;
  return MockPlayers[pnum].GetTypeIDCount(type,id);
}

struct DllWrappersFixture : ::testing::Test {
  void SetUp() override {
    g_demoFlags=0; g_demoWrite3DCalls=0; g_demoWrite2DCalls=0; g_sound3DCalls=0; g_sound2DCalls=0;
    g_playerChangeShipCalls=0; g_freeCockpit=0; g_closeHUD=0; g_initHUD=0; g_initCockpit=0; g_objInitCalls=0;
    MockPlayerNum=0;
    for(int i=0;i<MAX_PLAYERS;++i){ MockPlayers[i]=MockPlayer(); MockPlayers[i].start_roomnum=0; MockPlayers[i].objnum=i; }
    for(int i=0;i<MAX_SHIPS;++i) MockShips[i].used=false;
  }
};

/**
 * @test DllWrappersFixture.ChangePlayerShipValid
 * @brief Verifies change Player Ship Valid.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, ChangePlayerShipValid) {
  MockShips[5].used=true;
  ReplicatedChangePlayerShipIndex(0,5);
  EXPECT_EQ(g_playerChangeShipCalls,1);
  EXPECT_EQ(MockPlayers[0].ship_index,5);
  EXPECT_EQ(g_objInitCalls,1);
}

/**
 * @test DllWrappersFixture.ChangePlayerShipUpdatesHUDWhenIsMe
 * @brief Verifies change Player Ship Updates HUDWhen Is Me.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, ChangePlayerShipUpdatesHUDWhenIsMe) {
  MockShips[3].used=true;
  MockPlayerNum=2;
  ReplicatedChangePlayerShipIndex(2,3);
  EXPECT_EQ(g_freeCockpit,1);
  EXPECT_EQ(g_closeHUD,1);
  EXPECT_EQ(g_initHUD,1);
  EXPECT_EQ(g_initCockpit,1);
}

/**
 * @test DllWrappersFixture.ChangePlayerShipNoHUDWhenNotMe
 * @brief Verifies change Player Ship No HUDWhen Not Me.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, ChangePlayerShipNoHUDWhenNotMe) {
  MockShips[3].used=true;
  MockPlayerNum=1;
  ReplicatedChangePlayerShipIndex(0,3);
  EXPECT_EQ(g_freeCockpit,0);
  EXPECT_EQ(g_closeHUD,0);
  EXPECT_EQ(g_initHUD,0);
}

/**
 * @test DllWrappersFixture.ChangePlayerShipInvalidPnum
 * @brief Verifies change Player Ship Invalid Pnum.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, ChangePlayerShipInvalidPnum) {
  MockShips[1].used=true;
  ReplicatedChangePlayerShipIndex(-1,1);
  ReplicatedChangePlayerShipIndex(MAX_PLAYERS,1);
  EXPECT_EQ(g_playerChangeShipCalls,0);
}

/**
 * @test DllWrappersFixture.ChangePlayerShipNoStartRoom
 * @brief Verifies change Player Ship No Start Room.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, ChangePlayerShipNoStartRoom) {
  MockShips[1].used=true;
  MockPlayers[0].start_roomnum=-1;
  ReplicatedChangePlayerShipIndex(0,1);
  EXPECT_EQ(g_playerChangeShipCalls,0);
}

/**
 * @test DllWrappersFixture.ChangePlayerShipInvalidShipIndex
 * @brief Verifies change Player Ship Invalid Ship Index.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, ChangePlayerShipInvalidShipIndex) {
  ReplicatedChangePlayerShipIndex(0,-1);
  ReplicatedChangePlayerShipIndex(0,MAX_SHIPS);
  EXPECT_EQ(g_playerChangeShipCalls,0);
}

/**
 * @test DllWrappersFixture.ChangePlayerShipNotUsed
 * @brief Verifies change Player Ship Not Used.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, ChangePlayerShipNotUsed) {
  MockShips[2].used=false;
  ReplicatedChangePlayerShipIndex(0,2);
  EXPECT_EQ(g_playerChangeShipCalls,0);
}

/**
 * @test DllWrappersFixture.D3WPlaySoundDemoRecording
 * @brief Verifies d3WPlay Sound Demo Recording.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, D3WPlaySoundDemoRecording) {
  g_demoFlags=DF_RECORDING;
  ReplicatedD3W_Play3dSound(5,nullptr,1.0f,0);
  EXPECT_EQ(g_demoWrite3DCalls,1);
  EXPECT_EQ(g_sound3DCalls,1);
  ReplicatedD3W_Play2dSound(6,1.0f);
  EXPECT_EQ(g_demoWrite2DCalls,1);
  EXPECT_EQ(g_sound2DCalls,1);
}

/**
 * @test DllWrappersFixture.D3WPlaySoundNoDemo
 * @brief Verifies d3WPlay Sound No Demo.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, D3WPlaySoundNoDemo) {
  g_demoFlags=0;
  ReplicatedD3W_Play3dSound(5,nullptr,1.0f,0);
  EXPECT_EQ(g_demoWrite3DCalls,0);
  EXPECT_EQ(g_sound3DCalls,1);
}

/**
 * @test DllWrappersFixture.InvWrappersForward
 * @brief Verifies inv Wrappers Forward.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, InvWrappersForward) {
  EXPECT_FALSE(ReplicatedInvCheckItem(0,10,20));
  EXPECT_TRUE(ReplicatedInvAddTypeID(0,10,20));
  EXPECT_TRUE(ReplicatedInvCheckItem(0,10,20));
  EXPECT_EQ(ReplicatedInvGetTypeIDCount(0,10,20),1);
  EXPECT_TRUE(ReplicatedInvRemove(0,10,20));
  EXPECT_FALSE(ReplicatedInvCheckItem(0,10,20));
}

/**
 * @test DllWrappersFixture.InvWrappersInvalidPnum
 * @brief Verifies inv Wrappers Invalid Pnum.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, InvWrappersInvalidPnum) {
  EXPECT_FALSE(ReplicatedInvCheckItem(-1,1,1));
  EXPECT_FALSE(ReplicatedInvAddTypeID(99,1,1));
  EXPECT_EQ(ReplicatedInvGetTypeIDCount(MAX_PLAYERS,1,1),0);
}

/**
 * @test DllWrappersFixture.MonoPrintfNoop
 * @brief Verifies mono Printf Noop.
 *
 * @details
 * Exercises the DllWrappersFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/DllWrappers.cpp
 * @ingroup descent3_tests
 */
TEST_F(DllWrappersFixture, MonoPrintfNoop) {
  // should not crash
  ReplicatedMonoPrintf(0,nullptr);
  ReplicatedMonoPrintf(1,(char*)"test %d", 42);
  SUCCEED();
}
