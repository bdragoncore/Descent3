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

// Whether or not to use T&L transforms or the pass-thru ones
static int sUseTransformPassthru = -1;

extern float Z_bias;
void g3_GetModelViewMatrix(const vector *viewPos, const matrix *viewMatrix, float *mvMat) {
  matrix localOrient = (*viewMatrix);
  vector localPos = -((vector)*viewPos);

  // GLM refactor (Phase 1): build the model-view matrix with GLM. The matrix
  // is stored column-major (OpenGL convention): column 0 holds the x
  // components of rvec/uvec/fvec, column 1 the y components, column 2 the z
  // components, and column 3 the translation. glm::value_ptr yields the same
  // layout as the previous hand-rolled code.
  glm::mat4 mv = glm::mat4(
      glm::vec4(localOrient.rvec.x(), localOrient.uvec.x(), localOrient.fvec.x(), 0.0f),
      glm::vec4(localOrient.rvec.y(), localOrient.uvec.y(), localOrient.fvec.y(), 0.0f),
      glm::vec4(localOrient.rvec.z(), localOrient.uvec.z(), localOrient.fvec.z(), 0.0f),
      glm::vec4(vm_Dot3Product(localPos, localOrient.rvec), vm_Dot3Product(localPos, localOrient.uvec),
                vm_Dot3Product(localPos, localOrient.fvec) + Z_bias, 1.0f));
  memcpy(mvMat, glm::value_ptr(mv), 16 * sizeof(float));
}

void g3_TransformMult(float res[4][4], float a[4][4], float b[4][4]) {
  // GLM refactor (Phase 1): the matrices are stored column-major (OpenGL
  // convention). The original hand-rolled loop indexed the arrays as
  // row-major, which computes b*a in column-major terms, so the GLM product
  // must be mb * ma to produce byte-identical output.
  glm::mat4 ma = glm::make_mat4x4(&a[0][0]);
  glm::mat4 mb = glm::make_mat4x4(&b[0][0]);
  glm::mat4 mr = mb * ma;
  memcpy(res, glm::value_ptr(mr), 16 * sizeof(float));
}

void g3_TransformTrans(float res[4][4], float t[4][4]) {
  // GLM refactor (Phase 1): glm::transpose on the column-major matrix yields
  // the same layout as the previous hand-rolled transpose.
  glm::mat4 mt = glm::make_mat4x4(&t[0][0]);
  glm::mat4 mr = glm::transpose(mt);
  memcpy(res, glm::value_ptr(mr), 16 * sizeof(float));
}

void g3_UpdateFullTransform() {
  // ModelView -> projection
  g3_TransformMult(gTransformFull, gTransformModelView, gTransformProjection);

  // projection  -> ViewPort
  g3_TransformMult(gTransformFull, gTransformFull, gTransformViewPort);
}

void g3_ForceTransformRefresh(void) { sUseTransformPassthru = -1; }

void g3_RefreshTransforms(bool usePassthru) {
  if (sUseTransformPassthru == 1 && usePassthru) {
    // we don't have to do anything because we are already setup for pass-thru
    return;
  }

  if (usePassthru) {
    // setup OpenGL to use pass-thru
    rend_TransformSetToPassthru();
  } else {
    // extract the viewport data from the renderer
    int viewportWidth, viewportHeight, viewportX, viewportY;
    rend_GetProjectionScreenParameters(viewportX, viewportY, viewportWidth, viewportHeight);

    // setup OpenGL to use full transform stack
    // TODO: in the future we only need to set those that have changed
    rend_TransformSetViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    rend_TransformSetProjection(gTransformProjection);
    rend_TransformSetModelView(gTransformModelView);
  }

  // store the pass-thru
  sUseTransformPassthru = (usePassthru) ? 1 : 0;
}
