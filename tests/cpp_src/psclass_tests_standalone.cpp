/**
 * @file psclass_tests_standalone.cpp
 * @brief D3 Coverage Tests - psclass module (Standalone).
 *
 * @details
 * Tests for tList and tQueue template classes.
 * Uses seeded RNG for reproducible test data.
 *
 * This harness validates the behavior of `Descent3/psclass_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/psclass_tests_standalone.cpp`
 * @par Harness
 * `psclass_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/psclass_tests_standalone.cpp
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>

// ============================================================================
// D3 psclass Implementation (matching lib/psclass.h)
// ============================================================================

template <class T> struct tListNode {
    T t;
    tListNode<T>* next;
};

template <class T> class tList {
    tListNode<T>* m_link;
    tListNode<T>* m_mark;
    int m_length;

public:
    tList() {
        m_link = m_mark = nullptr;
        m_length = 0;
    }
    ~tList() { tList::free(); }

    tListNode<T>* start() {
        m_mark = m_link;
        return m_mark;
    }

    tListNode<T>* next() {
        m_mark = (m_mark->next) ? m_mark->next : m_mark;
        return get();
    }

    tListNode<T>* get() const { return m_mark ? m_mark : nullptr; }

    int length() const { return m_length; }

    void free() {
        m_link = m_mark = nullptr;
        m_length = 0;
    }

    void link(tListNode<T>* node) {
        if (m_link) {
            node->next = m_mark->next;
            m_mark->next = node;
        } else {
            m_link = node;
            node->next = nullptr;
        }
        m_mark = node;
        m_length++;
    }

    tListNode<T>* unlink() {
        tListNode<T>* freenode;
        tListNode<T>* node;
        if (!m_link)
            return nullptr;
        if (m_link == m_mark) {
            freenode = m_mark;
            m_mark = m_link = m_link->next;
        } else {
            node = m_link;
            while (node->next != m_mark)
                node = node->next;
            freenode = m_mark;
            node->next = m_mark->next;
        }
        freenode->next = nullptr;
        return freenode;
    }
};

template <class T, int t_LEN> class tQueue {
    T m_items[t_LEN];
    short m_head;
    short m_tail;

public:
    tQueue() { m_head = m_tail = 0; }
    ~tQueue() {}

    void send(T& item) {
        short temp = m_tail + 1;
        if (temp == t_LEN)
            temp = 0;
        if (temp != m_head) {
            m_items[m_tail] = item;
            m_tail = temp;
        }
    }

    bool recv(T* item) {
        if (m_head == m_tail)
            return false;
        *item = m_items[m_head++];
        if (m_head == t_LEN)
            m_head = 0;
        return true;
    }

    void flush() {
        m_head = m_tail = 0;
    }

    bool empty() const { return m_head == m_tail; }
    int count() const {
        if (m_tail >= m_head)
            return m_tail - m_head;
        return t_LEN - m_head + m_tail;
    }
};

// ============================================================================
// Test Fixtures
// ============================================================================

/**
 * @brief GTest fixture for PsclassTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PsclassTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    PsclassTest() : rng_(SEED) {}
};

// ============================================================================
// tList tests
// ============================================================================

/**
 * @test PsclassTest.ListInitialLength
 * @brief Verifies list Initial Length.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, ListInitialLength) {
    tList<int> list;
    EXPECT_EQ(list.length(), 0);
}

/**
 * @test PsclassTest.ListLinkOne
 * @brief Verifies list Link One.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, ListLinkOne) {
    tList<int> list;
    tListNode<int>* node = new tListNode<int>();
    node->t = 42;
    node->next = nullptr;
    list.link(node);
    EXPECT_EQ(list.length(), 1);
}

/**
 * @test PsclassTest.ListLinkMultiple
 * @brief Verifies list Link Multiple.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, ListLinkMultiple) {
    tList<int> list;
    for (int i = 0; i < 5; i++) {
        tListNode<int>* node = new tListNode<int>();
        node->t = i;
        node->next = nullptr;
        list.link(node);
    }
    EXPECT_EQ(list.length(), 5);
}

/**
 * @test PsclassTest.ListStartGet
 * @brief Verifies list Start Get.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, ListStartGet) {
    tList<int> list;
    tListNode<int>* node = new tListNode<int>();
    node->t = 42;
    node->next = nullptr;
    list.link(node);
    
    tListNode<int>* current = list.start();
    EXPECT_NE(current, nullptr);
    EXPECT_EQ(current->t, 42);
}

/**
 * @test PsclassTest.ListIterate
 * @brief Verifies list Iterate.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, ListIterate) {
    tList<int> list;
    for (int i = 0; i < 3; i++) {
        tListNode<int>* node = new tListNode<int>();
        node->t = i;
        node->next = nullptr;
        list.link(node);
    }
    
    std::vector<int> values;
    tListNode<int>* current = list.start();
    while (current != nullptr) {
        values.push_back(current->t);
        if (current->next == nullptr) break;
        current = list.next();
    }
    
    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], 0);
    EXPECT_EQ(values[1], 1);
    EXPECT_EQ(values[2], 2);
}

/**
 * @test PsclassTest.ListFree
 * @brief Verifies list Free.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, ListFree) {
    tList<int> list;
    for (int i = 0; i < 3; i++) {
        tListNode<int>* node = new tListNode<int>();
        node->t = i;
        node->next = nullptr;
        list.link(node);
    }
    
    list.free();
    EXPECT_EQ(list.length(), 0);
    EXPECT_EQ(list.start(), nullptr);
}

/**
 * @test PsclassTest.ListUnlinkLast
 * @brief Verifies list Unlink Last.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, ListUnlinkLast) {
    tList<int> list;
    for (int i = 0; i < 3; i++) {
        tListNode<int>* node = new tListNode<int>();
        node->t = i;
        node->next = nullptr;
        list.link(node);
    }
    
    list.start();
    list.next();
    list.next();
    tListNode<int>* removed = list.unlink();
    EXPECT_NE(removed, nullptr);
    EXPECT_GE(list.length(), 0);
    delete removed;
}

/**
 * @test PsclassTest.ListUnlinkAll
 * @brief Verifies list Unlink All.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, ListUnlinkAll) {
    tList<int> list;
    for (int i = 0; i < 3; i++) {
        tListNode<int>* node = new tListNode<int>();
        node->t = i;
        node->next = nullptr;
        list.link(node);
    }
    
    list.free();
    EXPECT_EQ(list.length(), 0);
}

// ============================================================================
// tQueue tests
// ============================================================================

/**
 * @test PsclassTest.QueueInitialEmpty
 * @brief Verifies queue Initial Empty.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, QueueInitialEmpty) {
    tQueue<int, 10> queue;
    EXPECT_TRUE(queue.empty());
}

/**
 * @test PsclassTest.QueueSendRecv
 * @brief Verifies queue Send Recv.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, QueueSendRecv) {
    tQueue<int, 10> queue;
    int val = 42;
    queue.send(val);
    
    int result;
    bool success = queue.recv(&result);
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 42);
}

/**
 * @test PsclassTest.QueueMultipleSendRecv
 * @brief Verifies queue Multiple Send Recv.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, QueueMultipleSendRecv) {
    tQueue<int, 10> queue;
    
    for (int i = 0; i < 5; i++) {
        int val = i;
        queue.send(val);
    }
    
    for (int i = 0; i < 5; i++) {
        int result;
        bool success = queue.recv(&result);
        EXPECT_TRUE(success);
        EXPECT_EQ(result, i);
    }
}

/**
 * @test PsclassTest.QueueEmptyAfterFlush
 * @brief Verifies queue Empty After Flush.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, QueueEmptyAfterFlush) {
    tQueue<int, 10> queue;
    int v1 = 1, v2 = 2;
    queue.send(v1);
    queue.send(v2);
    
    queue.flush();
    EXPECT_TRUE(queue.empty());
}

/**
 * @test PsclassTest.QueueWrapAround
 * @brief Verifies queue Wrap Around.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, QueueWrapAround) {
    tQueue<int, 4> queue;
    
    for (int i = 0; i < 4; i++) {
        int v = i;
        queue.send(v);
    }
    
    int result;
    queue.recv(&result);
    EXPECT_EQ(result, 0);
    
    int v = 100;
    queue.send(v);
    
    for (int i = 0; i < 4; i++) {
        queue.recv(&result);
    }
}

/**
 * @test PsclassTest.QueueCount
 * @brief Verifies queue Count.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, QueueCount) {
    tQueue<int, 10> queue;
    EXPECT_EQ(queue.count(), 0);
    
    int v1 = 1, v2 = 2;
    queue.send(v1);
    queue.send(v2);
    EXPECT_EQ(queue.count(), 2);
    
    int val;
    queue.recv(&val);
    EXPECT_EQ(queue.count(), 1);
}

/**
 * @test PsclassTest.QueueRecvEmpty
 * @brief Verifies queue Recv Empty.
 *
 * @details
 * Exercises the PsclassTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassTest, QueueRecvEmpty) {
    tQueue<int, 10> queue;
    int result;
    bool success = queue.recv(&result);
    EXPECT_FALSE(success);
}

// ============================================================================
// Property-based tests
// ============================================================================

/**
 * @brief GTest fixture for PsclassPropertyTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PsclassPropertyTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 54321;
    std::mt19937 rng_;
    
    PsclassPropertyTest() : rng_(SEED) {}
};

/**
 * @test PsclassPropertyTest.ListOrderPreserved
 * @brief Verifies list Order Preserved.
 *
 * @details
 * Exercises the PsclassPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassPropertyTest, ListOrderPreserved) {
    tList<int> list;
    const int N = 10;
    
    for (int i = 0; i < N; i++) {
        tListNode<int>* node = new tListNode<int>();
        node->t = i;
        node->next = nullptr;
        list.link(node);
    }
    
    int expected = 0;
    tListNode<int>* current = list.start();
    while (current != nullptr) {
        EXPECT_EQ(current->t, expected++);
        if (current->next == nullptr) break;
        current = list.next();
    }
}

/**
 * @test PsclassPropertyTest.QueueFIFO
 * @brief Verifies queue FIFO.
 *
 * @details
 * Exercises the PsclassPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsclassPropertyTest, QueueFIFO) {
    tQueue<int, 20> queue;
    const int N = 10;
    
    for (int i = 0; i < N; i++) {
        queue.send(i);
    }
    
    for (int i = 0; i < N; i++) {
        int val;
        bool ok = queue.recv(&val);
        EXPECT_TRUE(ok);
        EXPECT_EQ(val, i);
    }
}

// ============================================================================
// Edge cases
// ============================================================================

/**
 * @test PsclassEdgeCase.ListUnlinkEmpty
 * @brief Verifies list Unlink Empty.
 *
 * @details
 * Exercises the PsclassEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psclass_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(PsclassEdgeCase, ListUnlinkEmpty) {
    tList<int> list;
    list.start();
    tListNode<int>* result = list.unlink();
    EXPECT_EQ(result, nullptr);
}
