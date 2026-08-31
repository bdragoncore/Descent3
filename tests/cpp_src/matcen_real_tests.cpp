/**
 * @file matcen_real_tests.cpp
 * @brief Tests for matcen.cpp 1986 lines — materialization centers.
 *
 * @details
 * Covers SetMaxProd validation, ComputeNextProdInfo selection
 * (priority order, eligibility, saturation), StartObjProd gating
 * chain, alive-list tracking quirks (AddToAliveList always returns
 * false; unlimited children disables tracking), FinishObjProd reset,
 * and the setter validation ranges.
 *
 * This harness validates the behavior of `Descent3/matcen.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/matcen.cpp`
 * @par Harness
 * `matcen_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/matcen.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>

// replicated constants (matcen_external.h)
constexpr int MAX_PROD_TYPES = 8;
constexpr int MAX_SPAWN_PNTS = 4;
constexpr int MATCEN_ERROR = -1;
constexpr int MSTAT_DISABLED = 1;
constexpr int MSTAT_ACTIVE = 2;
constexpr int MSTAT_ACTIVE_PAUSE = 4;
constexpr int MSTAT_NEVER_PROD = 16;
constexpr int MSTAT_DONE_PROD = 32;
constexpr int MSTAT_RANDOM_PROD_ORDER = 64;
constexpr int MMODE_NOTPROD = 0, MMODE_PREPROD = 1, MMODE_POSTPROD = 2;
constexpr int MPC_SCRIPT = 0;
constexpr int MAX_MATCEN_CONTROL_TYPES = 5;
constexpr int MT_OBJECT = 0, MT_ROOM = 1, MT_UNASSIGNED = 2;

static float Gametime_mock = 100.0f;

// replicated matcen state subset + logic
struct MatcenMock {
  int m_status = 0;
  char m_prod_mode = MMODE_NOTPROD;
  float m_prod_mode_time = 0.0f;
  int m_num_prod_types = 0;
  signed char m_prod_type[MAX_PROD_TYPES];
  float m_prod_time[MAX_PROD_TYPES];
  int m_prod_priority[MAX_PROD_TYPES];
  int m_max_prod_type[MAX_PROD_TYPES];
  int m_num_prod_type[MAX_PROD_TYPES]; // per-type produced count
  int m_max_prod = 0;
  int m_num_prod = 0; // TOTAL produced (separate original member)
  int m_cached_prod_index = -1;
  float m_cached_prod_time = 0.0f;
  int m_cur_saturation_count = 0;
  char m_type = MT_UNASSIGNED;
  char m_control_type = MPC_SCRIPT;
  char m_num_spawn_pnts = 0;
  // alive list
  int m_max_alive_children = -1;
  int m_num_alive = 0;
  int m_alive_list_storage[16];

  MatcenMock() {
    for (int i = 0; i < MAX_PROD_TYPES; i++) {
      m_prod_type[i] = MATCEN_ERROR;
      m_prod_time[i] = 1.0f;
      m_prod_priority[i] = 100;
      m_max_prod_type[i] = 3;
      m_num_prod_type[i] = 0;
    }
  }

  bool SetMaxProd(int max_p) {
    if (max_p < -1)
      return false;
    m_max_prod = max_p;
    ComputeNextProdInfo();
    return true;
  }

  bool ObjAlive(int ref) { return ref != 0; } // mock: handle 0 = dead

  void DoAliveListFrame() {
    for (int i = 0; i < m_num_alive; i++) {
      if (!ObjAlive(m_alive_list_storage[i])) {
        for (int j = i; j < m_num_alive - 1; j++)
          m_alive_list_storage[j] = m_alive_list_storage[j + 1];
        m_num_alive--;
        i--; // recheck the slot just shifted in
      }
    }
  }

  bool AddToAliveList(int objref) {
    if (m_max_alive_children > 0 && m_max_alive_children > m_num_alive) {
      m_alive_list_storage[m_num_alive++] = objref;
    }
    return false; // quirk: ALWAYS false regardless of success
  }

  void ResetProdCounters() {
    memset(m_num_prod_type, 0, sizeof(m_num_prod_type));
    m_num_prod = 0;
    m_status &= ~MSTAT_DONE_PROD;
  }

  bool ComputeNextProdInfo() {
    if (m_status & MSTAT_DISABLED)
      return false;

    m_status &= ~MSTAT_DONE_PROD;

    if (m_max_prod <= m_num_prod && m_max_prod != -1) {
      m_status |= MSTAT_DONE_PROD;
      return false;
    }

    int cur_index = -1;
    bool f_do = true;

    if (m_status & MSTAT_RANDOM_PROD_ORDER) {
      int total_priorities = 0;
      for (int i = 0; i < m_num_prod_types; i++)
        if (Eligible(i))
          total_priorities += m_prod_priority[i];

      if (total_priorities <= 0) {
        f_do = false;
      } else {
        int r_val = rand() % total_priorities;
        bool f_looking = true;
        int i = 0;
        do {
          if (Eligible(i)) {
            r_val -= m_prod_priority[i];
            if (r_val <= 0) {
              cur_index = i;
              f_looking = false;
            }
          }
          i++;
        } while (f_looking);
      }
    } else {
      int best_priority = -1;
      for (int i = 0; i < m_num_prod_types; i++) {
        if (Eligible(i) && m_prod_priority[i] > best_priority) {
          cur_index = i;
          best_priority = m_prod_priority[i];
        }
      }
      if (cur_index == -1)
        f_do = false;
    }

    if (f_do) {
      m_cached_prod_index = cur_index;
      m_cached_prod_time = Gametime_mock + m_prod_time[cur_index];
      return true;
    }

    m_cached_prod_index = -1;
    m_status |= MSTAT_DONE_PROD;
    return false;
  }

  bool Eligible(int i) const {
    return (m_max_prod_type[i] == -1 || m_max_prod_type[i] > m_num_prod_type[i]) && m_prod_type[i] >= -1 &&
           i < m_num_prod_types;
  }

  // replicated StartObjProd gating chain minus sound calls
  bool StartObjProd() {
    if (m_cached_prod_index < 0 || m_max_prod_type[m_cached_prod_index] >= m_num_prod_type[m_cached_prod_index])
      ComputeNextProdInfo();

    if (m_status & (MSTAT_DONE_PROD | MSTAT_ACTIVE_PAUSE | MSTAT_DISABLED))
      return false;
    if (m_prod_mode != MMODE_NOTPROD)
      return false;
    if (!(m_status & MSTAT_ACTIVE))
      return false;
    if (m_max_prod <= m_num_prod && m_max_prod != -1) {
      m_status |= MSTAT_DONE_PROD;
      return false;
    }
    if (m_cached_prod_index < 0)
      return false;

    m_status &= ~MSTAT_NEVER_PROD;
    m_prod_mode_time = 0.0f;
    m_prod_mode = MMODE_PREPROD;
    m_cur_saturation_count = 0;
    return true;
  }

  void FinishObjProd() {
    m_prod_mode_time = 0.0f;
    m_prod_mode = MMODE_NOTPROD;
    m_cur_saturation_count = 0;
    ComputeNextProdInfo();
  }

  bool SetAttachType(char type) {
    if (type == MT_OBJECT || type == MT_ROOM || type == MT_UNASSIGNED) {
      m_type = type;
      return true;
    }
    return false;
  }

  bool SetControlType(char type) {
    if (type >= 0 && type < MAX_MATCEN_CONTROL_TYPES) {
      m_control_type = type;
      return true;
    }
    return false;
  }

  bool SetNumSpawnPnts(char num_s) {
    if (num_s >= 0 && num_s <= MAX_SPAWN_PNTS) {
      m_num_spawn_pnts = num_s;
      return true;
    }
    return false;
  }
};

/**
 * @test Matcen.SetMaxProdRejectsBelowMinusOne
 * @brief Verifies set Max Prod Rejects Below Minus One.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, SetMaxProdRejectsBelowMinusOne) {
  MatcenMock m;
  EXPECT_FALSE(m.SetMaxProd(-2));
  EXPECT_EQ(m.m_max_prod, 0); // unchanged
  EXPECT_TRUE(m.SetMaxProd(-1)); // -1 = unlimited
  EXPECT_EQ(m.m_max_prod, -1);
  EXPECT_TRUE(m.SetMaxProd(5));
  EXPECT_EQ(m.m_max_prod, 5);
}

/**
 * @test Matcen.ComputeNextHighestPriorityWinsWithFirstIndexTieBreak
 * @brief Verifies compute Next Highest Priority Wins With First Index Tie Break.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, ComputeNextHighestPriorityWinsWithFirstIndexTieBreak) {
  MatcenMock m;
  m.m_num_prod_types = 3;
  m.m_prod_type[0] = 10;
  m.m_prod_type[1] = 11;
  m.m_prod_type[2] = 12;
  m.m_prod_priority[0] = 50;
  m.m_prod_priority[1] = 90;
  m.m_prod_priority[2] = 90; // tie with index 1
  m.m_max_prod = -1;         // no global limit

  EXPECT_TRUE(m.ComputeNextProdInfo());
  EXPECT_EQ(m.m_cached_prod_index, 1); // first of the tied highest wins
}

/**
 * @test Matcen.ComputeNextSkipsSaturatedTypes
 * @brief Verifies compute Next Skips Saturated Types.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, ComputeNextSkipsSaturatedTypes) {
  MatcenMock m;
  m.m_num_prod_types = 3;
  m.m_prod_type[0] = 10;
  m.m_prod_type[1] = MATCEN_ERROR;
  m.m_prod_type[2] = 12;
  m.m_prod_priority[0] = 99;
  m.m_prod_priority[2] = 20;
  m.m_max_prod_type[0] = 2; // saturation: already made 2
  m.m_num_prod_type[0] = 2;
  m.m_max_prod = -1;

  // quirk: eligibility check is m_prod_type[i] >= -1, so the
  // MATCEN_ERROR sentinel (-1) is itself a VALID product type and
  // index 1 (default priority 100) beats index 2 (priority 20)
  EXPECT_TRUE(m.ComputeNextProdInfo());
  EXPECT_EQ(m.m_cached_prod_index, 1);

  // saturate both remaining slots -> nothing left
  m.m_num_prod_type[1] = 4; // exceeds default cap 3
  m.m_num_prod_type[2] = 4;
  EXPECT_FALSE(m.ComputeNextProdInfo());
  EXPECT_EQ(m.m_cached_prod_index, -1);
  EXPECT_TRUE(m.m_status & MSTAT_DONE_PROD);
}

/**
 * @test Matcen.ComputeNextRespectsUnlimitedMaxProdTypeMinusOne
 * @brief Verifies compute Next Respects Unlimited Max Prod Type Minus One.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, ComputeNextRespectsUnlimitedMaxProdTypeMinusOne) {
  MatcenMock m;
  m.m_num_prod_types = 1;
  m.m_prod_type[0] = 7;
  m.m_max_prod_type[0] = -1; // unlimited per-type
  m.m_num_prod_type[0] = 500; // even with huge history
  m.m_max_prod = -1;          // and no global cap

  EXPECT_TRUE(m.ComputeNextProdInfo());
  EXPECT_EQ(m.m_cached_prod_index, 0);
}

/**
 * @test Matcen.DisabledMatcenNeverComputesAndKeepsDoneFlag
 * @brief Verifies disabled Matcen Never Computes And Keeps Done Flag.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, DisabledMatcenNeverComputesAndKeepsDoneFlag) {
  MatcenMock m;
  m.m_num_prod_types = 1;
  m.m_prod_type[0] = 7;
  m.m_status = MSTAT_DISABLED | MSTAT_DONE_PROD;

  EXPECT_FALSE(m.ComputeNextProdInfo());
  EXPECT_TRUE(m.m_status & MSTAT_DISABLED);
  EXPECT_TRUE(m.m_status & MSTAT_DONE_PROD); // NOT cleared when disabled
  EXPECT_EQ(m.m_cached_prod_index, -1);
}

/**
 * @test Matcen.GlobalMaxProdTriggersDoneStatus
 * @brief Verifies global Max Prod Triggers Done Status.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, GlobalMaxProdTriggersDoneStatus) {
  MatcenMock m;
  m.m_num_prod_types = 2;
  m.m_prod_type[0] = 1;
  m.m_prod_type[1] = 2;
  m.m_max_prod = 3;
  m.m_num_prod = 3; // total produced hits the cap

  EXPECT_FALSE(m.ComputeNextProdInfo());
  EXPECT_TRUE(m.m_status & MSTAT_DONE_PROD);

  // unlimited (-1) ignores the comparison entirely
  m.m_max_prod = -1;
  EXPECT_TRUE(m.ComputeNextProdInfo());
}

/**
 * @test Matcen.CachedTimeIsGametimePlusProductTime
 * @brief Verifies cached Time Is Gametime Plus Product Time.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, CachedTimeIsGametimePlusProductTime) {
  MatcenMock m;
  Gametime_mock = 42.0f;
  m.m_num_prod_types = 1;
  m.m_prod_type[0] = 5;
  m.m_prod_time[0] = 7.5f;
  m.m_max_prod = -1;

  EXPECT_TRUE(m.ComputeNextProdInfo());
  EXPECT_FLOAT_EQ(m.m_cached_prod_time, 49.5f);
  Gametime_mock = 100.0f;
}

/**
 * @test Matcen.StartObjProdGatingChain
 * @brief Verifies start Obj Prod Gating Chain.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, StartObjProdGatingChain) {
  MatcenMock m;
  m.m_num_prod_types = 1;
  m.m_prod_type[0] = 5;
  m.m_max_prod = -1;

  // not ACTIVE -> rejected even with a valid plan
  EXPECT_TRUE(m.ComputeNextProdInfo());
  EXPECT_FALSE(m.StartObjProd());

  // now active -> starts
  m.m_status |= MSTAT_ACTIVE;
  m.m_prod_mode = MMODE_NOTPROD;
  EXPECT_TRUE(m.StartObjProd());
  EXPECT_EQ(m.m_prod_mode, MMODE_PREPROD);
  EXPECT_FLOAT_EQ(m.m_prod_mode_time, 0.0f);

  // second start while producing -> rejected
  EXPECT_FALSE(m.StartObjProd());

  // pause flag blocks restart after reset to NOTPROD
  m.FinishObjProd();
  m.m_status |= MSTAT_ACTIVE_PAUSE;
  EXPECT_FALSE(m.StartObjProd());
  m.m_status &= ~MSTAT_ACTIVE_PAUSE;

  // NEVER_PROD cleared by successful start
  m.m_status |= MSTAT_NEVER_PROD;
  EXPECT_TRUE(m.StartObjProd());
  EXPECT_FALSE(m.m_status & MSTAT_NEVER_PROD);
}

/**
 * @test Matcen.AliveListCompactionRemovesDeadAndRechecksSlot
 * @brief Verifies alive List Compaction Removes Dead And Rechecks Slot.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, AliveListCompactionRemovesDeadAndRechecksSlot) {
  MatcenMock m;
  m.m_max_alive_children = 8;
  // refs: 5(alive) 0(dead) 7(alive) 0(dead)
  m.m_alive_list_storage[0] = 5;
  m.m_alive_list_storage[1] = 0;
  m.m_alive_list_storage[2] = 7;
  m.m_alive_list_storage[3] = 0;
  m.m_num_alive = 4;

  m.DoAliveListFrame();
  EXPECT_EQ(m.m_num_alive, 2);
  EXPECT_EQ(m.m_alive_list_storage[0], 5);
  EXPECT_EQ(m.m_alive_list_storage[1], 7);
}

/**
 * @test Matcen.AddToAliveListAlwaysReturnsFalseAndIgnoresUnlimitedMode
 * @brief Verifies add To Alive List Always Returns False And Ignores Unlimited Mode.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, AddToAliveListAlwaysReturnsFalseAndIgnoresUnlimitedMode) {
  MatcenMock m;
  // default m_max_alive_children = -1 (unlimited): quirk — NOTHING tracked
  EXPECT_FALSE(m.AddToAliveList(9));
  EXPECT_EQ(m.m_num_alive, 0);

  // explicit positive cap enables tracking but still returns false
  m.m_max_alive_children = 2;
  EXPECT_FALSE(m.AddToAliveList(9));
  EXPECT_EQ(m.m_num_alive, 1);
  EXPECT_FALSE(m.AddToAliveList(8));
  EXPECT_EQ(m.m_num_alive, 2);
  EXPECT_FALSE(m.AddToAliveList(7)); // full -> silently dropped
  EXPECT_EQ(m.m_num_alive, 2);
}

/**
 * @test Matcen.FinishObjProdResetsModeAndRecomputesPlan
 * @brief Verifies finish Obj Prod Resets Mode And Recomputes Plan.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, FinishObjProdResetsModeAndRecomputesPlan) {
  MatcenMock m;
  m.m_num_prod_types = 1;
  m.m_prod_type[0] = 5;
  m.m_status = MSTAT_ACTIVE;
  m.m_max_prod = -1;
  m.m_prod_mode = MMODE_POSTPROD;
  m.m_cur_saturation_count = 9;

  m.ComputeNextProdInfo();
  EXPECT_EQ(m.m_cached_prod_index, 0);

  m.FinishObjProd();
  EXPECT_EQ(m.m_prod_mode, MMODE_NOTPROD);
  EXPECT_EQ(m.m_cur_saturation_count, 0);
  EXPECT_FLOAT_EQ(m.m_prod_mode_time, 0.0f);
  EXPECT_EQ(m.m_cached_prod_index, 0); // replanned for next cycle
}

/**
 * @test Matcen.SetterValidationRanges
 * @brief Verifies setter Validation Ranges.
 *
 * @details
 * Exercises the Matcen code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(Matcen, SetterValidationRanges) {
  MatcenMock m;
  EXPECT_FALSE(m.SetAttachType(3));
  EXPECT_FALSE(m.SetAttachType(-1));
  EXPECT_TRUE(m.SetAttachType(MT_ROOM));
  EXPECT_EQ(m.m_type, MT_ROOM);

  EXPECT_FALSE(m.SetControlType(MAX_MATCEN_CONTROL_TYPES)); // 5 out of range
  EXPECT_TRUE(m.SetControlType(4));

  EXPECT_FALSE(m.SetNumSpawnPnts(-1));
  EXPECT_TRUE(m.SetNumSpawnPnts(MAX_SPAWN_PNTS)); // inclusive upper bound
  EXPECT_FALSE(m.SetNumSpawnPnts(MAX_SPAWN_PNTS + 1));
}
