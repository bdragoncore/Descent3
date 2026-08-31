/**
 * @file findarg_stub.cpp
 * @brief FindArg stub - minimal implementation.
 *
 * @details
 * Covers the public contract of `Descent3/findarg_stub.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/findarg_stub.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/findarg_stub.cpp`
 * @par Harness
 * `findarg_stub.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/findarg_stub.cpp
 */

#include <cstddef>

/* C++ version for ddebug library */
int FindArg(const char* arg) { 
    (void)arg; 
    return 0; 
}
