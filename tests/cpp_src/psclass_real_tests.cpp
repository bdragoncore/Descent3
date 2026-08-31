/**
 * @file psclass_real_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for the real lib/psclass.h template containers (tList, tQueue).
 * The header is self-contained (no game dependencies).
 *
 * This harness validates the behavior of `Descent3/psclass.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/psclass.cpp`
 * @par Harness
 * `psclass_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/psclass.cpp
 */

#include <gtest/gtest.h>

#include "psclass.h"

// ============================================================================
// tList — singly-linked list of dynamically allocated tListNode<T> nodes
// (each node embeds its payload in the `t` member)
// ============================================================================

/**
 * @test Psclass.tList_EmptyList
 * @brief Verifies t List Empty List.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tList_EmptyList) {
  tList<int> list;
  EXPECT_EQ(list.length(), 0);
  EXPECT_EQ(list.start(), nullptr);
  EXPECT_EQ(list.get(), nullptr);
}

/**
 * @test Psclass.tList_LinkAndIterate
 * @brief Verifies t List Link And Iterate.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tList_LinkAndIterate) {
  tList<int> list;
  auto *a = new tListNode<int>; a->t = 1;
  auto *b = new tListNode<int>; b->t = 2;
  auto *c = new tListNode<int>; c->t = 3;
  list.link(a);
  list.link(b);
  list.link(c);
  EXPECT_EQ(list.length(), 3);

  // start() moves to the first link (a), next() walks forward.
  tListNode<int> *n = list.start();
  EXPECT_EQ(n, a);
  EXPECT_EQ(n->t, 1);
  n = list.next();
  EXPECT_EQ(n, b);
  EXPECT_EQ(n->t, 2);
  n = list.next();
  EXPECT_EQ(n, c);
  EXPECT_EQ(n->t, 3);

  list.free();
  delete a; delete b; delete c;
}

/**
 * @test Psclass.tList_NextStaysAtEnd
 * @brief Verifies t List Next Stays At End.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tList_NextStaysAtEnd) {
  tList<int> list;
  auto *a = new tListNode<int>; a->t = 1;
  list.link(a);
  tListNode<int> *n = list.start();
  n = list.next(); // at end
  n = list.next(); // stays at end, does not wrap
  EXPECT_EQ(n, a);

  list.free();
  delete a;
}

/**
 * @test Psclass.tList_UnlinkMiddle
 * @brief Verifies t List Unlink Middle.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tList_UnlinkMiddle) {
  tList<int> list;
  auto *a = new tListNode<int>; a->t = 1;
  auto *b = new tListNode<int>; b->t = 2;
  auto *c = new tListNode<int>; c->t = 3;
  list.link(a);
  list.link(b);
  list.link(c);

  // Move mark to b, then unlink it.
  list.start();
  list.next();
  tListNode<int> *removed = list.unlink();
  EXPECT_EQ(removed, b);
  EXPECT_EQ(list.length(), 2);

  // Remaining order: a, c.
  tListNode<int> *n = list.start();
  EXPECT_EQ(n, a);
  n = list.next();
  EXPECT_EQ(n, c);

  list.free();
  delete a; delete b; delete c;
}

/**
 * @test Psclass.tList_UnlinkHead
 * @brief Verifies t List Unlink Head.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tList_UnlinkHead) {
  tList<int> list;
  auto *a = new tListNode<int>; a->t = 1;
  auto *b = new tListNode<int>; b->t = 2;
  list.link(a);
  list.link(b);

  list.start(); // mark at a (head)
  tListNode<int> *removed = list.unlink();
  EXPECT_EQ(removed, a);
  EXPECT_EQ(list.length(), 1);
  EXPECT_EQ(list.start(), b);

  list.free();
  delete a; delete b;
}

/**
 * @test Psclass.tList_UnlinkEmpty
 * @brief Verifies t List Unlink Empty.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tList_UnlinkEmpty) {
  tList<int> list;
  EXPECT_EQ(list.unlink(), nullptr);
}

/**
 * @test Psclass.tList_FreeDetachesAll
 * @brief Verifies t List Free Detaches All.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tList_FreeDetachesAll) {
  tList<int> list;
  auto *a = new tListNode<int>; a->t = 1;
  auto *b = new tListNode<int>; b->t = 2;
  list.link(a);
  list.link(b);
  list.free();
  EXPECT_EQ(list.length(), 0);
  EXPECT_EQ(list.start(), nullptr);
  delete a; delete b;
}

// ============================================================================
// tQueue — fixed-capacity circular queue
// ============================================================================

/**
 * @test Psclass.tQueue_SendRecvFifo
 * @brief Verifies t Queue Send Recv Fifo.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tQueue_SendRecvFifo) {
  tQueue<int, 4> q;
  int a = 1, b = 2, c = 3;
  q.send(a);
  q.send(b);
  q.send(c);

  int out = 0;
  EXPECT_TRUE(q.recv(&out));
  EXPECT_EQ(out, 1);
  EXPECT_TRUE(q.recv(&out));
  EXPECT_EQ(out, 2);
  EXPECT_TRUE(q.recv(&out));
  EXPECT_EQ(out, 3);
  EXPECT_FALSE(q.recv(&out)); // empty
}

/**
 * @test Psclass.tQueue_WrapsAround
 * @brief Verifies t Queue Wraps Around.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tQueue_WrapsAround) {
  // Ring buffer capacity is t_LEN-1 (one slot reserved to distinguish
  // full from empty).
  tQueue<int, 4> q;
  int v = 0;
  for (int i = 0; i < 3; i++) {
    v = i;
    q.send(v);
  }
  // Fill up, then drain, then fill again — exercises head/tail wraparound.
  for (int i = 0; i < 3; i++) {
    EXPECT_TRUE(q.recv(&v));
    EXPECT_EQ(v, i);
  }
  for (int i = 10; i < 13; i++) {
    v = i;
    q.send(v);
  }
  for (int i = 10; i < 13; i++) {
    EXPECT_TRUE(q.recv(&v));
    EXPECT_EQ(v, i);
  }
  EXPECT_FALSE(q.recv(&v));
}

/**
 * @test Psclass.tQueue_OverflowDropsNewItems
 * @brief Verifies t Queue Overflow Drops New Items.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tQueue_OverflowDropsNewItems) {
  tQueue<int, 3> q; // capacity 2
  int a = 1, b = 2, c = 3;
  q.send(a);
  q.send(b);
  q.send(c); // dropped: queue is full

  int out = 0;
  EXPECT_TRUE(q.recv(&out));
  EXPECT_EQ(out, 1);
  EXPECT_TRUE(q.recv(&out));
  EXPECT_EQ(out, 2);
  EXPECT_FALSE(q.recv(&out)); // c was dropped, nothing left
}

/**
 * @test Psclass.tQueue_FlushEmptiesQueue
 * @brief Verifies t Queue Flush Empties Queue.
 *
 * @details
 * Exercises the Psclass code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass.cpp
 * @ingroup descent3_tests
 */
TEST(Psclass, tQueue_FlushEmptiesQueue) {
  tQueue<int, 4> q;
  int a = 1;
  q.send(a);
  q.flush();
  int out = 0;
  EXPECT_FALSE(q.recv(&out));
}
