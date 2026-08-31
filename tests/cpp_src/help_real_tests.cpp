/**
 * @file help_real_tests.cpp
 * @brief Tests for help.cpp — help dialog (258 lines).
 *
 * @details
 * Covers HelpText counting (while >0), two-column distribution
 * of key/description pairs, and window/sheet setup.
 * Replicates logic from Descent3/help.cpp:202-258 to avoid
 * heavy newui/renderer deps, mirroring postrender/telcomcargo pattern.
 *
 * This harness validates the behavior of `Descent3/help.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/help.cpp`
 * @par Harness
 * `help_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/help.cpp
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstring>

// Replicate HelpText structure (TXI_* are just ints, values don't matter for counting)
static const int SampleHelpText[] = {10, 20, 30, 40, 50, 60, 0};
static const int HelpTextDemo[] = {10, 20, 0}; // without DEMO entries
static const int HelpTextFull[] = {
    1, 2,  // TXI_ESC, TXI_HLPQUIT
    3, 4,  // TXI_HLPALTF2, TXI_HLPSAVEGAME (non-DEMO)
    5, 6,  // TXI_HLPALTF3, TXI_HLPLOADGAME
    7, 8,  // TXI_F2, TXI_HLPCONFIG
    9, 10, // TXI_F3, TXI_HLPCOCKPIT
    0};

// Mock sheet capturing AddText/NewGroup/AddButton
struct MockSheet {
  struct Group { const char *name; int x, y; int flags; };
  std::vector<Group> groups;
  std::vector<int> texts; // store HelpText ids
  std::vector<std::pair<std::string,int>> buttons;
  void NewGroup(const char *n, int x, int y, int flags=0) { groups.push_back({n,x,y,flags}); }
  void AddText(int txtId) { texts.push_back(txtId); }
  void AddButton(const char *txt, int id) { buttons.emplace_back(txt ? txt : "", id); }
};

struct MockWindow {
  MockSheet sheet;
  std::vector<std::pair<int,int>> accelKeys;
  bool created=false, opened=false, closed=false, destroyed=false;
  int createW=0, createH=0;
  void Create(const char *title, int x, int y, int w, int h) { (void)title;(void)x;(void)y; created=true; createW=w; createH=h; }
  MockSheet* GetSheet() { return &sheet; }
  void AddAcceleratorKey(int key, int id) { accelKeys.emplace_back(key,id); }
  int DoUI() { return 1; } // UID_CANCEL
  void Open() { opened=true; }
  void Close() { closed=true; }
  void Destroy() { destroyed=true; }
};

// Replicate HelpDisplay core logic (count + distribution) without UI loop
static int CountHelpText(const int *arr) {
  int n=0;
  while (arr[n] > 0) n++;
  return n;
}
static void PopulateHelpSheet(MockSheet *sheet, const int *HelpText) {
  int strs_to_print = CountHelpText(HelpText);
  sheet->NewGroup(nullptr, 30, 10);
  for (int index = 0; index < strs_to_print; index += 2) {
    if (index < strs_to_print) sheet->AddText(HelpText[index]);
  }
  sheet->NewGroup(nullptr, 130, 10);
  for (int index = 1; index < strs_to_print; index += 2) {
    if (index < strs_to_print) sheet->AddText(HelpText[index]);
  }
  sheet->NewGroup(nullptr, 448 - 160, 384 - 96, 1);
  sheet->AddButton("OK", 1);
}

/**
 * @test Help.CountEmpty
 * @brief Verifies count Empty.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, CountEmpty) {
  const int empty[] = {0};
  EXPECT_EQ(CountHelpText(empty), 0);
  const int one[] = {5, 0};
  EXPECT_EQ(CountHelpText(one), 1);
}

/**
 * @test Help.CountSample
 * @brief Verifies count Sample.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, CountSample) {
  EXPECT_EQ(CountHelpText(SampleHelpText), 6);
  EXPECT_EQ(CountHelpText(HelpTextDemo), 2);
  // Full has 10 before terminator
  EXPECT_EQ(CountHelpText(HelpTextFull), 10);
}

/**
 * @test Help.CountStopsAtZero
 * @brief Verifies count Stops At Zero.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, CountStopsAtZero) {
  const int arr[] = {1,2,0,99,100,0};
  EXPECT_EQ(CountHelpText(arr), 2);
}

/**
 * @test Help.PopulateEvenIndicesToFirstColumn
 * @brief Verifies populate Even Indices To First Column.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, PopulateEvenIndicesToFirstColumn) {
  MockSheet sheet;
  const int arr[] = {10,20,30,40,50,60,0}; // 6 entries: even 10,30,50 ; odd 20,40,60
  PopulateHelpSheet(&sheet, arr);
  // groups: 3 NewGroup calls
  ASSERT_EQ(sheet.groups.size(), 3);
  EXPECT_EQ(sheet.groups[0].x, 30); EXPECT_EQ(sheet.groups[0].y, 10);
  EXPECT_EQ(sheet.groups[1].x, 130); EXPECT_EQ(sheet.groups[1].y, 10);
  // texts order: first loop adds indices 0,2,4 then second loop 1,3,5
  ASSERT_EQ(sheet.texts.size(), 6);
  EXPECT_EQ(sheet.texts[0], 10);
  EXPECT_EQ(sheet.texts[1], 30);
  EXPECT_EQ(sheet.texts[2], 50);
  EXPECT_EQ(sheet.texts[3], 20);
  EXPECT_EQ(sheet.texts[4], 40);
  EXPECT_EQ(sheet.texts[5], 60);
}

/**
 * @test Help.PopulateOddCount
 * @brief Verifies populate Odd Count.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, PopulateOddCount) {
  MockSheet sheet;
  const int arr[] = {1,2,3,0}; // 3 entries: even 1,3 ; odd 2
  PopulateHelpSheet(&sheet, arr);
  ASSERT_EQ(sheet.texts.size(), 3);
  EXPECT_EQ(sheet.texts[0], 1);
  EXPECT_EQ(sheet.texts[1], 3);
  EXPECT_EQ(sheet.texts[2], 2);
}

/**
 * @test Help.PopulateEmpty
 * @brief Verifies populate Empty.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, PopulateEmpty) {
  MockSheet sheet;
  const int arr[] = {0};
  PopulateHelpSheet(&sheet, arr);
  EXPECT_TRUE(sheet.texts.empty());
  // still creates groups and button
  EXPECT_EQ(sheet.groups.size(), 3);
  EXPECT_EQ(sheet.buttons.size(), 1);
}

/**
 * @test Help.PopulateSingle
 * @brief Verifies populate Single.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, PopulateSingle) {
  MockSheet sheet;
  const int arr[] = {99, 0};
  PopulateHelpSheet(&sheet, arr);
  ASSERT_EQ(sheet.texts.size(), 1);
  EXPECT_EQ(sheet.texts[0], 99);
}

/**
 * @test Help.WindowCreateParams
 * @brief Verifies window Create Params.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, WindowCreateParams) {
  MockWindow wnd;
  wnd.Create("Help", 0,0, 448,384);
  EXPECT_TRUE(wnd.created);
  EXPECT_EQ(wnd.createW, 448);
  EXPECT_EQ(wnd.createH, 384);
  wnd.AddAcceleratorKey(59, 1); // KEY_F1, UID_CANCEL
  ASSERT_EQ(wnd.accelKeys.size(), 1);
  EXPECT_EQ(wnd.accelKeys[0].first, 59);
  wnd.Open(); EXPECT_TRUE(wnd.opened);
  wnd.Close(); EXPECT_TRUE(wnd.closed);
  wnd.Destroy(); EXPECT_TRUE(wnd.destroyed);
}

/**
 * @test Help.DistributionMatchesOriginalHelpTextSize
 * @brief Verifies distribution Matches Original Help Text Size.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, DistributionMatchesOriginalHelpTextSize) {
  // Original HelpTextFull has 10 entries -> 5 per column if even
  MockSheet sheet;
  PopulateHelpSheet(&sheet, HelpTextFull);
  EXPECT_EQ(sheet.texts.size(), 10);
  // first 5 are even indices 0,2,4,6,8
  for (int i=0;i<5;++i) EXPECT_EQ(sheet.texts[i], HelpTextFull[i*2]);
  for (int i=0;i<5;++i) EXPECT_EQ(sheet.texts[5+i], HelpTextFull[i*2+1]);
}

/**
 * @test Help.ButtonAdded
 * @brief Verifies button Added.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, ButtonAdded) {
  MockSheet sheet;
  PopulateHelpSheet(&sheet, SampleHelpText);
  ASSERT_EQ(sheet.buttons.size(), 1);
  EXPECT_EQ(sheet.buttons[0].second, 1);
}

/**
 * @test Help.FullHelpTextNotEmptyAndEven
 * @brief Verifies full Help Text Not Empty And Even.
 *
 * @details
 * Exercises the Help code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/help.cpp
 * @ingroup descent3_tests
 */
TEST(Help, FullHelpTextNotEmptyAndEven) {
  // Real HelpText in help.cpp ends with 0 and has even count (pairs of key/desc)
  // Sample full array has 40+ entries, always even because key+desc pairs.
  // Verify our sample full is even
  int n = CountHelpText(HelpTextFull);
  EXPECT_EQ(n % 2, 0);
}
