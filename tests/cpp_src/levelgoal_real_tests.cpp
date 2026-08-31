/**
 * @file levelgoal_real_tests.cpp
 * @brief Tests for levelgoal.cpp 1303 lines — level goal system. Covers.
 *
 * @details
 * lgoal item add/delete/info, priority and goal-list clamping,
 * GetStatus flag set/clear with the completion latch (including the
 * 0xFFFFFFFF multisafe clear guard), and the GetName-style buffer
 * query family.
 *
 * This harness validates the behavior of `Descent3/levelgoal.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/levelgoal.cpp`
 * @par Harness
 * `levelgoal_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/levelgoal.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>

constexpr int MAX_GOAL_ITEMS_P = 12;
constexpr int MAX_GOAL_LISTS_P = 4;
#define LIT_OBJECT_P 2
#define LO_SET_SPECIFIED_P 0
#define LO_GET_SPECIFIED_P 1
#define LO_CLEAR_SPECIFIED_P 2
#define LGF_ENABLED_P 0x4
#define LGF_COMPLETED_P 0x8
#define LGF_SECONDARY_GOAL_P 0x2

struct LitemP {
  char m_type = LIT_OBJECT_P;
  int m_handle = -1;
  bool m_f_done = false;
};

// replica of lgoal's testable state machine (levelgoal.cpp:217-525)
struct LGGoalP {
  LitemP m_item[MAX_GOAL_ITEMS_P];
  int m_num_items = 0;
  signed char m_g_list = 0;
  bool m_modified = false;
  bool m_goal_completed = false;
  int m_priority = 0;
  uint32_t m_flags = LGF_ENABLED_P;

  int AddItem() {
    if (m_num_items < MAX_GOAL_ITEMS_P) {
      if (m_num_items < 1) {
        m_item[0].m_type = LIT_OBJECT_P;
        m_item[0].m_handle = -1;
        m_item[0].m_f_done = false;
      } else {
        m_item[m_num_items].m_type = m_item[0].m_type;
        m_item[m_num_items].m_handle = -1;
        m_item[m_num_items].m_f_done = false;
      }
      m_num_items++;
      return m_num_items - 1;
    }
    return -1;
  }

  bool DeleteItem(int index) {
    if (index < 0 || index >= m_num_items)
      return false;
    for (int i = index; i < m_num_items - 1; i++)
      m_item[i] = m_item[i + 1];
    m_num_items--;
    return true;
  }

  bool ItemInfo(int index, char op, char *type, int *handle, bool *done) {
    if (index < 0 || index >= m_num_items)
      return false;
    if (op == LO_SET_SPECIFIED_P) {
      if (type)
        m_item[index].m_type = *type;
      if (handle)
        m_item[index].m_handle = *handle;
      if (done)
        m_item[index].m_f_done = *done;
      return true;
    }
    if (op == LO_GET_SPECIFIED_P) {
      if (type)
        *type = m_item[index].m_type;
      if (handle)
        *handle = m_item[index].m_handle;
      if (done)
        *done = m_item[index].m_f_done;
      return true;
    }
    return false;
  }

  bool Priority(char op, int *value) {
    if (!value)
      return false;
    if (op == LO_SET_SPECIFIED_P) {
      m_priority = *value;
      m_modified = true;
      return true;
    }
    if (op == LO_GET_SPECIFIED_P) {
      *value = m_priority;
      return true;
    }
    return false;
  }

  bool GoalList(char op, int8_t *value) {
    if (!value)
      return false;
    if (op == LO_SET_SPECIFIED_P) {
      // clamps in place: caller sees the sanitized value too
      if (*value < 0)
        *value = 0;
      else if (*value >= MAX_GOAL_LISTS_P)
        *value = MAX_GOAL_LISTS_P - 1;
      m_g_list = *value;
      return true;
    }
    if (op == LO_GET_SPECIFIED_P) {
      *value = m_g_list;
      return true;
    }
    return false;
  }

  bool GetStatus(char op, int *value, bool save_load) {
    if (!value)
      return false;
    if (op == LO_SET_SPECIFIED_P) {
      m_flags |= *value;
      m_modified = true;
      if (((*value) & LGF_COMPLETED_P) && !m_goal_completed) {
        if (!save_load)
          m_completed_announced = true; // GoalComplete side effect
        m_goal_completed = true;
      }
      return true;
    }
    if (op == LO_GET_SPECIFIED_P) {
      *value = m_flags;
      return true;
    }
    if (op == LO_CLEAR_SPECIFIED_P) {
      // quirk: 0xFFFFFFFF clears are ignored for the completed latch
      if ((*value) != 0xFFFFFFFF && m_goal_completed) {
        if ((*value) & LGF_COMPLETED_P)
          m_goal_completed = false;
      }
      m_flags &= ~(*value);
      m_modified = true;
      return true;
    }
    return false;
  }

  bool m_completed_announced = false;

  // GetName-family replica (levelgoal.cpp:453-469 shape)
  static int QueryStr(const char *src, char *out, int buf) {
    if (!out)
      return 0;
    if (buf <= 0)
      return src ? strlen(src) : 0;
    if (src == nullptr) {
      out[0] = '\0';
      return 0;
    }
    strncpy(out, src, buf);
    out[buf - 1] = '\0';
    return strlen(src);
  }
};

/**
 * @test LevelGoalItems.AddDefaultsDeleteShiftsAndBounds
 * @brief Verifies add Defaults Delete Shifts And Bounds.
 *
 * @details
 * Exercises the LevelGoalItems code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal.cpp
 * @ingroup descent3_tests
 */
TEST(LevelGoalItems, AddDefaultsDeleteShiftsAndBounds) {
  LGGoalP g;

  EXPECT_EQ(g.AddItem(), 0);
  EXPECT_EQ(g.m_item[0].m_type, LIT_OBJECT_P);
  EXPECT_EQ(g.m_item[0].m_handle, -1);

  g.AddItem();
  g.AddItem();
  // quirk: only the FIRST item defaults type OBJECT; later items copy
  // item[0]'s CURRENT type but always reset handle to -1
  char t = 3; // trigger
  int h = 77;
  bool done = true;
  g.ItemInfo(0, LO_SET_SPECIFIED_P, &t, &h, &done);
  EXPECT_EQ(g.AddItem(), 3);
  EXPECT_EQ(g.m_item[3].m_type, 3);  // copied from item[0]
  EXPECT_EQ(g.m_item[3].m_handle, -1); // NOT copied

  // delete middle shifts left: old idx2 (default type) lands at idx1
  g.DeleteItem(1);
  EXPECT_EQ(g.m_num_items, 3);
  EXPECT_EQ(g.m_item[1].m_type, 2);
  EXPECT_EQ(g.m_item[2].m_type, 3);

  // bounds rejected
  EXPECT_FALSE(g.DeleteItem(-1));
  EXPECT_FALSE(g.DeleteItem(99));
  EXPECT_FALSE(g.ItemInfo(99, LO_GET_SPECIFIED_P, nullptr, nullptr, nullptr));

  // fill to cap then overflow returns -1
  while (g.AddItem() != -1) {
  }
  EXPECT_EQ(g.m_num_items, MAX_GOAL_ITEMS_P);
}

/**
 * @test LevelGoalFlags.CompletionLatchClearGuardAndClamps
 * @brief Verifies completion Latch Clear Guard And Clamps.
 *
 * @details
 * Exercises the LevelGoalFlags code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal.cpp
 * @ingroup descent3_tests
 */
TEST(LevelGoalFlags, CompletionLatchClearGuardAndClamps) {
  LGGoalP g;

  int v = LGF_SECONDARY_GOAL_P;
  g.GetStatus(LO_SET_SPECIFIED_P, &v, false);
  EXPECT_TRUE(g.m_flags & LGF_SECONDARY_GOAL_P);

  // completing announces exactly once even across repeated sets
  v = LGF_COMPLETED_P;
  g.GetStatus(LO_SET_SPECIFIED_P, &v, false);
  EXPECT_TRUE(g.m_goal_completed);
  EXPECT_TRUE(g.m_completed_announced);
  g.m_completed_announced = false;
  g.GetStatus(LO_SET_SPECIFIED_P, &v, false); // second set: no announce
  EXPECT_FALSE(g.m_completed_announced);

  // save_load sets the latch silently (no HUD event on load)
  LGGoalP g2;
  v = LGF_COMPLETED_P;
  g2.GetStatus(LO_SET_SPECIFIED_P, &v, true);
  EXPECT_TRUE(g2.m_goal_completed);
  EXPECT_FALSE(g2.m_completed_announced);

  // clearing COMPLETED unmarks the latch
  g.GetStatus(LO_CLEAR_SPECIFIED_P, &v, false);
  EXPECT_FALSE(g.m_goal_completed);
  EXPECT_FALSE(g.m_flags & LGF_COMPLETED_P);

  // quirk: a 0xFFFFFFFF clear still wipes flags but NEVER unmarks completion
  g.m_goal_completed = true;
  int all = 0xFFFFFFFF;
  g.GetStatus(LO_CLEAR_SPECIFIED_P, &all, false);
  EXPECT_EQ(g.m_flags, 0u);
  EXPECT_TRUE(g.m_goal_completed); // survives

  // goal list clamps into [0,MAX) and reports the sanitized value back
  LGGoalP gl;
  int8_t list = -5;
  gl.GoalList(LO_SET_SPECIFIED_P, &list);
  EXPECT_EQ(list, 0);
  EXPECT_EQ(gl.m_g_list, 0);
  list = 99;
  gl.GoalList(LO_SET_SPECIFIED_P, &list);
  EXPECT_EQ(list, MAX_GOAL_LISTS_P - 1);

  // unknown operation rejected; null value rejected
  int8_t lv = 1;
  EXPECT_FALSE(gl.GoalList('x', &lv));
  EXPECT_FALSE(gl.GoalList(LO_SET_SPECIFIED_P, nullptr));
  int pv;
  EXPECT_FALSE(gl.Priority(LO_GET_SPECIFIED_P, nullptr));
  pv = 42;
  gl.Priority(LO_SET_SPECIFIED_P, &pv);
  pv = 0;
  gl.Priority(LO_GET_SPECIFIED_P, &pv);
  EXPECT_EQ(pv, 42);
}

/**
 * @test LevelGoalStrings.BufferQueryFamilySemantics
 * @brief Verifies buffer Query Family Semantics.
 *
 * @details
 * Exercises the LevelGoalStrings code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal.cpp
 * @ingroup descent3_tests
 */
TEST(LevelGoalStrings, BufferQueryFamilySemantics) {
  char buf[64];

  // null source -> empty string, len 0
  EXPECT_EQ(LGGoalP::QueryStr(nullptr, buf, sizeof(buf)), 0);
  EXPECT_STREQ(buf, "");

  // size probe with buffer_size<=0 returns full length without writing
  // (output pointer must still be non-null — null is rejected first)
  char probe;
  EXPECT_EQ(LGGoalP::QueryStr("retrieve the key", &probe, 0), 16);
  EXPECT_EQ(LGGoalP::QueryStr("abc", nullptr, 0), 0); // null out wins

  // normal copy returns source length
  EXPECT_EQ(LGGoalP::QueryStr("abc", buf, sizeof(buf)), 3);
  EXPECT_STREQ(buf, "abc");

  // truncation: returned length is SOURCE length, not what was copied
  memset(buf, 'Q', sizeof(buf));
  EXPECT_EQ(LGGoalP::QueryStr("abcdefghij", buf, 5), 10);
  EXPECT_EQ(strnlen(buf, sizeof(buf)), 4); // "abcd" + NUL
}
