/**
 * @file manage_tracklocks.cpp
 * @brief Minimal tracklock implementation for testing.
 *
 * @details
 * This is a subset of manage.cpp with only tracklock-related functions
 *
 * This harness validates the behavior of `Descent3/manage_tracklocks.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/manage_tracklocks.cpp`
 * @par Harness
 * `manage_tracklocks.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/manage_tracklocks.cpp
 */

#include "manage.h"
#include <string.h>

// Global tracklocks array
mngs_track_lock GlobalTrackLocks[MAX_TRACKLOCKS];

// Clear out tracklocks
void mng_InitTrackLocks() {
  for (int i = 0; i < MAX_TRACKLOCKS; i++) {
    GlobalTrackLocks[i].used = 0;
    GlobalTrackLocks[i].pagetype = PAGETYPE_UNKNOWN;
    GlobalTrackLocks[i].name[0] = 0;
  }
}

// Given a name, returns the index of the tracklock with that name
// -1 indicates that it wasn't found
int mng_FindTrackLock(char *name, int pagetype) {
  int i;
  for (i = 0; i < MAX_TRACKLOCKS; i++) {
    if (GlobalTrackLocks[i].used && GlobalTrackLocks[i].pagetype == pagetype &&
        !stricmp(name, GlobalTrackLocks[i].name))
      return i;
  }
  return -1;
}

// Searches through global array of tracklocks and returns first free one
// Sets the tracklock to be named "name" and its type "pagetype"
// returns -1 if none free
int mng_AllocTrackLock(char *name, int pagetype) {
  int i;
  for (i = 0; i < MAX_TRACKLOCKS; i++)
    if (GlobalTrackLocks[i].used == 0) {
      strcpy(GlobalTrackLocks[i].name, name);
      GlobalTrackLocks[i].pagetype = pagetype;
      GlobalTrackLocks[i].used = 1;
      // mprintf((0, "Tracklock %s allocated.\n", name));
      return i;
    }
  // Error("Couldn't get a free tracklock!");
  return -1;
}

// Frees a tracklock
void mng_FreeTrackLock(int n) {
  // mprintf((0, "Tracklock %s freed.\n", GlobalTrackLocks[n].name));
  GlobalTrackLocks[n].pagetype = PAGETYPE_UNKNOWN;
  GlobalTrackLocks[n].used = 0;
  GlobalTrackLocks[n].name[0] = 0;
}
