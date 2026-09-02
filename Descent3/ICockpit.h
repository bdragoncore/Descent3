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

// ICockpit.h — Abstract cockpit interface
// Provides a polymorphic cockpit system switchable between Legacy (4:3) and
// Widescreen (aspect-corrected) implementations at runtime.

#ifndef ICOCKPIT_H
#define ICOCKPIT_H

#include "pstypes.h"
#include "vecmat.h"

struct poly_model;
struct bsp_info;

#define COCKPIT_STATE_DORMANT 0
#define COCKPIT_STATE_QUASI 1
#define COCKPIT_STATE_FUNCTIONAL 2

#define COCKPIT_MODE_LEGACY 0
#define COCKPIT_MODE_WIDESCREEN 1

class ICockpit {
public:
  virtual ~ICockpit() = default;

  virtual void Init(int ship_index) = 0;
  virtual void Free() = 0;
  virtual bool IsValid() const = 0;

  virtual void Open() = 0;
  virtual void Close() = 0;
  virtual void QuickOpen() = 0;
  virtual void QuickClose() = 0;
  virtual void Resize() = 0;

  virtual void Render() = 0;
  virtual void StartShake(float mag, vector *vec) = 0;
  virtual int State() const = 0;

  virtual bsp_info *GetMonitorSubmodel(uint16_t monitor_flag) = 0;
  virtual poly_model *GetPolyModel() = 0;
};

// Global access to active cockpit instance
ICockpit *GetCockpit();
void SetCockpitMode(int mode);
int GetCockpitMode();

#endif
