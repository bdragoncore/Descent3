/*
 * Cube mesh data for D3 game render tests.
 * Centered at origin, side length 2 (from -1 to +1).
 * Vertices and face indices for CCW winding when viewed from outside.
 */
#ifndef TESTS_RENDER_FIXTURES_CUBE_H
#define TESTS_RENDER_FIXTURES_CUBE_H

#include "vecmat.h"

namespace d3_render_test {

// 8 vertices: 0=(-1,-1,-1), 1=(+1,-1,-1), 2=(+1,+1,-1), 3=(-1,+1,-1),
//             4=(-1,-1,+1), 5=(+1,-1,+1), 6=(+1,+1,+1), 7=(-1,+1,+1)
inline void CubeVertex(int i, vector *v) {
  static const float CUBE_VERTS[8][3] = {
      {-1, -1, -1}, {+1, -1, -1}, {+1, +1, -1}, {-1, +1, -1},
      {-1, -1, +1}, {+1, -1, +1}, {+1, +1, +1}, {-1, +1, +1},
  };
  v->x() = CUBE_VERTS[i][0];
  v->y() = CUBE_VERTS[i][1];
  v->z() = CUBE_VERTS[i][2];
}

// 6 faces, 4 vertices each (quad). CCW from outside.
// Face order: front(z+), back(z-), right(x+), left(x-), top(y+), bottom(y-)
constexpr int CUBE_FACE[6][4] = {
    {4, 5, 6, 7},  /* front  z=+1 */
    {1, 0, 3, 2},  /* back   z=-1 */
    {5, 1, 2, 6},  /* right  x=+1 */
    {0, 4, 7, 3},  /* left   x=-1 */
    {3, 7, 6, 2},  /* top    y=+1 */
    {0, 1, 5, 4},  /* bottom y=-1 */
};

constexpr int CUBE_NUM_VERTS = 8;
constexpr int CUBE_NUM_FACES = 6;
constexpr int CUBE_VERTS_PER_FACE = 4;

}  // namespace d3_render_test

#endif
