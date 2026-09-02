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

// LegacyCockpit.h — Original 4:3 cockpit implementation
// This is a direct port of the original cockpit.cpp into the ICockpit
// class hierarchy. No behavioral changes from the original code.

#ifndef LEGACYCOCKPIT_H
#define LEGACYCOCKPIT_H

#include "ICockpit.h"

struct tCockpitInfo;
struct tCockpitCfgInfo;

class LegacyCockpit : public ICockpit {
public:
  LegacyCockpit();
  ~LegacyCockpit() override;

  void Init(int ship_index) override;
  void Free() override;
  bool IsValid() const override;

  void Open() override;
  void Close() override;
  void QuickOpen() override;
  void QuickClose() override;
  void Resize() override;

  void Render() override;
  void StartShake(float mag, vector *vec) override;
  int State() const override;

  bsp_info *GetMonitorSubmodel(uint16_t monitor_flag) override;
  poly_model *GetPolyModel() override;

protected:
  tCockpitInfo *GetInfo() { return &m_info; }
  const tCockpitInfo *GetInfo() const { return &m_info; }
  float KeyframeAnimate();

private:
  void LoadCockpitInfo(const char *ckt_file, tCockpitCfgInfo *info);
  static bool CockpitFileParse(const char *command, const char *operand, void *data);

  tCockpitInfo m_info;
};

#endif
