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

// cockpit.cpp — Cockpit system delegation layer
// All functions delegate to the active ICockpit implementation created
// by the factory. The actual cockpit logic lives in LegacyCockpit.cpp
// and WidescreenCockpit.cpp.

#include "cockpit.h"
#include "cockpit_factory.h"
#include "player.h"

// BUGFIX #685: After CreateCockpit() swaps the active cockpit implementation,
// the new instance is uninitialized (model_num == -1) and would not render
// until the next level load. Initialize and open it for the current player so
// the mode change takes effect immediately, even if a level is resumed without
// reloading.
void RecreateCockpitForCurrentPlayer() {
  int ship_index = (Player_num >= 0) ? Players[Player_num].ship_index : 0;
  if (ship_index < 0)
    ship_index = 0;
  InitCockpit(ship_index);
  QuickOpenCockpit();
}

void RenderCockpit() {
  if (ICockpit *c = GetCockpit())
    c->Render();
}

void InitCockpit(int ship_index) {
  if (ICockpit *c = GetCockpit())
    c->Init(ship_index);
}

void FreeCockpit() {
  if (ICockpit *c = GetCockpit())
    c->Free();
}

bool IsValidCockpit() {
  if (ICockpit *c = GetCockpit())
    return c->IsValid();
  return false;
}

void OpenCockpit() {
  if (ICockpit *c = GetCockpit())
    c->Open();
}

void CloseCockpit() {
  if (ICockpit *c = GetCockpit())
    c->Close();
}

void ResizeCockpit() {
  if (ICockpit *c = GetCockpit())
    c->Resize();
}

void QuickOpenCockpit() {
  if (ICockpit *c = GetCockpit())
    c->QuickOpen();
}

void QuickCloseCockpit() {
  if (ICockpit *c = GetCockpit())
    c->QuickClose();
}

bsp_info *CockpitGetMonitorSubmodel(uint16_t monitor_flag) {
  if (ICockpit *c = GetCockpit())
    return c->GetMonitorSubmodel(monitor_flag);
  return NULL;
}

poly_model *CockpitGetPolyModel() {
  if (ICockpit *c = GetCockpit())
    return c->GetPolyModel();
  return NULL;
}

void StartCockpitShake(float mag, vector *vec) {
  if (ICockpit *c = GetCockpit())
    c->StartShake(mag, vec);
}

int CockpitState() {
  if (ICockpit *c = GetCockpit())
    return c->State();
  return COCKPIT_STATE_DORMANT;
}
