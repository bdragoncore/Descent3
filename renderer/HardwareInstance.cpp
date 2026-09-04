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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "3d.h"
#include "HardwareInternal.h"
#include "pserror.h"

#define MAX_INSTANCE_DEPTH 30
static glm::mat4 sInstanceModelStack[MAX_INSTANCE_DEPTH];
static matrix sInstanceOrientCache[MAX_INSTANCE_DEPTH];
static vector sInstancePosCache[MAX_INSTANCE_DEPTH];
static int sInstanceDepth = 0;

// Returns the composed instance model matrix (identity when no instance is active).
static const glm::mat4 &GetInstanceModelMatrix() {
  static const glm::mat4 identity(1.0f);
  if (sInstanceDepth == 0) {
    return identity;
  }
  return sInstanceModelStack[sInstanceDepth - 1];
}

// Returns the composed instance transform in the g3 row-vector convention:
// world = src * ~orient + pos. When no instance is active, orient is identity
// and pos is zero. The decomposed form is cached at push time to avoid
// decomposing the 4x4 model matrix per-vertex in g3_RotatePoint and
// g3_CheckNormalFacing.
void g3_GetInstanceTransform(matrix *orient, vector *pos) {
  if (sInstanceDepth == 0) {
    static const matrix identity = [] {
      matrix m;
      vm_MakeIdentity(&m);
      return m;
    }();
    *orient = identity;
    *pos = vector{0, 0, 0};
    return;
  }
  *orient = sInstanceOrientCache[sInstanceDepth - 1];
  *pos = sInstancePosCache[sInstanceDepth - 1];
}

// Recomputes gTransformModelView as view * model (the composed instance model
// matrix, identity when no instance is active). Shared by g3_StartInstanceMatrix,
// g3_DoneInstance and rend_SetZBias so the model transform is never lost.
void g3_UpdateModelViewMatrix() {
  float view[4][4];
  g3_GetModelViewMatrix(&View_position, &Unscaled_matrix, (float *)view);
  glm::mat4 mv = glm::make_mat4x4(&view[0][0]) * GetInstanceModelMatrix();
  memcpy(gTransformModelView, glm::value_ptr(mv), sizeof(gTransformModelView));
  g3_UpdateFullTransform();
}

// instance at specified point with specified orientation
void g3_StartInstanceMatrix(vector *pos, matrix *orient) {
  ASSERT(orient != NULL);
  ASSERT(sInstanceDepth < MAX_INSTANCE_DEPTH);

  // BUGFIX (g3 replacement, Phase 3): the old code re-based the global
  // View_position/View_matrix/Unscaled_matrix so the CPU g3 functions would
  // work on object-space vertices. That trick is removed: the globals now
  // always hold the true view state, and the composed model matrix is pushed
  // on a GLM stack. Consumers (g3_RotatePoint, g3_CheckNormalFacing,
  // g3_GetViewPosition, g3_GetUnscaledMatrix) apply the model transform via
  // g3_GetInstanceTransform, and g3_UpdateModelViewMatrix keeps the GPU
  // model-view uniform equal to view * model.
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos->x(), pos->y(), pos->z())) *
                    glm::mat4(glm::vec4(orient->rvec.x(), orient->rvec.y(), orient->rvec.z(), 0.0f),
                              glm::vec4(orient->uvec.x(), orient->uvec.y(), orient->uvec.z(), 0.0f),
                              glm::vec4(orient->fvec.x(), orient->fvec.y(), orient->fvec.z(), 0.0f),
                              glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
  if (sInstanceDepth > 0) {
    model = sInstanceModelStack[sInstanceDepth - 1] * model;
  }
  sInstanceModelStack[sInstanceDepth] = model;

  // Pre-compute and cache the decomposed orient/pos so g3_GetInstanceTransform
  // can return them without decomposing the 4x4 matrix per-vertex.
  sInstanceOrientCache[sInstanceDepth].rvec = vector{model[0][0], model[1][0], model[2][0]};
  sInstanceOrientCache[sInstanceDepth].uvec = vector{model[0][1], model[1][1], model[2][1]};
  sInstanceOrientCache[sInstanceDepth].fvec = vector{model[0][2], model[1][2], model[2][2]};
  sInstancePosCache[sInstanceDepth].x() = model[3][0];
  sInstancePosCache[sInstanceDepth].y() = model[3][1];
  sInstancePosCache[sInstanceDepth].z() = model[3][2];

  ++sInstanceDepth;

  g3_UpdateModelViewMatrix();
}

// instance at specified point with specified orientation
void g3_StartInstanceAngles(vector *pos, angvec *angles) {
  if (angles == NULL) {
    matrix ident;
    vm_MakeIdentity(&ident);
    g3_StartInstanceMatrix(pos, &ident);
    return;
  }

  matrix tm;
  vm_AnglesToMatrix(&tm, angles->p(), angles->h(), angles->b());

  g3_StartInstanceMatrix(pos, &tm);
}

// pops the old context
void g3_DoneInstance() {
  --sInstanceDepth;
  ASSERT(sInstanceDepth >= 0);

  g3_UpdateModelViewMatrix();
}
