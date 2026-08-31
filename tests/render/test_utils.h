/*
 * Test helpers for D3 render tests: camera/viewport setup and shared constants.
 * Used by the game suite (d3_render_tests_game.cpp) for 3D frame setup.
 */
#ifndef TESTS_RENDER_TEST_UTILS_H
#define TESTS_RENDER_TEST_UTILS_H

#include "vecmat.h"
#include "3d.h"

namespace d3_render_test {

// Default camera position for tests: looking down +Z at origin.
inline void DefaultViewPosition(vector *view_pos) {
  view_pos->x() = 0.0f;
  view_pos->y() = 0.0f;
  view_pos->z() = -500.0f;
}

// Default view orientation: identity (camera looking along +Z).
inline void DefaultViewMatrix(matrix *view_matrix) {
  vm_MakeIdentity(view_matrix);
}

}  // namespace d3_render_test

#endif
