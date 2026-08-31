/**
 * @file list_real_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for the smallest game component: Descent3/list.cpp
 * Generic doubly-linked list of void* (used by attach, editor, etc.)
 * Covers: NewListNode, AddListItem, RemoveListItem, DestroyList,
 * CountListItems, GetListItem, GetListItemIndex
 * Documents / fixes latent bugs:
 *  - AddListItem missed duplicate check on tail and leaked newnode on duplicate
 *  - DestroyList left *listp dangling instead of NULL
 *
 * This harness validates the behavior of `Descent3/list.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/list.cpp`
 * @par Harness
 * `list_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/list.cpp
 */

#include <gtest/gtest.h>

#include "list.h"
#include "mem.h"

// list.cpp allocates via mem_rmalloc; ensure mem is usable.
// mem_Init is lightweight; we rely on existing mem tests for thorough coverage.

/**
 * @test List.NewListNode_Zeroed
 * @brief Verifies new List Node Zeroed.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, NewListNode_Zeroed) {
  listnode *n = NewListNode();
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(n->data, nullptr);
  EXPECT_EQ(n->next, nullptr);
  EXPECT_EQ(n->prev, nullptr);
  mem_free(n);
}

/**
 * @test List.AddAndCount_Single
 * @brief Verifies add And Count Single.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, AddAndCount_Single) {
  listnode *head = nullptr;
  int a = 1;
  EXPECT_EQ(AddListItem(&head, &a), 1);
  EXPECT_EQ(CountListItems(&head), 1);
  EXPECT_EQ(GetListItem(&head, 0), &a);
  EXPECT_EQ(GetListItemIndex(&head, &a), 0);
  DestroyList(&head);
  head = nullptr; // DestroyList now nulls; explicit for safety in case of old code
}

/**
 * @test List.AddMultiple_PreservesOrder
 * @brief Verifies add Multiple Preserves Order.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, AddMultiple_PreservesOrder) {
  listnode *head = nullptr;
  int a = 1, b = 2, c = 3;
  ASSERT_EQ(AddListItem(&head, &a), 1);
  ASSERT_EQ(AddListItem(&head, &b), 1);
  ASSERT_EQ(AddListItem(&head, &c), 1);
  EXPECT_EQ(CountListItems(&head), 3);
  EXPECT_EQ(GetListItem(&head, 0), &a);
  EXPECT_EQ(GetListItem(&head, 1), &b);
  EXPECT_EQ(GetListItem(&head, 2), &c);
  EXPECT_EQ(GetListItemIndex(&head, &a), 0);
  EXPECT_EQ(GetListItemIndex(&head, &b), 1);
  EXPECT_EQ(GetListItemIndex(&head, &c), 2);
  EXPECT_EQ(GetListItem(&head, 99), nullptr);
  EXPECT_EQ(GetListItemIndex(&head, nullptr), -1);
  DestroyList(&head);
  head = nullptr;
}

/**
 * @test List.AddDuplicate_InteriorRejected
 * @brief Verifies add Duplicate Interior Rejected.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, AddDuplicate_InteriorRejected) {
  listnode *head = nullptr;
  int a = 1, b = 2;
  AddListItem(&head, &a);
  AddListItem(&head, &b);
  // Adding head item again (duplicate interior) must fail (return 0)
  // Fixed code frees the leaked newnode on duplicate.
  EXPECT_EQ(AddListItem(&head, &a), 0);
  EXPECT_EQ(CountListItems(&head), 2);
  DestroyList(&head);
  head = nullptr;
}

/**
 * @test List.AddDuplicate_TailRejected
 * @brief Verifies add Duplicate Tail Rejected.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, AddDuplicate_TailRejected) {
  listnode *head = nullptr;
  int a = 1, b = 2;
  AddListItem(&head, &a);
  AddListItem(&head, &b);
  // Tail duplicate was previously missed (appended duplicate). Fixed code must reject.
  EXPECT_EQ(AddListItem(&head, &b), 0);
  EXPECT_EQ(CountListItems(&head), 2);
  DestroyList(&head);
  head = nullptr;
}

/**
 * @test List.RemoveHead
 * @brief Verifies remove Head.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, RemoveHead) {
  listnode *head = nullptr;
  int a = 1, b = 2, c = 3;
  AddListItem(&head, &a);
  AddListItem(&head, &b);
  AddListItem(&head, &c);
  EXPECT_EQ(RemoveListItem(&head, &a), 1);
  EXPECT_EQ(CountListItems(&head), 2);
  EXPECT_EQ(GetListItem(&head, 0), &b);
  EXPECT_EQ(head->prev, nullptr);
  DestroyList(&head);
  head = nullptr;
}

/**
 * @test List.RemoveMiddle
 * @brief Verifies remove Middle.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, RemoveMiddle) {
  listnode *head = nullptr;
  int a = 1, b = 2, c = 3;
  AddListItem(&head, &a);
  AddListItem(&head, &b);
  AddListItem(&head, &c);
  EXPECT_EQ(RemoveListItem(&head, &b), 1);
  EXPECT_EQ(CountListItems(&head), 2);
  EXPECT_EQ(GetListItem(&head, 0), &a);
  EXPECT_EQ(GetListItem(&head, 1), &c);
  // Check doubly-linked consistency
  EXPECT_EQ(head->next->prev, head);
  DestroyList(&head);
  head = nullptr;
}

/**
 * @test List.RemoveTail
 * @brief Verifies remove Tail.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, RemoveTail) {
  listnode *head = nullptr;
  int a = 1, b = 2, c = 3;
  AddListItem(&head, &a);
  AddListItem(&head, &b);
  AddListItem(&head, &c);
  EXPECT_EQ(RemoveListItem(&head, &c), 1);
  EXPECT_EQ(CountListItems(&head), 2);
  EXPECT_EQ(head->next->next, nullptr);
  DestroyList(&head);
  head = nullptr;
}

/**
 * @test List.RemoveNotFound
 * @brief Verifies remove Not Found.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, RemoveNotFound) {
  listnode *head = nullptr;
  int a = 1, b = 99;
  AddListItem(&head, &a);
  EXPECT_EQ(RemoveListItem(&head, &b), 0);
  EXPECT_EQ(CountListItems(&head), 1);
  DestroyList(&head);
  head = nullptr;
}

/**
 * @test List.RemoveOnlyItem
 * @brief Verifies remove Only Item.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, RemoveOnlyItem) {
  listnode *head = nullptr;
  int a = 1;
  AddListItem(&head, &a);
  EXPECT_EQ(RemoveListItem(&head, &a), 1);
  EXPECT_EQ(head, nullptr);
  EXPECT_EQ(CountListItems(&head), 0);
}

/**
 * @test List.Destroy_NullsHead
 * @brief Verifies destroy Nulls Head.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, Destroy_NullsHead) {
  listnode *head = nullptr;
  int a = 1, b = 2;
  AddListItem(&head, &a);
  AddListItem(&head, &b);
  DestroyList(&head);
  // Fixed DestroyList must set *listp to NULL; previously left dangling.
  EXPECT_EQ(head, nullptr);
  EXPECT_EQ(CountListItems(&head), 0);
}

/**
 * @test List.DestroyEmpty_IsSafe
 * @brief Verifies destroy Empty Is Safe.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, DestroyEmpty_IsSafe) {
  listnode *head = nullptr;
  DestroyList(&head);
  EXPECT_EQ(head, nullptr);
}

/**
 * @test List.CountEmpty
 * @brief Verifies count Empty.
 *
 * @details
 * Exercises the List code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/list.cpp
 * @ingroup descent3_tests
 */
TEST(List, CountEmpty) {
  listnode *head = nullptr;
  EXPECT_EQ(CountListItems(&head), 0);
  EXPECT_EQ(GetListItem(&head, 0), nullptr);
}
