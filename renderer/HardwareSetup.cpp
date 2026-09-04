/*
* Descent 3
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstring>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "3d.h"
#include "HardwareInternal.h"
#include "renderer.h"

// User-specified aspect ratio, stored as w/h
static float sAspect = 0.0f;

// allows the user to specify an aspect ratio that overrides the renderer's
// The parameter is the w/h of the screen pixels
void g3_SetAspectRatio(float aspect) { sAspect = aspect; }
// returns the user-specified aspect ratio used to override the renderer's
float g3_GetAspectRatio() { return sAspect; }

void g3_GetViewPortMatrix(float *viewMat) {
  // extract the viewport data from the renderer
  int viewportWidth, viewportHeight;
  int viewportX, viewportY;
  rend_GetProjectionScreenParameters(viewportX, viewportY, viewportWidth, viewportHeight);

  float viewportWidthOverTwo = ((float)viewportWidth) * 0.5f;
  float viewportHeightOverTwo = ((float)viewportHeight) * 0.5f;

  // GLM refactor (Phase 1): build the viewport matrix with GLM. Column-major
  // layout: [0][0] scales x by w/2, [1][1] flips y by -h/2, [2][2]=1, and
  // [3][0]/[3][1] translate to the viewport center plus origin.
  glm::mat4 view(0.0f);
  view[0][0] = viewportWidthOverTwo;
  view[1][1] = -viewportHeightOverTwo;
  view[2][2] = 1.0f;
  view[3][0] = viewportWidthOverTwo + (float)viewportX;
  view[3][1] = viewportHeightOverTwo + (float)viewportY;
  view[3][3] = 1.0f;
  memcpy(viewMat, glm::value_ptr(view), 16 * sizeof(float));
}

void g3_GetProjectionMatrix(float zoom, float *projMat) {
  // get window size
  int viewportWidth, viewportHeight;
  rend_GetProjectionParameters(&viewportWidth, &viewportHeight);

  float s = ((float)viewportWidth) / ((float)viewportHeight);
  float vertical_fov = zoom * 3.0f / 4.0f;

  // BUGFIX (PiccuEngine #2): Use constant near/far planes instead of
  // dividing by zoom. The original code scaled znear and zfar by zoom,
  // which caused depth range issues when FOV was changed.
  float oOT = 1.0f / vertical_fov;

  // GLM refactor (Phase 1): build the projection matrix with GLM. The matrix
  // is column-major: [0][0] is the horizontal scale, [1][1] the vertical
  // scale, [2][2]=1 and [2][3]=1 encode the near=0/far=infinity convention,
  // and [3][2]=-1 is the perspective divide term.
  glm::mat4 proj(0.0f);
  if (s <= 1.0f) {
    proj[0][0] = oOT;
    proj[1][1] = oOT * s;
  } else {
    proj[0][0] = oOT / s;
    proj[1][1] = oOT;
  }
  proj[2][2] = 1.0f;
  proj[2][3] = 1.0f;
  proj[3][2] = -1.0f;
  memcpy(projMat, glm::value_ptr(proj), 16 * sizeof(float));
}

// start the frame
void g3_StartFrame(vector *view_pos, matrix *view_matrix, float zoom) {
  // initialize the viewport transform
  g3_GetViewPortMatrix((float *)gTransformViewPort);
  g3_GetProjectionMatrix(zoom, (float *)gTransformProjection);
  g3_GetModelViewMatrix(view_pos, view_matrix, (float *)gTransformModelView);
  g3_UpdateFullTransform();

  // get window size
  rend_GetProjectionParameters(&Window_width, &Window_height);

  // Set vars for projection
  Window_w2 = ((scalar)Window_width) * 0.5f;
  Window_h2 = ((scalar)Window_height) * 0.5f;

  // ISB trick: use the window aspect only, screen aspect ratio
  // is not important because we assume pixels are square
  scalar s = (scalar)Window_height / (scalar)Window_width;

  Matrix_scale = { s <= 1.0f ? s : 1.0f / s, 1.0f };


  //ISB: Convert zoom into vertical FOV for convenience
  zoom *= 3.f / 4.f;

  Matrix_scale.z() = 1.0f;

  // Set the view variables
  View_position = *view_pos;
  View_zoom = zoom;
  Unscaled_matrix = *view_matrix;

  // BUGFIX (PiccuEngine #2): Always scale x and y to apply FOV.
  // The original code scaled Z when zooming in (View_zoom <= 1.0),
  // which caused depth range problems. Scaling x and y uniformly
  // applies FOV correctly regardless of zoom direction.
  float oOZ = 1.0f / View_zoom;
  Matrix_scale.x() = Matrix_scale.x() * oOZ;
  Matrix_scale.y() = Matrix_scale.y() * oOZ;

  // Scale the matrix elements
  View_matrix.rvec = Unscaled_matrix.rvec * Matrix_scale.x();
  View_matrix.uvec = Unscaled_matrix.uvec * Matrix_scale.y();
  View_matrix.fvec = Unscaled_matrix.fvec * Matrix_scale.z();

  // Reset the list of free points
  InitFreePoints();

  // Reset the far clip plane
  g3_ResetFarClipZ();
}

// this doesn't do anything, but is here for completeness
void g3_EndFrame(void) {
  // make sure temp points are free
  CheckTempPoints();
}

// get the current view position
void g3_GetViewPosition(vector *vp) {
  // BUGFIX (g3 replacement, Phase 3): during instancing the old code returned
  // the re-based view position. The globals now always hold the true view
  // state, so express the view position in the object's local frame (identity
  // transform when no instance is active).
  matrix orient;
  vector pos;
  g3_GetInstanceTransform(&orient, &pos);
  *vp = (View_position - pos) * orient;
}

void g3_GetViewMatrix(matrix *mat) { *mat = View_matrix; }

void g3_GetUnscaledMatrix(matrix *mat) {
  // BUGFIX (g3 replacement, Phase 3): during instancing the old code returned
  // the re-based unscaled matrix. The globals now always hold the true view
  // state, so express the unscaled matrix in the object's local frame (identity
  // transform when no instance is active).
  matrix orient;
  vector pos;
  g3_GetInstanceTransform(&orient, &pos);
  *mat = ~orient * Unscaled_matrix;
}

// Gets the matrix scale vector
void g3_GetMatrixScale(vector *matrix_scale) { *matrix_scale = Matrix_scale; }
