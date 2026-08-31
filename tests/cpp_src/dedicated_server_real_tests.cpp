/**
 * @file dedicated_server_real_tests.cpp
 * @brief Tests for Descent3/dedicated_server.cpp -- cvar table semantics,.
 *
 * @details
 * permission gating, console/telnet command routing, and the idle
 * timeout that quits the server.
 *
 * The socket layer is shimmed inert (socket() fails), so every code
 * path here runs headless. RunServerConfigs/DedicatedServerLex are
 * static in the module and only reachable through public entry points.
 *
 * This harness validates the behavior of `Descent3/dedicated_server.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/dedicated_server.cpp`
 * @par Harness
 * `dedicated_server_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/dedicated_server.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>

#include "dedicated_server.h"
#include "inffile.h"
#include "multi.h"
#include "multi_external.h"
#include "multi_ui.h"
#include "multi_dll_mgr.h"
#include "objinfo.h"
#include "ship.h"
#include "player.h"
#include "Mission.h"
#include "init.h"
#include "args.h"
#include "descent.h"
#include "chrono_timer.h"
#include "hud.h"
#include "AppConsole.h"
#include "game2dll.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// module-local helpers missing from every header
void ParseLine(char *srcline, char *command, char *operand, int cmdlen, int oprlen);

// ---- recorded output ----
struct SendRecord {
  int color;
  std::string msg;
  int to;
};
static std::vector<SendRecord> g_sends;
static std::vector<std::string> g_order;
static std::vector<std::string> g_console_out;
#define REC(name) g_order.push_back(name)

static bool g_load_multidll_ok = true;
static int g_find_objid = -1;
static int g_msg_dest_to = MULTI_SEND_MESSAGE_ALL;
static float g_now = 0.0f;
static std::vector<std::string> g_con_queue;
static int g_last_gamedll_evt = -1;
static std::string g_last_input_string;

// ---- globals the module reads/writes ----
netgame_info Netgame;
netplayer NetPlayers[MAX_NET_PLAYERS];
int Player_num = 0;
player Players[MAX_PLAYERS];
object_info Object_info[MAX_OBJECT_IDS];
ship Ships[MAX_SHIPS];
tMission Current_mission;
int ServerTimeout = 0;
float LastPacketReceived = 0;
int Multi_next_level = 0;
char Auto_login_name[MAX_AUTO_LOGIN_STUFF_LEN];
char Auto_login_pass[MAX_AUTO_LOGIN_STUFF_LEN];
char PXO_hosted_lobby_name[100];
char Multi_message_of_the_day[HUD_MESSAGE_LENGTH * 2];
int MultiDLLGameStarting = 0;
dllinfo DLLInfo;

// GameArgs is a fixed 2D table per args.h -- a char** alias links but
// reads garbage, so mirror the real layout exactly.
char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];

int FindArg(const char *which, int start) {
  for (int i = start; i < MAX_ARGS && GameArgs[i][0]; i++)
    if (strcasecmp(GameArgs[i], which) == 0)
      return i;
  return 0;
}
int FindArgChar(const char *which, char single) {
  for (int i = 1; i < MAX_ARGS && GameArgs[i][0]; i++) {
    const char *s = GameArgs[i];
    if (s[0] == '-' && s[1] == single && s[2] == '\0')
      return i;
  }
  return FindArg(which);
}

const char *GetStringFromTable(int id) {
  switch (id) {
  case 117:
    return "%s>%s"; // TXT_HUDSAY
  case 505:
    return "couldntinit";
  case 506:
    return "dllinit";
  case 507:
    return "connectloaderr";
  case 508:
    return "loadmissionerr";
  case 509:
    return "missionloaded";
  case 510:
    return "-Server-";
  case 511:
    return "badcommandline";
  case 512:
    return "missingconfig";
  case 513:
    return "badconfig";
  case 514:
    return "settingsloaded";
  case 515:
    return "settingserr";
  case 516:
    return "disallowobject";
  case 517:
    return "allowobjects";
  case 518:
    return "varset";
  case 519:
    return "badcommand";
  case 520:
    return "enterpass";
  case 807:
    return "notcompatible";
  default:
    return "str";
  }
}

void Error(const char *, ...) {}
Inventory::Inventory() {}
Inventory::~Inventory() {}
void Inventory::Reset(bool, int) {}

namespace D3 {
float ChronoTimer::GetTime() { return g_now; }
} // namespace D3

void *mem_malloc_sub(int size, const char *, int) { return malloc(size); }
void mem_free_sub(void *p) { free(p); }

std::size_t CleanupStr(char *dest, const char *src, std::size_t destlen) {
  while (*src == ' ' || *src == '\t')
    src++;
  std::size_t len = strlen(src);
  while (len > 0 && (src[len - 1] == ' ' || src[len - 1] == '\t' || src[len - 1] == '\r' || src[len - 1] == '\n'))
    len--;
  if (len >= destlen)
    len = destlen - 1;
  memcpy(dest, src, len);
  dest[len] = '\0';
  return len;
}

const char *GetMessageDestination(const char *message, int *destination) {
  *destination = g_msg_dest_to;
  return message;
}

bool con_Input(char *buf, int buflen) {
  if (g_con_queue.empty())
    return false;
  std::string cmd = g_con_queue.front();
  g_con_queue.erase(g_con_queue.begin());
  strncpy(buf, cmd.c_str(), buflen - 1);
  buf[buflen - 1] = '\0';
  return true;
}

void con_Printf(const char *fmt, ...) {
  char buf[CON_MAX_STRINGLEN];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  g_console_out.push_back(buf);
}

void CallGameDLL(int evt, dllinfo *info) {
  g_last_gamedll_evt = evt;
  if (info && info->input_string)
    g_last_input_string = info->input_string;
  REC("gamedll" + std::to_string(evt));
}
void CallMultiDLL(int) { REC("multidll"); }
int LoadMultiDLL(const char *) { return g_load_multidll_ok ? 1 : 0; }
const char *GetMissionName(const char *) { return "TestMission"; }
bool LoadMission(const char *) { REC("loadmission"); return true; }
int CheckMissionForScript(char *, char *, int) { return 2; }
void MultiStartServer(int, char *, int) { REC("startserver"); }
void MultiEndLevel() { REC("endlevel"); }
void MultiLeaveGame() { REC("leavegame"); }
int MultiLoadSettings(const std::filesystem::path &) { REC("loadsettings"); return true; }
void MultiResetSettings() { REC("resetsettings"); }
void MultiSendMessageFromServer(int color, char *msg, int to) {
  g_sends.push_back({color, msg, to});
}
void MultiSetAudioTauntTime(float t, int) { REC("taunt" + std::to_string(t)); }
bool PlayerSetShipPermission(int pnum, char *name, bool allowed) {
  REC((std::string("perm:") + name + ":" + (allowed ? "1" : "0") + ":" + std::to_string(pnum)));
  return true;
}
int FindObjectIDName(const char *) { return g_find_objid; }
void SetFunctionMode(function_mode mode) { REC("mode" + std::to_string((int)mode)); }

void rtp_StartLog() { REC("startlog"); }
void rtp_StopLog() { REC("stoplog"); }

uint32_t nw_GetThisIP() { return 0x0100007f; } // 127.0.0.1

// libc socket shims -- inert network for headless tests. Signatures
// mirror the system headers (pulled in above) so the definitions match.
extern "C" {
int socket(int, int, int) { return -1; }
int bind(int, const struct sockaddr *, socklen_t) { return -1; }
int listen(int, int) { return -1; }
int accept(int, struct sockaddr *, socklen_t *) { return -1; }
ssize_t recv(int, void *, size_t, int) { return -1; }
ssize_t send(int, const void *, size_t, int) { return 0; }
int shutdown(int, int) { return 0; }
int close(int) { return 0; }
in_addr_t inet_addr(const char *) { return 0x0100007f; }
char *inet_ntoa(struct in_addr) {
  static char buf[16];
  strcpy(buf, "127.0.0.1");
  return buf;
}
}
int make_nonblocking(int) { return 0; }

// InfFile is only exercised through LoadServerConfigFile's failure paths;
// Open always failing is enough for those.
InfFile::InfFile() : m_fp(nullptr), m_lexfn(nullptr) {}
InfFile::~InfFile() {}
bool InfFile::Open(const std::filesystem::path &, const char *, int (*)(const char *)) { return false; }
void InfFile::Close() {}
bool InfFile::ReadLine() { return false; }
int InfFile::ParseLine(char *, int) { return INFFILE_ERROR; }

// ---- helpers ----
// index constants mirror the CVars[] table order in dedicated_server.cpp
enum {
  CX_PPS = 0,
  CX_TIMELIMIT = 1,
  CX_KILLGOAL = 2,
  CX_RESPAWN = 3,
  CX_GAMENAME = 4,
  CX_QUIT = 8,
  CX_ENDLEVEL = 9,
  CX_MAXPLAYERS = 10,
  CX_SAY = 11,
  CX_SETDIFF = 34,
};

static void ResetWorld() {
  memset(&Netgame, 0, sizeof(Netgame));
  memset(NetPlayers, 0, sizeof(NetPlayers));
  memset(Players, 0, sizeof(Players));
  memset(Object_info, 0, sizeof(Object_info));
  memset(Ships, 0, sizeof(Ships));
  memset(&Current_mission, 0, sizeof(Current_mission));
  memset(GameArgs, 0, sizeof(GameArgs));
  strcpy(GameArgs[0], "descent3");
  memset(Auto_login_name, 0, sizeof(Auto_login_name));
  memset(Auto_login_pass, 0, sizeof(Auto_login_pass));
  memset(PXO_hosted_lobby_name, 0, sizeof(PXO_hosted_lobby_name));
  memset(Multi_message_of_the_day, 0, sizeof(Multi_message_of_the_day));
  Player_num = 0;
  ServerTimeout = 0;
  LastPacketReceived = 0;
  Multi_next_level = 0;
  MultiDLLGameStarting = 1;
  Dedicated_server = false;
  g_sends.clear();
  g_order.clear();
  g_console_out.clear();
  g_con_queue.clear();
  g_load_multidll_ok = true;
  g_find_objid = -1;
  g_msg_dest_to = MULTI_SEND_MESSAGE_ALL;
  g_now = 0.0f;
  g_last_gamedll_evt = -1;
  g_last_input_string.clear();
  strcpy(Players[0].callsign, "Cmdr");
}

/**
 * @test DedicatedServer.StartRequiresDashDedicatedArg
 * @brief Verifies start Requires Dash Dedicated Arg.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, StartRequiresDashDedicatedArg) {
  ResetWorld();
  StartDedicatedServer();
  EXPECT_FALSE(Dedicated_server);

  strcpy(GameArgs[1], "-dedicated");
  StartDedicatedServer();
  EXPECT_TRUE(Dedicated_server);
}

/**
 * @test DedicatedServer.ParseLineTokenizesSeparatorsAndTrims
 * @brief Verifies parse Line Tokenizes Separators And Trims.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, ParseLineTokenizesSeparatorsAndTrims) {
  char cmd[255], opr[255];

  memset(cmd, 0xAA, sizeof(cmd));
  memset(opr, 0xAA, sizeof(opr));
  char src1[] = "TimeLimit \t=:\t 30 minutes ";
  ParseLine(src1, cmd, opr, 255, 255);
  EXPECT_STREQ(cmd, "TimeLimit");
  // QUIRK: strtok splits the command off with " \t=:", but the operand
  // is the raw remainder of the line -- separator characters survive
  EXPECT_STREQ(opr, "=:\t 30 minutes");

  // an empty source line leaves both outputs untouched (quirk)
  char untouched[8] = "keep";
  char src2[] = "";
  ParseLine(src2, untouched, opr, 8, 255);
  EXPECT_STREQ(untouched, "keep");
  EXPECT_STREQ(opr, "=:\t 30 minutes");

  // a bare separator produces a null command -> bail with outputs untouched
  memset(cmd, 0, sizeof(cmd));
  char src3[] = "   ";
  ParseLine(src3, cmd, opr, 255, 255);
  EXPECT_EQ(cmd[0], '\0');
}

/**
 * @test DedicatedServer.IntCvarsClampToDeclaredRange
 * @brief Verifies int Cvars Clamp To Declared Range.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, IntCvarsClampToDeclaredRange) {
  ResetWorld();

  SetCVarInt(CX_PPS, 999); // PPS range [2,40]
  EXPECT_EQ(Netgame.packets_per_second, 40);
  SetCVarInt(CX_PPS, 0);
  EXPECT_EQ(Netgame.packets_per_second, 2);
  SetCVarInt(CX_PPS, 15);
  EXPECT_EQ(Netgame.packets_per_second, 15);

  SetCVarInt(CX_MAXPLAYERS, 1); // MaxPlayers re-clamped to [2,MAX_PLAYERS]
  EXPECT_EQ(Netgame.max_players, 2);
  SetCVarInt(CX_MAXPLAYERS, 99);
  EXPECT_EQ(Netgame.max_players, MAX_PLAYERS);
}

/**
 * @test DedicatedServer.IntCvarsToggleNetgameFlagsAndDifficulty
 * @brief Verifies int Cvars Toggle Netgame Flags And Difficulty.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, IntCvarsToggleNetgameFlagsAndDifficulty) {
  ResetWorld();

  SetCVarInt(CX_TIMELIMIT, 10);
  EXPECT_TRUE(Netgame.flags & NF_TIMER);
  SetCVarInt(CX_TIMELIMIT, 0);
  EXPECT_FALSE(Netgame.flags & NF_TIMER);

  SetCVarInt(CX_KILLGOAL, 25);
  EXPECT_TRUE(Netgame.flags & NF_KILLGOAL);
  SetCVarInt(CX_KILLGOAL, 0);
  EXPECT_FALSE(Netgame.flags & NF_KILLGOAL);

  Netgame.flags |= NF_BRIGHT_PLAYERS;
  SetCVarInt(21, 0); // BrightPlayers
  EXPECT_FALSE(Netgame.flags & NF_BRIGHT_PLAYERS);

  Netgame.flags |= NF_USE_SMOOTHING;
  SetCVarInt(12, 0); // UseSmoothing
  EXPECT_FALSE(Netgame.flags & NF_USE_SMOOTHING);

  SetCVarInt(13, 1); // SendRotVel
  EXPECT_TRUE(Netgame.flags & NF_SENDROTVEL);
  SetCVarInt(23, 1); // AccurateCollisions
  EXPECT_TRUE(Netgame.flags & NF_USE_ACC_WEAP);
  SetCVarInt(30, 1); // RandomizeRespawn
  EXPECT_TRUE(Netgame.flags & NF_RANDOMIZE_RESPAWN);
  SetCVarInt(31, 1); // AllowMouselook
  EXPECT_TRUE(Netgame.flags & NF_ALLOW_MLOOK);

  SetCVarInt(CX_SETDIFF, 3);
  EXPECT_EQ(Netgame.difficulty, 3);
}

/**
 * @test DedicatedServer.Peer2PeerAndPermissableAreMutuallyExclusive
 * @brief Verifies peer2Peer And Permissable Are Mutually Exclusive.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, Peer2PeerAndPermissableAreMutuallyExclusive) {
  ResetWorld();

  SetCVarInt(22, 1); // Peer2Peer
  EXPECT_TRUE(Netgame.flags & NF_PEER_PEER);
  EXPECT_FALSE(Netgame.flags & NF_PERMISSABLE);

  SetCVarInt(27, 1); // Permissable kicks out peer2peer
  EXPECT_TRUE(Netgame.flags & NF_PERMISSABLE);
  EXPECT_FALSE(Netgame.flags & NF_PEER_PEER);

  SetCVarInt(22, 1); // ...and back
  EXPECT_TRUE(Netgame.flags & NF_PEER_PEER);
  EXPECT_FALSE(Netgame.flags & NF_PERMISSABLE);

  SetCVarInt(27, 0);
  SetCVarInt(22, 0);
  EXPECT_FALSE(Netgame.flags & NF_PEER_PEER);
  EXPECT_FALSE(Netgame.flags & NF_PERMISSABLE);
}

/**
 * @test DedicatedServer.SetLevelOnlyAdvancesWhilePlaying
 * @brief Verifies set Level Only Advances While Playing.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, SetLevelOnlyAdvancesWhilePlaying) {
  ResetWorld();

  // playing: valid level advances rotation via MultiEndLevel
  NetPlayers[0].sequence = NETSEQ_PLAYING;
  Current_mission.num_levels = 5;
  SetCVarInt(33, 3); // SetLevel
  EXPECT_EQ(Multi_next_level, 3);
  ASSERT_EQ(g_order.size(), 1u);
  EXPECT_EQ(g_order[0], "endlevel");

  // playing: out-of-range level ignored entirely
  g_order.clear();
  Multi_next_level = 0;
  SetCVarInt(33, 99);
  SetCVarInt(33, 0);
  EXPECT_TRUE(g_order.empty());
  EXPECT_EQ(Multi_next_level, 0);

  // idle: nothing observable happens (no endlevel call)
  NetPlayers[0].sequence = 0;
  SetCVarInt(33, 2);
  EXPECT_TRUE(g_order.empty());
  EXPECT_EQ(Multi_next_level, 0);
}

/**
 * @test DedicatedServer.NoneCvarsRouteByPlayState
 * @brief Verifies none Cvars Route By Play State.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, NoneCvarsRouteByPlayState) {
  ResetWorld();

  // Quit only acts while playing; idle servers ignore it
  SetCVar("quit", "", false);
  EXPECT_TRUE(g_order.empty());

  NetPlayers[0].sequence = NETSEQ_PLAYING;
  SetCVar("quit", "", false);
  ASSERT_EQ(g_order.size(), 2u);
  EXPECT_EQ(g_order[0], "leavegame");
  EXPECT_EQ(g_order[1], "mode" + std::to_string((int)QUIT_MODE));

  g_order.clear();
  SetCVar("endlevel", "", false);
  ASSERT_EQ(g_order.size(), 1u);
  EXPECT_EQ(g_order[0], "endlevel");

  g_order.clear();
  SetCVar("startRtLog", "", false);
  SetCVar("stoprtlog", "", false);
  ASSERT_EQ(g_order.size(), 2u);
  EXPECT_EQ(g_order[0], "startlog");
  EXPECT_EQ(g_order[1], "stoplog");
}

/**
 * @test DedicatedServer.FloatAccessorOnlyAcceptsFloatCvars
 * @brief Verifies float Accessor Only Accepts Float Cvars.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, FloatAccessorOnlyAcceptsFloatCvars) {
  ResetWorld();

  SetCVarFloat(32, 0.5f); // AudioTauntDelay
  ASSERT_EQ(g_order.size(), 1u);
  EXPECT_EQ(g_order[0], "taunt0.500000");

  // feeding a non-float cvar through this accessor silently no-ops
  g_order.clear();
  SetCVarFloat(CX_PPS, 9.0f);
  EXPECT_TRUE(g_order.empty());
  EXPECT_EQ(Netgame.packets_per_second, 0);
}

/**
 * @test DedicatedServer.StringSaySendsFormattedToDestination
 * @brief Verifies string Say Sends Formatted To Destination.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, StringSaySendsFormattedToDestination) {
  ResetWorld();

  g_msg_dest_to = 7; // scripted destination (a player slot)
  char line[64];
  strcpy(line, "Say hello team");
  SetCVarString(CX_SAY, line + 4); // "hello team" like the console would

  ASSERT_EQ(g_sends.size(), 1u);
  EXPECT_EQ(g_sends[0].color, GR_RGB(0, 128, 255));
  EXPECT_EQ(g_sends[0].to, 7);
  EXPECT_EQ(g_sends[0].msg, "Cmdr>hello team");
}

/**
 * @test DedicatedServer.AllowDisallowTogglesObjectMultiFlag
 * @brief Verifies allow Disallow Toggles Object Multi Flag.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, AllowDisallowTogglesObjectMultiFlag) {
  ResetWorld();
  g_find_objid = 42;
  strcpy(Object_info[42].name, "Virus");

  SetCVarString(15, "Virus"); // DisallowPowerup
  EXPECT_EQ(Object_info[42].multi_allowed, 0);

  SetCVarString(16, "Virus"); // AllowPowerup
  EXPECT_EQ(Object_info[42].multi_allowed, 1);

  // unknown object names are ignored without touching anything
  Object_info[42].multi_allowed = 1;
  g_find_objid = -1;
  SetCVarString(15, "Nonsense");
  EXPECT_EQ(Object_info[42].multi_allowed, 1);
}

/**
 * @test DedicatedServer.StringCvarsCopyIntoDestinations
 * @brief Verifies string Cvars Copy Into Destinations.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, StringCvarsCopyIntoDestinations) {
  ResetWorld();

  SetCVarString(CX_GAMENAME, "My Coop Game");
  EXPECT_STREQ(Netgame.name, "My Coop Game");

  SetCVarString(19, "pxouser"); // PXOUsername -> Auto_login_name
  EXPECT_STREQ(Auto_login_name, "pxouser");

  SetCVarString(35, "welcome aboard"); // MOTD
  EXPECT_STREQ(Multi_message_of_the_day, "welcome aboard");

  // strings longer than the declared max are copied with strncpy(var_max)
  // which does NOT null-terminate -- the full 50 bytes land in the buffer
  const char *longpass = "a-very-long-password-that-exceeds-fifty-characters-in-total-length!";
  SetCVarString(20, longpass);
  EXPECT_EQ(memcmp(Auto_login_pass, longpass, MAX_AUTO_LOGIN_STUFF_LEN), 0);
}

/**
 * @test DedicatedServer.PermissionGatesSplitInitFromGameplay
 * @brief Verifies permission Gates Split Init From Gameplay.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, PermissionGatesSplitInitFromGameplay) {
  ResetWorld();
  NetPlayers[0].sequence = NETSEQ_PLAYING;

  // init-only cvar rejected during gameplay phase
  SetCVar("pps", "30", false);
  EXPECT_EQ(Netgame.packets_per_second, 0);

  // gameplay-only cvar rejected during init phase (no quit side effect)
  SetCVar("quit", "", true);
  EXPECT_TRUE(g_order.empty());

  // correct phases work
  SetCVar("pps", "30", true);
  EXPECT_EQ(Netgame.packets_per_second, 30);

  g_order.clear();
  SetCVar("quit", "", false);
  ASSERT_EQ(g_order.size(), 2u);
  EXPECT_EQ(g_order[0], "leavegame");
  EXPECT_EQ(g_order[1], "mode" + std::to_string((int)QUIT_MODE));

  // MOTD carries both permission bits
  SetCVar("motd", "hi both phases", true);
  EXPECT_STREQ(Multi_message_of_the_day, "hi both phases");
}

/**
 * @test DedicatedServer.FrameTimesOutAndQuitsWhenPacketsStop
 * @brief Verifies frame Times Out And Quits When Packets Stop.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, FrameTimesOutAndQuitsWhenPacketsStop) {
  ResetWorld();

  // under timeout threshold: nothing happens
  NetPlayers[0].sequence = NETSEQ_PLAYING;
  ServerTimeout = 1;
  LastPacketReceived = 0;
  g_now = 60.0f; // (60-0)/60 = 1 minute, not > timeout
  DoDedicatedServerFrame();
  EXPECT_TRUE(g_order.empty());

  // past the threshold while playing: quit sequence fires
  g_now = 120.0f; // 2 minutes > 1
  DoDedicatedServerFrame();
  ASSERT_GE(g_order.size(), 2u);
  EXPECT_EQ(g_order[0], "leavegame");
  EXPECT_EQ(g_order[1], "mode" + std::to_string((int)QUIT_MODE));

  // past the threshold but not playing: quit cvar does nothing
  ResetWorld();
  ServerTimeout = 1;
  NetPlayers[0].sequence = 0;
  g_now = 120.0f;
  DoDedicatedServerFrame();
  EXPECT_TRUE(g_order.empty());
}

/**
 * @test DedicatedServer.FrameRoutesConsoleCommands
 * @brief Verifies frame Routes Console Commands.
 *
 * @details
 * Exercises the DedicatedServer code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/dedicated_server.cpp
 * @ingroup descent3_tests
 */
TEST(DedicatedServer, FrameRoutesConsoleCommands) {
  ResetWorld();
  Dedicated_server = true;
  strcpy(GameArgs[1], "-dedicated");

  // '$' lines bypass the cvar parser and go straight to game DLL
  g_con_queue.push_back("$gimmie");
  DoDedicatedServerFrame();
  ASSERT_EQ(g_order.size(), 1u);
  EXPECT_EQ(g_last_gamedll_evt, EVT_CLIENT_INPUT_STRING);
  EXPECT_EQ(g_last_input_string, "$gimmie");

  // gameplay-phase commands apply: only gameplay-permitted cvars work
  // from the console (AllowPowerup carries the gameplay bit)
  g_find_objid = 42;
  strcpy(Object_info[42].name, "Virus");
  Object_info[42].multi_allowed = 0;
  g_con_queue.push_back("AllowPowerup Virus");
  DoDedicatedServerFrame();
  EXPECT_EQ(Object_info[42].multi_allowed, 1);

  // unknown commands report failure but don't crash or set anything
  Object_info[42].multi_allowed = 0;
  g_con_queue.push_back("nosuchthing 1");
  DoDedicatedServerFrame();
  EXPECT_EQ(Object_info[42].multi_allowed, 0);
  bool saw_badcommand = false;
  for (auto &line : g_console_out)
    if (line.find("badcommand") != std::string::npos)
      saw_badcommand = true;
  EXPECT_TRUE(saw_badcommand);

  // frames with no console input do nothing
  g_order.clear();
  DoDedicatedServerFrame();
  EXPECT_TRUE(g_order.empty());
}
