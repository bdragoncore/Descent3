/*
 * Descent 3
 * Copyright (C) 2024 Parallax Software, 2026 InsanityBringer (PiccuEngine)
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

// WidescreenCockpit.cpp — Aspect-ratio-corrected cockpit for widescreen displays
// Overrides Render() to apply a horizontal vertex scale that counteracts the
// stretching caused by non-4:3 projection matrices.

#include "WidescreenCockpit.h"
#include "game.h"
#include "polymodel.h"
#include "player.h"
#include "room.h"
#include "3d.h"
#include "log.h"

extern float GetTerrainDynamicScalar(vector *pos, int seg);
extern void GetRoomDynamicScalar(vector *pos, room *rp, float *r, float *g, float *b);

#define COCKPIT_ANIM_TIME 2.0f
#define BUFFET_PERIOD 0.25f

WidescreenCockpit::WidescreenCockpit() : LegacyCockpit() {}

WidescreenCockpit::~WidescreenCockpit() {}

// Compute the horizontal correction scale factor.
// The cockpit polymodel is authored for 4:3. On wider screens, the projection
// matrix stretches it horizontally. We counteract by scaling rvec by:
//   (4/3) / (current_aspect)
// Example: 16:9 (1.778) => scale = 0.75 (narrower)
//          21:9 (2.333) => scale = 0.571
//          4:3  (1.333) => scale = 1.0  (no change)
float WidescreenCockpit::ComputeHorizontalScale() const {
  float current_aspect = (float)Game_window_w / (float)Game_window_h;
  if (current_aspect <= 0.0f)
    return 1.0f;
  constexpr float k4by3 = 4.0f / 3.0f;
  return k4by3 / current_aspect;
}

void WidescreenCockpit::Render() {
  tCockpitInfo *info = GetInfo();

  if (info->state == COCKPIT_STATE_DORMANT || info->model_num == -1)
    return;

  // Run parent logic up through all computation, but intercept the final
  // DrawPolygonModel calls to apply the horizontal scale to the orientation.
  //
  // We replicate the core render logic from LegacyCockpit::Render() here
  // with the orientation modification applied.

  object *player_obj = &Objects[Players[Player_num].objnum];
  vector view_pos, light_vec;
  matrix view_tmat;
  float view_z, view_y, view_x, keyframe;
  float light_scalar_r, light_scalar_g, light_scalar_b;
  float normalized_time[MAX_SUBOBJECTS];
  bool gauge_reset = false;

  bsp_info *viewer_subobj = GetMonitorSubmodel(SOF_VIEWER);
  if (!viewer_subobj) {
    LOG_WARNING << "Cockpit missing viewer!";
    return;
  }

  view_z = viewer_subobj->offset.z() - info->buffet_vec.z() * info->buffet_amp * info->buffet_wave * 1.1f;
  view_y = viewer_subobj->offset.y() + info->buffet_vec.y() * info->buffet_amp * info->buffet_wave;
  view_x = viewer_subobj->offset.x() + info->buffet_vec.x() * info->buffet_amp * info->buffet_wave;

  view_tmat = Viewer_object->orient;
  view_pos = (view_tmat.fvec * view_z) + (view_tmat.uvec * view_y) + (view_tmat.rvec * view_x) + Viewer_object->pos;
  view_tmat.fvec = -view_tmat.fvec;
  view_tmat.rvec = -view_tmat.rvec;

  // BUGFIX (PiccuEngine #3): Apply horizontal scale to counteract widescreen
  // stretching. The cockpit polymodel is authored for 4:3, so on wider screens
  // the projection matrix stretches it. Scaling rvec narrows the model to
  // maintain its intended proportions.
  float h_scale = ComputeHorizontalScale();
  view_tmat.rvec = view_tmat.rvec * h_scale;

  light_vec = -Viewer_object->orient.uvec;
  if (OBJECT_OUTSIDE(player_obj)) {
    float light_scalar = GetTerrainDynamicScalar(&player_obj->pos, CELLNUM(player_obj->roomnum));
    light_scalar_r = light_scalar;
    light_scalar_g = light_scalar;
    light_scalar_b = light_scalar;
  } else {
    GetRoomDynamicScalar(&player_obj->pos, &Rooms[player_obj->roomnum], &light_scalar_r, &light_scalar_g,
                         &light_scalar_b);
  }
  if (light_scalar_r < 0.1f)
    light_scalar_r = 0.1f;
  if (light_scalar_g < 0.1f)
    light_scalar_g = 0.1f;
  if (light_scalar_b < 0.1f)
    light_scalar_b = 0.1f;

  light_scalar_r *= .8f;
  light_scalar_g *= .8f;
  light_scalar_b *= .8f;

  if (player_obj->effect_info) {
    light_scalar_r = std::min(1.0f, light_scalar_r + (player_obj->effect_info->dynamic_red));
    light_scalar_g = std::min(1.0f, light_scalar_g + (player_obj->effect_info->dynamic_green));
    light_scalar_b = std::min(1.0f, light_scalar_b + (player_obj->effect_info->dynamic_blue));
  }
  if (Players[player_obj->id].flags & PLAYER_FLAGS_HEADLIGHT) {
    light_scalar_r = 1.0;
    light_scalar_g = 1.0;
    light_scalar_b = 1.0;
  }

  keyframe = KeyframeAnimate();
  SetNormalizedTimeAnim(keyframe, normalized_time, info->model);

  if (info->buffet_amp > 0.04f) {
    angle buffet_angle;
    info->buffet_time += Frametime;
    if (info->buffet_time > BUFFET_PERIOD) {
      info->buffet_time = 0.0f;
      info->buffet_amp *= 0.5f;
    }
    buffet_angle = (angle)(65536.0 * info->buffet_time /
                           (BUFFET_PERIOD - ((BUFFET_PERIOD - info->buffet_time) * 0.5f)));
    info->buffet_wave = FixSin(buffet_angle);
    if (info->buffet_wave > 0.5f)
      info->buffet_wave = 1.0f;
    else if (info->buffet_wave < -0.5f)
      info->buffet_wave = -1.0f;
    else
      info->buffet_wave = 0.0f;
    info->animating = true;
  } else if (info->buffet_amp > 0.0f) {
    info->animating = true;
    info->buffet_amp = 0.0f;
  }

  rend_SetZBufferState(0);
  DrawPolygonModel(&view_pos, &view_tmat, info->model_num, normalized_time, 0, &light_vec, light_scalar_r,
                   light_scalar_g, light_scalar_b, info->nonlayered_mask, 0, 1);
  RenderGauges(&view_pos, &view_tmat, normalized_time, (info->animating || info->resized), gauge_reset);
  rend_SetZBufferState(0);
  DrawPolygonModel(&view_pos, &view_tmat, info->model_num, normalized_time, 0, &light_vec, light_scalar_r,
                   light_scalar_g, light_scalar_b, info->layered_mask, 0, 1);

  info->resized = false;
}
