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

// LegacyCockpit.cpp — Original 4:3 cockpit implementation
// Direct port of the original cockpit.cpp into the ICockpit class hierarchy.

#include <algorithm>
#include <cstring>

#include "LegacyCockpit.h"
#include "game.h"
#include "polymodel.h"
#include "hud.h"
#include "gauges.h"
#include "log.h"
#include "ship.h"
#include "player.h"
#include "pserror.h"
#include "room.h"
#include "hlsoundlib.h"
#include "sounds.h"

#define COCKPIT_ANIM_TIME 2.0f
#define COCKPIT_DORMANT_FRAME 0.0
#define COCKPIT_START_FRAME 3.0
#define COCKPIT_MIDWAY_FRAME 7.0
#define COCKPIT_COMPLETE_FRAME ((float)m_info.model->frame_max)
#define MAX_BUFFET_STRENGTH 0.75f
#define BUFFET_PERIOD 0.25f
#define COCKPIT_SHIFT_DELTA 0.02f

LegacyCockpit::LegacyCockpit() { memset(&m_info, 0, sizeof(tCockpitInfo)); }

LegacyCockpit::~LegacyCockpit() { Free(); }

// BUGFIX: Direct port of cockpit.cpp — no changes to original logic.
void LegacyCockpit::Init(int ship_index) {
  tCockpitCfgInfo cfginfo;
  int i;
  LOG_INFO << "Initializing cockpit.";
  LoadCockpitInfo(Ships[ship_index].cockpit_name, &cfginfo);
  for (i = 0; i < LC_NUM_SHIELD_GAUGE_FRAMES; i++) {
    if (cfginfo.shieldrings[i][0])
      HUD_resources.shield_bmp[i] = bm_AllocLoadFileBitmap(IGNORE_TABLE(cfginfo.shieldrings[i]), 0);
  }
  HUD_resources.ship_bmp = bm_AllocLoadFileBitmap(IGNORE_TABLE(cfginfo.shipimg), 0);
  HUD_resources.energy_bmp = bm_AllocLoadFileBitmap(IGNORE_TABLE(cfginfo.energyimg), 0);
  HUD_resources.afterburn_bmp = bm_AllocLoadFileBitmap(IGNORE_TABLE(cfginfo.burnimg), 0);
  HUD_resources.invpulse_bmp = bm_AllocLoadFileBitmap(IGNORE_TABLE(cfginfo.invpulseimg), 0);
  if (cfginfo.modelname[0] == 0) {
    m_info.model_num = -1;
    LOG_WARNING << "No cockpit found for ship.";
    return;
  }
  m_info.ship_index = ship_index;
  m_info.model_num = LoadPolyModel(IGNORE_TABLE(cfginfo.modelname), 0);
  m_info.model = GetPolymodelPointer(m_info.model_num);
  m_info.frame_time = 0.0f;
  m_info.state = COCKPIT_STATE_DORMANT;
  m_info.this_keyframe = COCKPIT_DORMANT_FRAME;
  m_info.next_keyframe = COCKPIT_DORMANT_FRAME;
  m_info.resized = false;
  m_info.buffet_amp = 0.0f;
  m_info.snd_open = SOUND_COCKPIT;
  vm_MakeZero(&m_info.buffet_vec);
  m_info.layered_mask = 0x00000000;
  m_info.nonlayered_mask = 0x00000000;
  for (i = 0; i < m_info.model->n_models; i++) {
    if ((m_info.model->submodel[i].flags & SOF_VIEWER) ||
        (m_info.model->submodel[i].flags & SOF_MONITOR_MASK))
      continue;
    if (m_info.model->submodel[i].flags & SOF_LAYER)
      m_info.layered_mask |= (1 << i);
    else
      m_info.nonlayered_mask |= (1 << i);
  }
  InitGauges(STAT_SHIELDS | STAT_SHIP | STAT_ENERGY | STAT_PRIMARYLOAD | STAT_SECONDARYLOAD | STAT_AFTERBURN);
  FreeReticle();
  InitReticle(-1, -1);
}

void LegacyCockpit::Free() {
  int i;
  CloseGauges();
  if (m_info.model_num > -1) {
    FreePolyModel(m_info.model_num);
    m_info.model = NULL;
    m_info.model_num = -1;
  }
  m_info.ship_index = -1;
  m_info.frame_time = 0.0f;
  m_info.state = COCKPIT_STATE_DORMANT;
  bm_FreeBitmap(HUD_resources.invpulse_bmp);
  bm_FreeBitmap(HUD_resources.afterburn_bmp);
  bm_FreeBitmap(HUD_resources.energy_bmp);
  for (i = 0; i < LC_NUM_SHIELD_GAUGE_FRAMES; i++)
    bm_FreeBitmap(HUD_resources.shield_bmp[i]);
  bm_FreeBitmap(HUD_resources.ship_bmp);
}

bool LegacyCockpit::IsValid() const { return (m_info.model_num > -1) ? true : false; }

bool LegacyCockpit::CockpitFileParse(const char *command, const char *operand, void *data) {
  tCockpitCfgInfo *cfginf = (tCockpitCfgInfo *)data;
  if (!strcmp(command, "ckptmodel")) {
    if (cfginf)
      strcpy(cfginf->modelname, operand);
  } else if (!strncmp(command, "shieldimg", strlen("shieldimg"))) {
    char buf[16];
    int i;
    for (i = 0; i < LC_NUM_SHIELD_GAUGE_FRAMES; i++) {
      snprintf(buf, sizeof(buf), "shieldimg%d", i);
      if (!stricmp(command, buf)) {
        if (cfginf)
          strcpy(cfginf->shieldrings[i], operand);
        break;
      }
    }
  } else if (!strcmp(command, "shipimg")) {
    if (cfginf)
      strcpy(cfginf->shipimg, operand);
  } else if (!strcmp(command, "afterburnimg")) {
    if (cfginf)
      strcpy(cfginf->burnimg, operand);
  } else if (!strcmp(command, "energyimg")) {
    if (cfginf)
      strcpy(cfginf->energyimg, operand);
  } else if (!strcmp(command, "invpulseimg")) {
    if (cfginf)
      strcpy(cfginf->invpulseimg, operand);
  } else if (!strcmp(command, "fullhudinf")) {
    if (cfginf)
      strcpy(HUD_resources.hud_inf_name, operand);
  } else {
    return false;
  }
  return true;
}

void LegacyCockpit::LoadCockpitInfo(const char *ckt_file, tCockpitCfgInfo *cfginfo) {
  if (cfginfo) {
    memset(cfginfo, 0, sizeof(tCockpitCfgInfo));
    ASSERT(LC_NUM_SHIELD_GAUGE_FRAMES == 5);
    strcpy(cfginfo->shieldrings[0], TBL_GAMEFILE("shieldring01.ogf"));
    strcpy(cfginfo->shieldrings[1], TBL_GAMEFILE("shieldring02.ogf"));
    strcpy(cfginfo->shieldrings[2], TBL_GAMEFILE("shieldring03.ogf"));
    strcpy(cfginfo->shieldrings[3], TBL_GAMEFILE("shieldring04.ogf"));
    strcpy(cfginfo->shieldrings[4], TBL_GAMEFILE("shieldring05.ogf"));
    strcpy(cfginfo->shipimg, TBL_GAMEFILE("hudship.ogf"));
    strcpy(cfginfo->burnimg, TBL_GAMEFILE("hudburn.ogf"));
    strcpy(cfginfo->energyimg, TBL_GAMEFILE("hudenergy.ogf"));
    strcpy(cfginfo->invpulseimg, TBL_GAMEFILE("shieldinv.ogf"));
  }
  if (ckt_file[0] == 0)
    return;
  LoadHUDConfig(ckt_file, CockpitFileParse, cfginfo);
}

void LegacyCockpit::Open() {
  if (m_info.this_keyframe <= COCKPIT_COMPLETE_FRAME) {
    m_info.state = COCKPIT_STATE_QUASI;
    m_info.this_keyframe = COCKPIT_DORMANT_FRAME;
    m_info.next_keyframe = COCKPIT_COMPLETE_FRAME;
    if (m_info.frame_time > 0.0f)
      m_info.frame_time = COCKPIT_ANIM_TIME - m_info.frame_time;
  }
  Sound_system.Play2dSound(m_info.snd_open);
  LoadCockpitInfo(Ships[m_info.ship_index].cockpit_name, NULL);
}

void LegacyCockpit::Close() {
  if (m_info.this_keyframe >= COCKPIT_DORMANT_FRAME) {
    m_info.next_keyframe = COCKPIT_DORMANT_FRAME;
    m_info.this_keyframe = COCKPIT_COMPLETE_FRAME;
    if (m_info.frame_time > 0.0f)
      m_info.frame_time = COCKPIT_ANIM_TIME - m_info.frame_time;
  }
  FlagGaugesNonfunctional(STAT_ALL);
  Sound_system.Play2dSound(m_info.snd_open);
}

void LegacyCockpit::QuickOpen() {
  m_info.frame_time = 0.0f;
  m_info.state = COCKPIT_STATE_FUNCTIONAL;
  m_info.this_keyframe = m_info.next_keyframe = COCKPIT_COMPLETE_FRAME;
  FlagGaugesFunctional(STAT_ALL);
  LoadCockpitInfo(Ships[m_info.ship_index].cockpit_name, NULL);
}

void LegacyCockpit::QuickClose() {
  m_info.frame_time = 0.0f;
  m_info.state = COCKPIT_STATE_DORMANT;
  m_info.this_keyframe = m_info.next_keyframe = COCKPIT_DORMANT_FRAME;
  FlagGaugesNonfunctional(STAT_ALL);
}

void LegacyCockpit::Resize() { m_info.resized = true; }

void LegacyCockpit::StartShake(float mag, vector *vec) {
  ASSERT(vec);
  if (mag > MAX_BUFFET_STRENGTH)
    mag = MAX_BUFFET_STRENGTH;
  m_info.buffet_amp = mag;
  m_info.buffet_vec = (*vec);
  m_info.buffet_wave = FixSin(0);
  m_info.buffet_time = 0.0f;
}

extern float GetTerrainDynamicScalar(vector *pos, int seg);
extern void GetRoomDynamicScalar(vector *pos, room *rp, float *r, float *g, float *b);

void LegacyCockpit::Render() {
  object *player_obj = &Objects[Players[Player_num].objnum];
  vector view_pos, light_vec;
  matrix view_tmat;
  float view_z, view_y, view_x, keyframe;
  float light_scalar_r, light_scalar_g, light_scalar_b;
  float normalized_time[MAX_SUBOBJECTS];
  bool gauge_reset = false;

  if (m_info.state == COCKPIT_STATE_DORMANT || m_info.model_num == -1)
    return;

  bsp_info *viewer_subobj = GetMonitorSubmodel(SOF_VIEWER);
  if (!viewer_subobj) {
    LOG_WARNING << "Cockpit missing viewer!";
    return;
  }
  view_z = viewer_subobj->offset.z() - m_info.buffet_vec.z() * m_info.buffet_amp * m_info.buffet_wave * 1.1f;
  view_y = viewer_subobj->offset.y() + m_info.buffet_vec.y() * m_info.buffet_amp * m_info.buffet_wave;
  view_x = viewer_subobj->offset.x() + m_info.buffet_vec.x() * m_info.buffet_amp * m_info.buffet_wave;

  view_tmat = Viewer_object->orient;
  view_pos = (view_tmat.fvec * view_z) + (view_tmat.uvec * view_y) + (view_tmat.rvec * view_x) + Viewer_object->pos;
  view_tmat.fvec = -view_tmat.fvec;
  view_tmat.rvec = -view_tmat.rvec;

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
  SetNormalizedTimeAnim(keyframe, normalized_time, m_info.model);

  if (m_info.buffet_amp > 0.04f) {
    angle buffet_angle;
    m_info.buffet_time += Frametime;
    if (m_info.buffet_time > BUFFET_PERIOD) {
      m_info.buffet_time = 0.0f;
      m_info.buffet_amp *= 0.5f;
    }
    buffet_angle = (angle)(65536.0 * m_info.buffet_time /
                           (BUFFET_PERIOD - ((BUFFET_PERIOD - m_info.buffet_time) * 0.5f)));
    m_info.buffet_wave = FixSin(buffet_angle);
    if (m_info.buffet_wave > 0.5f)
      m_info.buffet_wave = 1.0f;
    else if (m_info.buffet_wave < -0.5f)
      m_info.buffet_wave = -1.0f;
    else
      m_info.buffet_wave = 0.0f;
    m_info.animating = true;
  } else if (m_info.buffet_amp > 0.0f) {
    m_info.animating = true;
    m_info.buffet_amp = 0.0f;
  }

  rend_SetZBufferState(0);
  DrawPolygonModel(&view_pos, &view_tmat, m_info.model_num, normalized_time, 0, &light_vec, light_scalar_r,
                   light_scalar_g, light_scalar_b, m_info.nonlayered_mask, 0, 1);
  RenderGauges(&view_pos, &view_tmat, normalized_time, (m_info.animating || m_info.resized), gauge_reset);
  rend_SetZBufferState(0);
  DrawPolygonModel(&view_pos, &view_tmat, m_info.model_num, normalized_time, 0, &light_vec, light_scalar_r,
                   light_scalar_g, light_scalar_b, m_info.layered_mask, 0, 1);

  m_info.resized = false;
}

float LegacyCockpit::KeyframeAnimate() {
  float newkeyframe;

  newkeyframe = m_info.this_keyframe + (m_info.next_keyframe - m_info.this_keyframe) *
                                           (m_info.frame_time / COCKPIT_ANIM_TIME);
  if (m_info.this_keyframe < m_info.next_keyframe) {
    if (newkeyframe >= m_info.next_keyframe) {
      m_info.frame_time = 0.0f;
      m_info.this_keyframe = m_info.next_keyframe;
    }
  } else if (m_info.this_keyframe > m_info.next_keyframe) {
    if (newkeyframe <= m_info.next_keyframe) {
      m_info.frame_time = 0.0f;
      m_info.this_keyframe = m_info.next_keyframe;
    }
  } else {
    m_info.animating = false;
    return newkeyframe;
  }

  m_info.animating = true;
  if (m_info.this_keyframe != m_info.next_keyframe) {
    m_info.frame_time += Frametime;
  }
  if (m_info.this_keyframe == COCKPIT_COMPLETE_FRAME &&
      m_info.next_keyframe == m_info.this_keyframe) {
    FlagGaugesFunctional(STAT_ALL);
    m_info.state = COCKPIT_STATE_FUNCTIONAL;
  } else if (m_info.this_keyframe == COCKPIT_DORMANT_FRAME &&
             m_info.next_keyframe == COCKPIT_DORMANT_FRAME) {
    m_info.state = COCKPIT_STATE_DORMANT;
  } else {
    m_info.state = COCKPIT_STATE_QUASI;
  }
  return newkeyframe;
}

bsp_info *LegacyCockpit::GetMonitorSubmodel(uint16_t monitor_flag) {
  int i;
  ASSERT(m_info.model_num > -1);
  for (i = 0; i < Poly_models[m_info.model_num].n_models; i++) {
    if (Poly_models[m_info.model_num].submodel[i].flags & monitor_flag)
      return &Poly_models[m_info.model_num].submodel[i];
  }
  return NULL;
}

poly_model *LegacyCockpit::GetPolyModel() {
  ASSERT(m_info.model_num > -1);
  return &Poly_models[m_info.model_num];
}

int LegacyCockpit::State() const { return m_info.state; }
