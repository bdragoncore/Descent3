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

--- HISTORICAL COMMENTS FOLLOW ---

 * $Logfile: /DescentIII/Main/loadstate.cpp $
 * $Revision: 74 $
 * $Date: 9/05/01 12:19p $
 * $Author: Matt $
 *
 * Load a saved game
 *
 * $Log: /DescentIII/Main/loadstate.cpp $
 *
 * 74    9/05/01 12:19p Matt
 * When loading a saved game, load the add-on data before the level so the
 * textures get mapped properly.
 *
 * 73    4/21/00 3:23p Matt
 * Check for bad vis effect structure on load, which probably means a game
 * saved with a prior to version 1.3.  If found, convert old struct to new
 * struct.
 *
 * 72    4/19/00 5:07p Matt
 * From Duane for 1.4
 * Added checks, asserts, and fixes for bad return values
 *
 * 71    4/07/00 5:51p Matt
 * Do id translation on dummy objects that are really generics, etc.
 *
 * 70    3/20/00 12:13p Matt
 * Merge of Duane's post-1.3 changes.
 * Fixed mem leak.
 * Added check for object type.
 *
 * 69    9/18/99 9:23p Jeff
 * re initialize objects' position history (for motion blur)
 *
 * 68    7/22/99 11:47a Chris
 *
 * 67    7/21/99 7:17p Chris
 * Fixed an attach related crash in the load/save game code
 *
 * 66    7/08/99 10:30a Kevin
 * when loading a save game >level 4 from a minimal install, don't ask for
 * CD1
 *
 * 65    5/21/99 11:15p Samir
 * savegames save hud timer and special text states. restore too.
 *
 * 64    5/20/99 2:48a Matt
 * Auto-waypoints now stored & used per player.  Manual waypoints will all
 * players, once Jason makes it work.
 *
 * 63    5/17/99 10:38a Samir
 * reset player ship and reset camera views before starting new level. may
 * fix some sequencing problems with ship reticle and inventory too.
 *
 * 62    5/12/99 6:02p Jeff
 * flip order of saving/loading objects and players
 *
 * 61    5/12/99 12:25p Kevin
 * save & restore Current_mission.game_state_flags
 *
 * 60    5/12/99 11:40a Matt
 * Changed some arrays to be malloc'd instead of declared locally on the
 * stack, because the Mac limits local data to 32K.
 *
 * 59    5/10/99 10:22p Ardussi
 * changes to compile on Mac
 *
 * 58    5/06/99 6:11p Kevin
 * fixes for save/load game syste. Also require CD again and added some
 * heat.net stuff
 *
 * 57    4/30/99 3:03p Kevin
 * improved times restored count
 *
 * 56    4/29/99 11:25a Kevin
 * Added room damage to save game
 *
 * 55    4/28/99 2:14a Samir
 * save and load Game messages in savegame.
 *
 * 54    4/23/99 10:23p Kevin
 * Fixed savegame doorways bug!
 *
 * 53    4/23/99 8:28p Kevin
 * trying to get doors working in save/load games
 *
 * 52    4/23/99 7:33p Kevin
 * save weather effects and reset the HUD to show the correct ship
 *
 * 51    4/23/99 4:50p Jeff
 * fixed page in polymodel problem
 *
 * 50    4/22/99 4:13p Matt
 * Deleted sounds array from the object struct, since it was never used.
 *
 * 49    4/16/99 12:33a Matt
 * Disable Soar on non-Windows systems.
 *
 * 48    4/15/99 5:41p Samir
 * save current music region and restore it.
 *
 * 47    4/15/99 1:40a Jeff
 * changes for linux compile
 *
 * 46    4/14/99 2:51a Jeff
 * fixed some case mismatched #includes
 *
 * 45    4/05/99 10:54a Matt
 * Added auto-waypoint system
 *
 * 44    3/23/99 6:09p Kevin
 * Marker text saved
 *
 * 43    3/23/99 12:33p Kevin
 * More post level results improvements
 *
 * 42    3/22/99 5:12p Samir
 * added snapshot to savegame.
 *
 * 41    3/11/99 9:09p Jeff
 * fixed remaining known bugs in demo system (DemoWriteObjCreate being
 * called twice and </<= problems with
 * Highest_room_index/Highest_object_index).
 *
 * 40    3/10/99 6:20p Jeff
 * many fixes to demo system.  Fixed IGC so cameras move...fixed osiris to
 * be restored correctly, and it handles errors on restore
 *
 * 39    3/10/99 2:25p Kevin
 * Save/Load and Demo file fixes
 *
 * 38    3/08/99 11:05a Jason
 * added saving/loading of texture changes
 *
 * 37    3/04/99 6:58p Jeff
 * fixed microwave assert
 *
 * 36    3/04/99 6:30p Jeff
 * saves out current waypoint and matcens
 *
 * 35    3/03/99 7:20p Jeff
 * save/restore some ai path information
 *
 * 34    3/02/99 6:39p Jeff
 * level goals working in save/load game
 *
 * 33    3/02/99 4:42p Jeff
 * Fixed inventory save/load
 *
 * 32    3/02/99 1:48p Kevin
 * Fixed collision with external rooms
 *
 * 31    3/01/99 9:03p Kevin
 * OEM Beta 4
 *
 * 30    3/01/99 11:41a Kevin
 * More progress...
 *
 * 29    2/28/99 10:50p Kevin
 *
 * $NoKeywords: $
 */

#include <algorithm>
#include <cstring>
#include <filesystem>

#include "gamesave.h"
#include "cfile.h"
#include "Mission.h"
#include "gamesequence.h"
#include "gameevent.h"
#include "gameloop.h"
#include "game.h"
#include "log.h"
#include "object.h"
#include "objinfo.h"
#include "gametexture.h"
#include "bitmap.h"
#include "ship.h"
#include "door.h"
#include "stringtable.h"
#include "weapon.h"
#include "vclip.h"
#include "viseffect.h"
#include "room.h"
#include "trigger.h"
#include "spew.h"
#include "doorway.h"
#include "AIMain.h"
#include "mem.h"
#include "osiris_dll.h"
#include "terrain.h"
#include "levelgoal.h"
#include "aistruct.h"
#include "matcen.h"
#include "pilot.h"
#include "marker.h"
#include "d3music.h"
#include "weather.h"
#include "cockpit.h"
#include "hud.h"
#include "pserror.h"

extern void PageInAllData();

// dynamically allocated to be efficient (only needed during save/load)

static int LGSSnapshot(CFILE *fp);
static void IncreaseRestoreCount(const std::filesystem::path &file);

int Times_game_restored = 0;
// static gs_tables *gs_Xlates = NULL;

gs_tables *gs_Xlates = NULL;
// int Gamesave_read_version=0;

void IncreaseRestoreCount(const std::filesystem::path &file) {
  // Open the file up in read more, read the current count, then incease it
  // and write the increased value back out.

  std::filesystem::path countpath = file;
  countpath.replace_extension(".cnt");

  CFILE *cfp = cfopen(countpath, "rb");
  if (cfp) {
    Times_game_restored = cf_ReadInt(cfp);
    cfclose(cfp);
  } else {
    Times_game_restored = 0;
  }
  Times_game_restored++;

  cfp = cfopen(countpath, "wb");
  if (cfp) {
    cf_WriteInt(cfp, Times_game_restored);
    cfclose(cfp);
  }
}

extern bool IsRestoredGame;

int LoadGameState(const std::filesystem::path &pathname) {
  int retval = LGS_OK;
  char desc[GAMESAVE_DESCLEN + 1];
  char path[_MAX_PATH];
  uint16_t version;
  uint16_t curlevel;
  int16_t pending_music_region;
  IsRestoredGame = true;
  //	load in stuff
  CFILE *fp = cfopen(pathname, "rb");
  if (!fp) {
    Int3();
    return LGS_FILENOTFOUND;
  }

  START_VERIFY_SAVEFILE(fp);
  gs_Xlates = new gs_tables;

  // read in header and do version check.
  cf_ReadBytes((uint8_t *)desc, sizeof(desc), fp);
  version = (uint16_t)cf_ReadShort(fp);
  if (version < GAMESAVE_OLDVER) {
    Int3();
    retval = LGS_OUTDATEDVER;
    goto loadsg_error;
  }

  retval = LGSSnapshot(fp); // read and clear snapshot.
  if (retval > 0) {
    bm_FreeBitmap(retval);
  }

  // Gamesave_read_version=version;

  // read translation tables
  retval = LGSXlateTables(fp);
  if (retval != LGS_OK)
    goto loadsg_error;
  //	read in gamemode info.

  //	read in mission level info.
  curlevel = (uint16_t)cf_ReadShort(fp);
  cf_ReadString(path, sizeof(path), fp);

  if ((curlevel > 4) && (stricmp(path, "d3.mn3") == 0)) {
    strcpy(path, "d3_2.mn3");
  }

  //	must load mission and initialize level before reading in other data.
  retval = LGSMission(path, curlevel);
  if (retval != LGS_OK)
    goto loadsg_error;

  Current_mission.game_state_flags = cf_ReadInt(fp);
  // Increase count for how many times this file was restored
  IncreaseRestoreCount(pathname);

  // read in information after level's been loaded.
  Gametime = cf_ReadFloat(fp);
  FrameCount = cf_ReadInt(fp);
  Current_waypoint = cf_ReadInt(fp);
  pending_music_region = cf_ReadShort(fp);
  D3MusicSetRegion(pending_music_region);

  // Times_game_restored = cf_ReadInt(fp);
  // BUGFIX #279: Portable weather deserialization.
  // Read fields individually instead of raw struct dump for cross-architecture
  // compatibility.
  {
    int weather_size = cf_ReadInt(fp);
    if (weather_size != sizeof(Weather)) {
      Int3();
      retval = LGS_OUTDATEDVER;
      goto loadsg_error;
    }
  }
  gs_ReadInt(fp, Weather.flags);
  gs_ReadFloat(fp, Weather.snow_intensity_scalar);
  gs_ReadFloat(fp, Weather.rain_intensity_scalar);
  gs_ReadInt(fp, Weather.rain_color);
  gs_ReadInt(fp, Weather.lightning_color);
  gs_ReadInt(fp, Weather.sky_flash_color);
  gs_ReadByte(fp, Weather.lightning_sequence);
  gs_ReadFloat(fp, Weather.last_lightning_evaluation_time);
  gs_ReadFloat(fp, Weather.lightning_interval_time);
  gs_ReadInt(fp, Weather.lightning_rand_value);
  gs_ReadInt(fp, Weather.snowflakes_to_create);

  // Restore active doorways
  {
    int num_active_dw = cf_ReadInt(fp);
    Num_active_doorways = cf_ReadInt(fp);

    for (int d = 0; d < num_active_dw; d++) {
      Active_doorways[d] = cf_ReadInt(fp);
    }
  }

  // read in room info
  retval = LGSRooms(fp);
  if (retval != LGS_OK)
    goto loadsg_error;

  // read in trigger info
  retval = LGSTriggers(fp);
  if (retval != LGS_OK)
    goto loadsg_error;

  // read in object info.
  retval = LGSObjects(fp, version);
  if (retval != LGS_OK)
    goto loadsg_error;

  // read in  players info
  retval = LGSPlayers(fp);
  if (retval != LGS_OK)
    goto loadsg_error;

  // read in matcen info.
  retval = LGSMatcens(fp);
  if (retval != LGS_OK)
    goto loadsg_error;

  // read in viseffecsts
  retval = LGSVisEffects(fp);
  if (retval != LGS_OK)
    goto loadsg_error;

  // read in spew
  retval = LGSSpew(fp);
  if (retval != LGS_OK)
    goto loadsg_error;

  // Load OSIRIS Stuff
  if (!Osiris_RestoreSystemState(fp)) {
    goto loadsg_error;
  }

  // Load Level goal info
  Level_goals.LoadLevelGoalInfo(fp);

  // read in game messages for console. (must occur AFTER players are read in!!!)
  LGSGameMessages(fp);

  // loads and restores hud state.
  if (!LGSHudState(fp)) {
    retval = LGS_OBJECTSCORRUPT;
    goto loadsg_error;
  }

loadsg_error:
  delete gs_Xlates;

  END_VERIFY_SAVEFILE(fp, "Total load");
  cfclose(fp);

  // Page everything in here!
  PageInAllData();

  IncrementPilotRestoredGamesForMission(&Current_pilot, Current_mission.name);

  return retval;
}

bool GetGameStateInfo(const std::filesystem::path &pathname, char *description, int *bm_handle) {
  int bitmap;
  char desc[GAMESAVE_DESCLEN + 1];

  CFILE *fp = cfopen(pathname, "rb");
  if (!fp)
    return false;

  if (!cf_ReadBytes((uint8_t *)desc, GAMESAVE_DESCLEN + 1, fp)) {
    strcpy(description, TXT_ILLEGALSAVEGAME);
    cfclose(fp);
    return false;
  }

  strcpy(description, desc);

  if (bm_handle) {
    cf_ReadShort(fp); // skip version.
    bitmap = LGSSnapshot(fp);
    *bm_handle = bitmap;
  }

  cfclose(fp);
  return true;
}

//////////////////////////////////////////////////////////////////////////////
#define BUILD_XLATE_TABLE(_table, _maxtable, _fn)                                                                      \
  do {                                                                                                                 \
    gs_ReadShort(fp, num);                                                                                             \
    for (i = 0; i < num; i++) {                                                                                        \
      cf_ReadString(name, sizeof(name), fp);                                                                           \
      index = _fn(name);                                                                                               \
      _table[i] = (index == -1) ? 0 : index;                                                                           \
    }                                                                                                                  \
    for (; i < _maxtable; i++)                                                                                         \
      _table[i] = 0;                                                                                                   \
  } while (0)

#define BUILD_MINI_XLATE_TABLE(_table, _fn)                                                                            \
  memset(_table, 0, sizeof(_table));                                                                                   \
  do {                                                                                                                 \
    gs_ReadShort(fp, i);                                                                                               \
    cf_ReadString(name, sizeof(name), fp);                                                                             \
    if (i == -1 && name[0] == 0)                                                                                       \
      break;                                                                                                           \
    index = _fn(name);                                                                                                 \
    _table[i] = (index == -1) ? 0 : index;                                                                             \
  } while (1)

//	reads in translation tables
int LGSXlateTables(CFILE *fp) {
  START_VERIFY_SAVEFILE(fp);
  int retval = LGS_OK;
  int16_t i, num, index;
  char name[64];

  //	load object info translation table
  BUILD_XLATE_TABLE(gs_Xlates->obji_indices, MAX_OBJECT_IDS, FindObjectIDName);

  //	load polymodel translation list.
  BUILD_XLATE_TABLE(gs_Xlates->model_handles, MAX_POLY_MODELS, FindPolyModelName);

  // load doar translation list.
  BUILD_XLATE_TABLE(gs_Xlates->door_handles, MAX_DOORS, FindDoorName);

  // load ship translation list.
  BUILD_XLATE_TABLE(gs_Xlates->ship_handles, MAX_SHIPS, FindShipName);

  // build weapon translation list
  BUILD_XLATE_TABLE(gs_Xlates->wpn_handles, MAX_WEAPONS, FindWeaponName);

  // read in limited texture name list.
  BUILD_XLATE_TABLE(gs_Xlates->tex_handles, MAX_TEXTURES, FindTextureName);

  // read in limited bitmap name list.  this is a slightly diff format than above to save space
  BUILD_MINI_XLATE_TABLE(gs_Xlates->bm_handles, bm_FindBitmapName);

  END_VERIFY_SAVEFILE(fp, "Xlate load");
  return retval;
}

//	loads in level's mission and level.
int LGSMission(const char *msnname, int level) {
  //	we will free the mission.
  //	Free any game objects/etc that needs to be done when ending a level here.
  FreeScriptsForLevel();

  Osiris_DisableCreateEvents();
  if (LoadMission((const char *)msnname)) {
    SetCurrentLevel(level);
    Player_num = 0; // Reset player num
    Players[Player_num].ship_index = FindShipName(DEFAULT_SHIP);
    ASSERT(Players[Player_num].ship_index != -1);

    // Load any addon data
    mng_LoadAddonPages();

    InitPlayerNewShip(Player_num, INVRESET_ALL);
    InitCameraViews(1); // Turn off all camera views, including rear views

    if (!LoadAndStartCurrentLevel()) {
      Int3();
      Osiris_EnableCreateEvents();
      return LGS_STARTLVLFAILED;
    }
  } else {
    Int3();
    Osiris_EnableCreateEvents();
    return LGS_MISSIONFAILED;
  }
  Osiris_EnableCreateEvents();
  // mng_LoadAddonPages ();
  return LGS_OK;
}

extern uint8_t AutomapVisMap[MAX_ROOMS];

//	initializes rooms
int LGSRooms(CFILE *fp) {
  int retval = LGS_OK;
  int16_t i, p, highest_index;

  gs_ReadShort(fp, highest_index);
  if (highest_index != (int16_t)Highest_room_index) {
    Int3();
    return LGS_CORRUPTLEVEL;
  }

  int16_t num_rooms;
  gs_ReadShort(fp, num_rooms);

  for (i = 0; i < num_rooms; i++) {
    gs_ReadByte(fp, AutomapVisMap[i]);
  }

  for (i = 0; i <= highest_index; i++) {
    uint8_t used;
    gs_ReadByte(fp, used);
    if (used) {
      // reset lists
      Rooms[i].objects = -1;
      Rooms[i].vis_effects = -1;

      // we need to save some room info out.
      gs_ReadInt(fp, Rooms[i].flags);
      gs_ReadByte(fp, Rooms[i].pulse_time);
      gs_ReadByte(fp, Rooms[i].pulse_offset);
      gs_ReadVector(fp, Rooms[i].wind);
      gs_ReadFloat(fp, Rooms[i].last_render_time);
      gs_ReadFloat(fp, Rooms[i].fog_depth);
      gs_ReadFloat(fp, Rooms[i].fog_r);
      gs_ReadFloat(fp, Rooms[i].fog_g);
      gs_ReadFloat(fp, Rooms[i].fog_b);
      gs_ReadFloat(fp, Rooms[i].damage);
      //	gs_ReadInt(fp, Rooms[i].objects);
      //??	gs_WriteFloat(fp, Rooms[i].ambient_sound); // need to save an index of sounds.

      // Find out about texture changes
      // if (Gamesave_read_version>=1)
      //{
      int num_changed = cf_ReadShort(fp);

      for (int t = 0; t < num_changed; t++) {
        int facenum = cf_ReadShort(fp);
        Rooms[i].faces[facenum].tmap = gs_Xlates->tex_handles[cf_ReadShort(fp)];
        Rooms[i].faces[facenum].flags |= FF_TEXTURE_CHANGED;
      }
      //}
      for (p = 0; p < Rooms[i].num_portals; p++) {
        gs_ReadInt(fp, Rooms[i].portals[p].flags);
      }

      // load doorway info
      if (Rooms[i].flags & RF_DOOR) {
        doorway *dp = Rooms[i].doorway_data;
        ASSERT(dp != NULL);
        gs_ReadByte(fp, dp->state);
        gs_ReadByte(fp, dp->flags);
        gs_ReadByte(fp, dp->keys_needed);
        gs_ReadFloat(fp, dp->position);
        gs_ReadFloat(fp, dp->dest_pos);
        gs_ReadInt(fp, dp->sound_handle);
        gs_ReadInt(fp, dp->activenum);
        gs_ReadInt(fp, dp->doornum);
      }
    }
  }

  // Rebuild the active doorway list
  // DoorwayRebuildActiveList();

  return retval;
}

// TODO: MTS: unused?
//	loads in and sets these events
int LGSEvents(CFILE *fp) {
  int retval = LGS_OK;

  // clear events
  ClearAllEvents();

  return retval;
}

//	loads in and sets these triggers
int LGSTriggers(CFILE *fp) {
  int16_t i, n_trigs = 0;

  gs_ReadShort(fp, n_trigs);
  if (n_trigs != (int16_t)Num_triggers) {
    Int3();
    return LGS_CORRUPTLEVEL;
  }

  for (i = 0; i < n_trigs; i++) {

    // must free current trigger script if there's one allocated.
    FreeTriggerScript(&Triggers[i]);

    gs_ReadShort(fp, Triggers[i].flags);
    gs_ReadShort(fp, Triggers[i].activator);

    // read script info
    //@@		script_info script;
    //@@
    //@@		scrmem = LGSScript(fp, &script, &is_scripted, &scrmemsize);
    //@@		if (is_scripted) {
    //@@			ReinitTriggerScripts(&Triggers[i], &script, scrmemsize, scrmem);
    //@@		}
    //@@		if (scrmem)
    //@@			delete[] scrmem;
    //@@		script.set_name(NULL);
    //@@		script.set_parms(0, NULL);
  }

  return LGS_OK;
}

struct old_vis_attach_info {
  int obj_handle, dest_objhandle;
  uint8_t subnum, subnum2;

  uint16_t modelnum;
  uint16_t vertnum, end_vertnum;
};

struct old_vis_effect {
  uint8_t type;
  uint8_t id;

  vector pos;

  vector velocity;
  float mass;
  float drag;
  float size;
  float lifeleft;
  float lifetime;
  float creation_time;

  int roomnum;
  uint16_t flags;
  int phys_flags;
  uint8_t movement_type;
  int16_t custom_handle;
  uint16_t lighting_color;

  vis_attach_info attach_info;
  axis_billboard_info billboard_info;

  vector end_pos;

  int16_t next;
  int16_t prev;
};

void CopyVisStruct(vis_effect *vis, old_vis_effect *old_vis) {
  // Copy fields over
  vis->type = old_vis->type;
  vis->id = old_vis->id;
  vis->pos = old_vis->pos;

  vis->velocity = old_vis->velocity;
  vis->mass = old_vis->mass;
  vis->drag = old_vis->drag;
  vis->size = old_vis->size;
  vis->lifeleft = old_vis->lifeleft;
  vis->lifetime = old_vis->lifetime;
  vis->creation_time = old_vis->creation_time;

  vis->roomnum = old_vis->roomnum;
  vis->flags = old_vis->flags;
  vis->phys_flags = old_vis->phys_flags;
  vis->movement_type = old_vis->movement_type;
  vis->custom_handle = old_vis->custom_handle;
  vis->lighting_color = old_vis->lighting_color;

  vis->attach_info.obj_handle = old_vis->attach_info.obj_handle;
  vis->attach_info.dest_objhandle = old_vis->attach_info.dest_objhandle;
  vis->attach_info.subnum = old_vis->attach_info.subnum;
  vis->attach_info.subnum2 = old_vis->attach_info.subnum2;
  vis->attach_info.modelnum = old_vis->attach_info.modelnum;
  vis->attach_info.vertnum = old_vis->attach_info.vertnum;
  vis->attach_info.end_vertnum = old_vis->attach_info.end_vertnum;

  vis->billboard_info = old_vis->billboard_info;
  vis->end_pos = old_vis->end_pos;
}

// save viseffects
// BUGFIX #279: Portable vis_effect deserialization.
// Read fields individually instead of raw struct dump for cross-architecture
// compatibility.
int LGSVisEffects(CFILE *fp) {
  int16_t i, count = 0;

  gs_ReadShort(fp, count);

  // reassemble viseffect list.
  for (i = 0; i < count; i++) {
    int roomnum, v;
    vis_effect vis;

    // BUGFIX #279: Read fields individually instead of raw struct dump
    memset(&vis, 0, sizeof(vis));
    gs_ReadVector(fp, vis.pos);
    gs_ReadVector(fp, vis.end_pos);
    gs_ReadVector(fp, vis.velocity);
    gs_ReadFloat(fp, vis.mass);
    gs_ReadFloat(fp, vis.drag);
    gs_ReadFloat(fp, vis.size);
    gs_ReadFloat(fp, vis.lifeleft);
    gs_ReadFloat(fp, vis.lifetime);
    gs_ReadFloat(fp, vis.creation_time);
    gs_ReadInt(fp, vis.roomnum);
    gs_ReadInt(fp, vis.phys_flags);
    gs_ReadShort(fp, vis.custom_handle);
    gs_ReadShort(fp, vis.lighting_color);
    gs_ReadShort(fp, vis.flags);
    gs_ReadShort(fp, vis.next);
    gs_ReadShort(fp, vis.prev);
    // vis_attach_info
    gs_ReadInt(fp, vis.attach_info.obj_handle);
    gs_ReadInt(fp, vis.attach_info.dest_objhandle);
    gs_ReadShort(fp, vis.attach_info.modelnum);
    gs_ReadShort(fp, vis.attach_info.vertnum);
    gs_ReadShort(fp, vis.attach_info.end_vertnum);
    gs_ReadByte(fp, vis.attach_info.subnum);
    gs_ReadByte(fp, vis.attach_info.subnum2);
    // axis_billboard_info
    gs_ReadByte(fp, vis.billboard_info.width);
    gs_ReadByte(fp, vis.billboard_info.height);
    gs_ReadByte(fp, vis.billboard_info.texture);
    gs_ReadByte(fp, vis.movement_type);
    gs_ReadByte(fp, vis.type);
    gs_ReadByte(fp, vis.id);

    roomnum = vis.roomnum;

    vis.roomnum = vis.prev = vis.next = -1;
    v = VisEffectAllocate();
    if (v >= 0) { // DAJ -1FIX
      memcpy(&VisEffects[v], &vis, sizeof(vis_effect));
      VisEffectLink(v, roomnum);
    }
  }

  return LGS_OK;
}

// players
int LGSPlayers(CFILE *fp) {
  player *plr = &Players[0];
  int16_t size;

  // must do this if we read player struct as whole.
  plr->inventory.Reset(false, INVRESET_ALL);
  plr->counter_measures.Reset(false, INVRESET_ALL);

  gs_ReadShort(fp, size);
  if (size != sizeof(player)) {
    Int3();
    return LGS_OUTDATEDVER;
  }

  // BUGFIX #279: Read all fields individually (portable serialization).
  // The old code read the entire player struct with cf_ReadBytes, which broke
  // cross-architecture compatibility because the struct contains pointer
  // fields (guided_obj, user_timeout_obj) and the Inventory C++ class.
  gs_ReadInt(fp, plr->start_index);
  gs_ReadVector(fp, plr->start_pos);
  gs_ReadInt(fp, plr->start_roomnum);
  gs_ReadMatrix(fp, plr->start_orient);
  gs_ReadInt(fp, plr->startpos_flags);
  gs_ReadInt(fp, plr->ship_index);

  cf_ReadBytes((uint8_t *)plr->callsign, sizeof(plr->callsign), fp);

  gs_ReadInt(fp, plr->flags);
  gs_ReadInt(fp, plr->score);
  gs_ReadFloat(fp, plr->damage_magnitude);
  gs_ReadFloat(fp, plr->edrain_magnitude);
  gs_ReadFloat(fp, plr->invul_magnitude);
  gs_ReadFloat(fp, plr->energy);
  gs_ReadByte(fp, plr->lives);
  gs_ReadByte(fp, plr->level);
  gs_ReadByte(fp, plr->starting_level);
  gs_ReadByte(fp, plr->keys);
  gs_ReadShort(fp, plr->killer_objnum);
  gs_ReadFloat(fp, plr->invulnerable_time);
  gs_ReadFloat(fp, plr->last_hit_wall_sound_time);
  gs_ReadFloat(fp, plr->last_homing_warning_sound_time);
  gs_ReadFloat(fp, plr->last_thrust_time);
  gs_ReadFloat(fp, plr->last_afterburner_time);
  gs_ReadShort(fp, plr->objnum);
  gs_ReadByte(fp, plr->team);

  gs_ReadInt(fp, plr->current_auto_waypoint_room);

  gs_ReadFloat(fp, plr->time_level);
  gs_ReadFloat(fp, plr->time_total);
  gs_ReadInt(fp, plr->num_hits_level);
  gs_ReadInt(fp, plr->num_discharges_level);
  gs_ReadShort(fp, plr->num_kills_level);
  gs_ReadShort(fp, plr->friendly_kills_level);
  gs_ReadShort(fp, plr->num_kills_total);

  gs_ReadInt(fp, plr->weapon_flags);
  for (int i = 0; i < MAX_PLAYER_WEAPONS; i++) {
    gs_ReadShort(fp, plr->weapon_ammo[i]);
  }

  for (int i = 0; i < 2; i++) {
    gs_ReadInt(fp, plr->weapon[i].index);
    gs_ReadFloat(fp, plr->weapon[i].firing_time);
    gs_ReadInt(fp, plr->weapon[i].sound_handle);
  }
  gs_ReadByte(fp, plr->laser_level);

  gs_ReadFloat(fp, plr->light_dist);
  gs_ReadFloat(fp, plr->r);
  gs_ReadFloat(fp, plr->g);
  gs_ReadFloat(fp, plr->b);

  gs_ReadFloat(fp, plr->ballspeed);
  gs_ReadByte(fp, plr->num_balls);
  for (int i = 0; i < 3; i++) {
    gs_ReadFloat(fp, plr->ball_r[i]);
    gs_ReadFloat(fp, plr->ball_g[i]);
    gs_ReadFloat(fp, plr->ball_b[i]);
  }

  gs_ReadInt(fp, plr->oldroom);

  gs_ReadFloat(fp, plr->last_fire_weapon_time);

  gs_ReadFloat(fp, plr->afterburner_mag);
  gs_ReadFloat(fp, plr->thrust_mag);
  gs_ReadInt(fp, plr->afterburner_sound_handle);
  gs_ReadFloat(fp, plr->afterburn_time_left);

  gs_ReadInt(fp, plr->thruster_sound_handle);
  gs_ReadInt(fp, plr->thruster_sound_state);

  gs_ReadInt(fp, plr->small_left_obj);
  gs_ReadInt(fp, plr->small_right_obj);
  gs_ReadInt(fp, plr->small_dll_obj);

  gs_ReadByte(fp, plr->multiplayer_flags);
  gs_ReadByte(fp, plr->last_multiplayer_flags);
  gs_ReadFloat(fp, plr->last_guided_time);

  cf_ReadBytes((uint8_t *)plr->tracker_id, sizeof(plr->tracker_id), fp);
  gs_ReadInt(fp, plr->kills);
  gs_ReadInt(fp, plr->deaths);
  gs_ReadInt(fp, plr->suicides);
  gs_ReadFloat(fp, plr->rank);
  gs_ReadFloat(fp, plr->lateral_thrust);
  gs_ReadFloat(fp, plr->rotational_thrust);
  gs_ReadInt(fp, plr->time_in_game);

  // BUGFIX #279: guided_obj and user_timeout_obj are pointer fields that differ
  // in size between x86 (4 bytes) and x64 (8 bytes). They are serialized as
  // object handles (int32_t) and resolved to pointers here.
  {
    int32_t guided_handle, user_timeout_handle;
    gs_ReadInt(fp, guided_handle);
    gs_ReadInt(fp, user_timeout_handle);
    plr->guided_obj = (guided_handle != OBJECT_HANDLE_NONE) ? &Objects[guided_handle & HANDLE_OBJNUM_MASK] : NULL;
    plr->user_timeout_obj =
        (user_timeout_handle != OBJECT_HANDLE_NONE) ? &Objects[user_timeout_handle & HANDLE_OBJNUM_MASK] : NULL;
  }

  gs_ReadFloat(fp, plr->zoom_distance);

  gs_ReadFloat(fp, plr->movement_scalar);
  gs_ReadFloat(fp, plr->damage_scalar);
  gs_ReadFloat(fp, plr->armor_scalar);
  gs_ReadFloat(fp, plr->turn_scalar);
  gs_ReadFloat(fp, plr->weapon_recharge_scalar);
  gs_ReadFloat(fp, plr->weapon_speed_scalar);

  gs_ReadInt(fp, plr->piggy_objnum);
  gs_ReadInt(fp, plr->piggy_sig);

  gs_ReadInt(fp, plr->custom_texture_handle);

  gs_ReadInt(fp, plr->ship_permissions);

  gs_ReadVector(fp, plr->invul_vector);

  gs_ReadInt(fp, plr->controller_bitflags);

  gs_ReadShort(fp, plr->num_markers);

  gs_ReadShort(fp, plr->num_deaths_level);
  gs_ReadShort(fp, plr->num_deaths_total);

  // Read inventory and countermeasures
  plr->inventory.ReadInventory(fp);
  plr->counter_measures.ReadInventory(fp);

  int ship_index = Players[Player_num].ship_index;

  if (ship_index < 0)
    ship_index = 0;

  FreeCockpit();
  CloseShipHUD();
  InitShipHUD(ship_index);
  InitCockpit(ship_index);

  return LGS_OK;
}

extern int Physics_NumLinked;
extern int PhysicsLinkList[MAX_OBJECTS];

int inreadobj = 0;

void VerifySaveGame(CFILE *fp) {
  int testint;
  testint = cf_ReadInt(fp);
  ASSERT(testint == 0xF00D4B0B);
}

#define VERIFY_SAVEFILE
// VerifySaveGame(fp)
extern char MarkerMessages[MAX_PLAYERS * 2][MAX_MARKER_MESSAGE_LENGTH];
extern int Marker_message;
//	loads in and sets these objects
int LGSObjects(CFILE *fp, int version) {

  inreadobj = 1;
  int retval = LGS_OK;
  vector pos, last_pos;
  int roomnum;
  int i, j, highest_index;
  int max_terr;

  auto objmat = mem_rmalloc<matrix>(MAX_OBJECTS);

  Osiris_DisableCreateEvents();
  // we must reset some data before continuing.
  InitBigObjects();

  START_VERIFY_SAVEFILE(fp);
  Marker_message = cf_ReadInt(fp);
  int num_marker_msgs = cf_ReadShort(fp);
  for (i = 0; i < num_marker_msgs; i++) {
    int msg_len = cf_ReadShort(fp);
    cf_ReadBytes((uint8_t *)MarkerMessages[i], msg_len, fp);
  }

  highest_index = (int)cf_ReadShort(fp);

  // load what objects are stuck to each other
  int max_num_linked = cf_ReadInt(fp);
  Physics_NumLinked = cf_ReadInt(fp);
  for (i = 0; i < max_num_linked; i++) {
    PhysicsLinkList[i] = cf_ReadInt(fp);
  }

  // load AI information
  //////////////////////////////////
  int num_read_max_dynamic_paths = cf_ReadInt(fp);
  int num_read_max_nodes = cf_ReadInt(fp);

  int num_dp_to_read = std::min(MAX_DYNAMIC_PATHS, num_read_max_dynamic_paths);
  int num_dp_to_skip =
      (MAX_DYNAMIC_PATHS < num_read_max_dynamic_paths) ? num_read_max_dynamic_paths - MAX_DYNAMIC_PATHS : 0;

  int num_n_to_read = std::min(MAX_NODES, num_read_max_nodes);
  int num_n_to_skip = (MAX_NODES < num_read_max_nodes) ? num_read_max_nodes - MAX_NODES : 0;

  int s;
  for (i = 0; i < num_dp_to_read; i++) {
    AIDynamicPath[i].num_nodes = cf_ReadShort(fp);
    AIDynamicPath[i].use_count = cf_ReadShort(fp);
    AIDynamicPath[i].owner_handle = cf_ReadInt(fp);

    for (s = 0; s < num_n_to_read; s++) {
      AIDynamicPath[i].pos[s].x() = cf_ReadFloat(fp);
      AIDynamicPath[i].pos[s].y() = cf_ReadFloat(fp);
      AIDynamicPath[i].pos[s].z() = cf_ReadFloat(fp);
      AIDynamicPath[i].roomnum[s] = cf_ReadInt(fp);
    }

    for (s = 0; s < num_n_to_skip; s++) {
      cf_ReadFloat(fp);
      cf_ReadFloat(fp);
      cf_ReadFloat(fp);
      cf_ReadInt(fp);
    }
  }

  for (i = 0; i < num_dp_to_skip; i++) {
    cf_ReadShort(fp);
    cf_ReadShort(fp);
    cf_ReadInt(fp);

    for (s = 0; s < num_n_to_read; s++) {
      cf_ReadFloat(fp);
      cf_ReadFloat(fp);
      cf_ReadFloat(fp);
      cf_ReadInt(fp);
    }

    for (s = 0; s < num_n_to_skip; s++) {
      cf_ReadFloat(fp);
      cf_ReadFloat(fp);
      cf_ReadFloat(fp);
      cf_ReadInt(fp);
    }
  }

  int num_read_rooms = cf_ReadInt(fp);
  int num_r_to_read = std::min(MAX_ROOMS, num_read_rooms);
  int num_r_to_skip = (MAX_ROOMS < num_read_rooms) ? num_read_rooms - MAX_ROOMS : 0;

  AIAltPathNumNodes = cf_ReadInt(fp);
  for (i = 0; i < num_r_to_read; i++) {
    AIAltPath[i] = cf_ReadInt(fp);
  }

  for (i = 0; i < num_r_to_skip; i++) {
    cf_ReadInt(fp);
  }

  //////////////////////////////////

  Osiris_DisableEvents(OEM_OBJECTS | OEM_TRIGGERS | OEM_LEVELS);
  for (i = 0; i < MAX_OBJECTS; i++) {
    if (Objects[i].lighting_render_type != LRT_LIGHTMAPS) {
      if (Objects[i].type != OBJ_NONE) {
        Objects[i].next = -1;
        Objects[i].prev = -1;
        ObjDelete(i);
      }
    } else {
      Objects[i].next = -1;
      Objects[i].prev = -1;
      ObjUnlink(i);
    }
  }
  Osiris_EnableEvents(OEM_OBJECTS | OEM_TRIGGERS | OEM_LEVELS);
  for (i = 0; i <= highest_index; i++) {
    object *op;
    weapon *wpn;
    ship *shp;
    object_info *obji;
    door *door;
    int index, nattach, new_model, handle;
    uint8_t type, dummy_type;
    int16_t sindex, size;

    //	if((i==98)||(i==100))
    //		Int3();
    int sig;
    gs_ReadInt(fp, sig);

    ASSERT(sig == 0xBADB0B);

    gs_ReadByte(fp, type);

    // skip rooms and deleted objects.  if this slot currently contains a real object, it should be
    // destroyed (unlinked, deallocated, etc) to match the new object list.
    if (type == OBJ_NONE) {
      Osiris_DisableEvents(OEM_OBJECTS | OEM_TRIGGERS | OEM_LEVELS);
      if (Objects[i].type != OBJ_NONE)
        ObjDelete(i);
      Osiris_EnableEvents(OEM_OBJECTS | OEM_TRIGGERS | OEM_LEVELS);
      continue;
    }
    uint8_t l_rend_type;
    gs_ReadByte(fp, l_rend_type);

    // See if the object has changed from its original lightmap type
    // DAJ added check for type
    if ((Objects[i].type != OBJ_NONE) && (Objects[i].lighting_render_type == LRT_LIGHTMAPS) &&
        (l_rend_type != LRT_LIGHTMAPS)) {
      Osiris_DisableEvents(OEM_OBJECTS | OEM_TRIGGERS | OEM_LEVELS);
      ObjDelete(i);
      Osiris_EnableEvents(OEM_OBJECTS | OEM_TRIGGERS | OEM_LEVELS);
    }

    op = &Objects[i];
    op->lighting_render_type = l_rend_type;

    // Store whether or not we have a pointer to lighting_info
    // BUGFIX #279: Read light_info fields individually for portability
    uint8_t has_lightinfo;
    gs_ReadByte(fp, has_lightinfo);
    if (has_lightinfo) {
      if (!op->lighting_info) {
        op->lighting_info = mem_rmalloc<light_info>();
      }
      gs_ReadInt(fp, op->lighting_info->flags);
      gs_ReadFloat(fp, op->lighting_info->light_distance);
      gs_ReadFloat(fp, op->lighting_info->red_light1);
      gs_ReadFloat(fp, op->lighting_info->green_light1);
      gs_ReadFloat(fp, op->lighting_info->blue_light1);
      gs_ReadFloat(fp, op->lighting_info->red_light2);
      gs_ReadFloat(fp, op->lighting_info->green_light2);
      gs_ReadFloat(fp, op->lighting_info->blue_light2);
      gs_ReadFloat(fp, op->lighting_info->time_interval);
      gs_ReadFloat(fp, op->lighting_info->flicker_distance);
      gs_ReadFloat(fp, op->lighting_info->directional_dot);
      gs_ReadInt(fp, op->lighting_info->timebits);
      gs_ReadByte(fp, op->lighting_info->angle);
      gs_ReadByte(fp, op->lighting_info->lighting_render_type);
    }

    // validate handle.
    gs_ReadInt(fp, handle);
    if ((handle & HANDLE_OBJNUM_MASK) != i) {
      Int3();
      retval = LGS_OBJECTSCORRUPT;
      goto done;
    }

    // read in the rest.
    gs_ReadByte(fp, dummy_type);

    //	positional information (not assigned to object 'cause of Unlink)
    gs_ReadInt(fp, roomnum);
    gs_ReadVector(fp, pos);
    gs_ReadVector(fp, last_pos);
    gs_ReadMatrix(fp, objmat[i]);

    // object name
    gs_ReadByte(fp, j);
    if (j > 0) {
      if (op->name)
        mem_free(op->name);
      op->name = (char *)mem_malloc(j + 1);
      if (!op->name)
        Error("Out of memory");

      cf_ReadBytes((uint8_t *)op->name, j, fp);
      op->name[j] = '\0';
    } else {
      op->name = NULL;
    }

    //	Kill any script thread associated with this object.

    if (op->type != OBJ_NONE) {
      if (Objects[i].lighting_render_type != LRT_LIGHTMAPS)
        FreeObjectScripts(op, false);

      // Free up effects memory
      if (op->effect_info) {
        mem_free(op->effect_info);
        op->effect_info = NULL;
      }
      if (op->ai_info != NULL) {
        AIDestroyObj(op);
        mem_free(op->ai_info);
        op->ai_info = NULL;
      }
      if (op->dynamic_wb != NULL) {
        mem_free(op->dynamic_wb);
        op->dynamic_wb = NULL;
      }
      if (op->attach_children != NULL) {
        mem_free(op->attach_children);
        op->attach_children = NULL;
      }
    }
    op->type = type;
    op->handle = handle;
    op->dummy_type = dummy_type;

    op->roomnum = roomnum;
    op->pos = pos;
    op->last_pos = last_pos;

    //	data universal to all objects
    gs_ReadShort(fp, op->id);

    // Get type, and xlate if dummy
    if (type == OBJ_DUMMY)
      type = op->dummy_type;

    // xlate id to new id.
    switch (type) {
    case OBJ_ROBOT:
    case OBJ_POWERUP:
    case OBJ_BUILDING:
    case OBJ_CLUTTER:
      op->id = gs_Xlates->obji_indices[op->id];
      obji = &Object_info[op->id];
      break;
    case OBJ_DOOR:
      op->id = gs_Xlates->door_handles[op->id];
      door = &Doors[op->id];
      break;
    case OBJ_WEAPON:
      op->id = gs_Xlates->wpn_handles[op->id];
      wpn = &Weapons[op->id];
      break;
    case OBJ_PLAYER:
      shp = &Ships[Players[op->id].ship_index];
      break;
    }

    gs_ReadInt(fp, op->flags);

    op->flags |= OF_SERVER_OBJECT;

    gs_ReadByte(fp, op->control_type);
    gs_ReadByte(fp, op->movement_type);
    gs_ReadByte(fp, op->render_type);

    gs_ReadShort(fp, op->renderframe);
    gs_ReadFloat(fp, op->size);
    gs_ReadFloat(fp, op->shields);
    gs_ReadByte(fp, op->contains_type);
    gs_ReadByte(fp, op->contains_id);
    gs_ReadByte(fp, op->contains_count);
    gs_ReadFloat(fp, op->creation_time);
    gs_ReadFloat(fp, op->lifeleft);
    gs_ReadFloat(fp, op->lifetime);
    gs_ReadInt(fp, op->parent_handle);

    // attachment info. free old info and load in new.
    gs_ReadInt(fp, op->attach_ultimate_handle);
    gs_ReadInt(fp, op->attach_parent_handle);

    gs_ReadInt(fp, nattach);
    if (nattach) {
      int f_allocated;

      if (version >= 2)
        gs_ReadInt(fp, f_allocated);
      else
        f_allocated = 1;

      if (f_allocated) {
        // mprintf(0,"Object %d has %d attach points.\n",i,nattach);
        op->attach_children = mem_rmalloc<int>(nattach);
        for (j = 0; j < nattach; j++)
          gs_ReadInt(fp, op->attach_children[j]);
      }
    }

    VERIFY_SAVEFILE;

    gs_ReadByte(fp, op->attach_type);
    gs_ReadShort(fp, op->attach_index);
    gs_ReadFloat(fp, op->attach_dist);
    gs_ReadVector(fp, op->min_xyz);
    gs_ReadVector(fp, op->max_xyz);
    gs_ReadFloat(fp, op->impact_size);
    gs_ReadFloat(fp, op->impact_time);
    gs_ReadFloat(fp, op->impact_player_damage);
    gs_ReadFloat(fp, op->impact_generic_damage);
    gs_ReadFloat(fp, op->impact_force);

    // custom default script info
    gs_ReadByte(fp, j);
    if (j > 0) {
      op->custom_default_script_name = (char *)mem_malloc(j + 1);
      if (!op->custom_default_script_name)
        Error("Out of memory");

      cf_ReadBytes((uint8_t *)op->custom_default_script_name, j, fp);
      op->custom_default_script_name[j] = '\0';
    } else {
      op->custom_default_script_name = NULL;
    }

    gs_ReadByte(fp, j);
    if (j > 0) {
      op->custom_default_module_name = (char *)mem_malloc(j + 1);
      if (!op->custom_default_module_name)
        Error("Out of memory");

      cf_ReadBytes((uint8_t *)op->custom_default_module_name, j, fp);
      op->custom_default_module_name[j] = '\0';
    } else {
      op->custom_default_module_name = NULL;
    }

    VERIFY_SAVEFILE;

    gs_ReadShort(fp, op->position_counter);

    VERIFY_SAVEFILE;

    // BUGFIX #279: Read union data based on the active type tag rather than
    // raw struct dumps. This matches the portable format written by SGSObjects.
    // movement info.
    {
      uint8_t mt_type;
      gs_ReadByte(fp, mt_type);
      op->movement_type = mt_type;
      switch (mt_type) {
      case MT_PHYSICS: {
        physics_info *pi = &op->mtype.phys_info;
        gs_ReadVector(fp, pi->velocity);
        gs_ReadVector(fp, pi->thrust);
        gs_ReadVector(fp, pi->rotvel);
        gs_ReadVector(fp, pi->rotthrust);
        gs_ReadAngle(fp, pi->turnroll);
        gs_ReadFloat(fp, pi->last_still_time);
        gs_ReadInt(fp, pi->num_bounces);
        gs_ReadFloat(fp, pi->coeff_restitution);
        gs_ReadFloat(fp, pi->mass);
        gs_ReadFloat(fp, pi->drag);
        gs_ReadFloat(fp, pi->rotdrag);
        gs_ReadFloat(fp, pi->full_thrust);
        gs_ReadFloat(fp, pi->full_rotthrust);
        gs_ReadFloat(fp, pi->max_turnroll_rate);
        gs_ReadFloat(fp, pi->turnroll_ratio);
        gs_ReadFloat(fp, pi->wiggle_amplitude);
        gs_ReadFloat(fp, pi->wiggles_per_sec);
        gs_ReadVector(fp, pi->dest_pos);
        gs_ReadInt(fp, pi->stuck_room);
        gs_ReadInt(fp, pi->stuck_portal);
        gs_ReadInt(fp, pi->flags);
        break;
      }
      case MT_SHOCKWAVE: {
        shockwave_info *si = &op->mtype.shock_info;
        for (int d = 0; d < (MAX_OBJECTS / 32) + 1; d++) {
          gs_ReadInt(fp, si->damaged_list[d]);
        }
        break;
      }
      case MT_OBJ_LINKED: {
        object_link_info *li = &op->mtype.obj_link_info;
        gs_ReadInt(fp, li->parent_handle);
        gs_ReadInt(fp, li->sobj_index);
        gs_ReadVector(fp, li->fvec);
        gs_ReadVector(fp, li->uvec);
        gs_ReadVector(fp, li->pos);
        break;
      }
      default: {
        // Unknown movement type -- skip and zero out
        Int3();
        memset(&op->mtype, 0, sizeof(op->mtype));
        break;
      }
      }
    }

    VERIFY_SAVEFILE;
    // Control info, determined by CONTROL_TYPE
    {
      uint8_t ct_type;
      gs_ReadByte(fp, ct_type);
      op->control_type = ct_type;
      switch (ct_type) {
      case CT_WEAPON:
      case CT_AI: {
        laser_info_s *li = &op->ctype.laser_info;
        gs_ReadShort(fp, li->parent_type);
        gs_ReadShort(fp, li->src_gun_num);
        gs_ReadInt(fp, li->last_hit_handle);
        gs_ReadInt(fp, li->track_handle);
        gs_ReadFloat(fp, li->last_track_time);
        gs_ReadInt(fp, li->hit_status);
        gs_ReadVector(fp, li->hit_pnt);
        gs_ReadVector(fp, li->hit_wall_pnt);
        gs_ReadVector(fp, li->hit_wall_normal);
        gs_ReadInt(fp, li->hit_room);
        gs_ReadInt(fp, li->hit_pnt_room);
        gs_ReadShort(fp, li->hit_face);
        gs_ReadFloat(fp, li->multiplier);
        gs_ReadFloat(fp, li->thrust_left);
        gs_ReadFloat(fp, li->last_drop_time);
        gs_ReadVector(fp, li->last_smoke_pos);
        gs_ReadByte(fp, li->casts_light);
        break;
      }
      case CT_EXPLOSION: {
        blast_info_s *bi = &op->ctype.blast_info;
        gs_ReadFloat(fp, bi->max_size);
        gs_ReadInt(fp, bi->bm_handle);
        break;
      }
      case CT_DEBRIS: {
        dying_info_s *di = &op->ctype.dying_info;
        gs_ReadInt(fp, di->death_flags);
        gs_ReadFloat(fp, di->delay_time);
        gs_ReadInt(fp, di->killer_playernum);
        gs_ReadFloat(fp, di->last_spark_time);
        gs_ReadFloat(fp, di->last_fireball_time);
        gs_ReadFloat(fp, di->last_smoke_time);
        break;
      }
      case CT_POWERUP: {
        powerup_info_s *pi = &op->ctype.powerup_info;
        gs_ReadInt(fp, pi->count);
        break;
      }
      case CT_SPLINTER: {
        splinter_info_s *si = &op->ctype.splinter_info;
        gs_ReadByte(fp, si->subobj_num);
        gs_ReadShort(fp, si->facenum);
        for (int v = 0; v < MAX_VERTS_PER_SPLINTER; v++) {
          gs_ReadVector(fp, si->verts[v]);
        }
        gs_ReadVector(fp, si->center);
        break;
      }
      case CT_SOUNDSOURCE: {
        soundsource_info_s *ssi = &op->ctype.soundsource_info;
        gs_ReadInt(fp, ssi->sound_index);
        gs_ReadFloat(fp, ssi->volume);
        break;
      }
      default: {
        // Unknown control type -- zero out
        Int3();
        memset(&op->ctype, 0, sizeof(op->ctype));
        break;
      }
      }
    }

    VERIFY_SAVEFILE;
    // remap bitmap handle if this is a fireball!
    if (type == OBJ_FIREBALL) {
      index = op->ctype.blast_info.bm_handle;
      op->ctype.blast_info.bm_handle = (index > -1) ? gs_Xlates->bm_handles[index] : -1;
    }

    // save ai information.
    retval = LGSObjAI(fp, &op->ai_info);
    if (retval != LGS_OK)
      goto lgsobjs_fail;

    VERIFY_SAVEFILE;

    // save out rendering information
    // BUGFIX #279: Read rtype fields individually. The polyobj_info struct
    // contains multi_turret which has pointer fields (keyframes,
    // last_keyframes) that differ in size between x86 and x64. These pointers
    // are rebuilt on load, so we only read the non-pointer fields.
    {
      uint8_t rt_type;
      gs_ReadByte(fp, rt_type);
      op->render_type = rt_type;
      memset(&op->rtype, 0, sizeof(op->rtype));
      switch (rt_type) {
      case RT_POLYOBJ: {
        polyobj_info *pi = &op->rtype.pobj_info;
        gs_ReadShort(fp, pi->model_num);
        gs_ReadShort(fp, pi->dying_model_num);
        gs_ReadFloat(fp, pi->anim_start_frame);
        gs_ReadFloat(fp, pi->anim_frame);
        gs_ReadFloat(fp, pi->anim_end_frame);
        gs_ReadFloat(fp, pi->anim_time);
        gs_ReadInt(fp, pi->anim_flags);
        gs_ReadFloat(fp, pi->max_speed);
        gs_ReadInt(fp, pi->subobj_flags);
        gs_ReadInt(fp, pi->tmap_override);
        // multi_turret_info: only read non-pointer fields
        gs_ReadFloat(fp, pi->multi_turret_info.time);
        gs_ReadFloat(fp, pi->multi_turret_info.last_time);
        gs_ReadByte(fp, pi->multi_turret_info.num_turrets);
        gs_ReadByte(fp, pi->multi_turret_info.flags);
        break;
      }
      case RT_SHARD: {
        shard_info_s *si = &op->rtype.shard_info;
        for (int v = 0; v < 3; v++) {
          gs_ReadVector(fp, si->points[v]);
        }
        for (int v = 0; v < 3; v++) {
          gs_ReadFloat(fp, si->u[v]);
          gs_ReadFloat(fp, si->v[v]);
        }
        gs_ReadVector(fp, si->normal);
        gs_ReadShort(fp, si->tmap);
        break;
      }
      case RT_EDITOR_SPHERE: {
        gs_ReadInt(fp, op->rtype.sphere_color);
        break;
      }
      default:
        break;
      }
    }

    op->size = cf_ReadFloat(fp);

    // page in correct graphical data now.
    switch (op->render_type) {
    case RT_NONE:
    case RT_EDITOR_SPHERE:
    case RT_FIREBALL:
    case RT_LINE:
    case RT_PARTICLE:
    case RT_SPLINTER:
    case RT_ROOM:
      break;
    case RT_WEAPON:
      if (!(op->flags & OF_POLYGON_OBJECT)) {
        if (!(Weapons[op->id].flags & WF_IMAGE_BITMAP)) {
          PageInVClip(Weapons[op->id].fire_image_handle);
        }
        break;
      }
      break;
    case RT_POLYOBJ: // be sure to use translated handles for polyobjs and textures
    {
      // the paging mess.  must update size of object accordingly.
      sindex = (int16_t)op->rtype.pobj_info.model_num;
      new_model = (sindex > -1) ? gs_Xlates->model_handles[sindex] : -1;
      if ((new_model != op->rtype.pobj_info.model_num) || (Poly_models[new_model].flags & PMF_NOT_RESIDENT)) {
        switch (type) {
        case OBJ_DOOR:
          PageInPolymodel(new_model);
          ComputeDefaultSize(OBJ_DOOR, new_model, &op->size);
          break;
        case OBJ_ROBOT:
        case OBJ_POWERUP:
        case OBJ_BUILDING:
        case OBJ_CLUTTER:
          PageInPolymodel(new_model, type, &obji->size);
          op->size = obji->size;
          break;
        case OBJ_WEAPON:
          PageInPolymodel(new_model, OBJ_WEAPON, &wpn->size);
          op->size = wpn->size;
          break;
        case OBJ_PLAYER:
          PageInPolymodel(new_model, OBJ_PLAYER, &shp->size);
          op->size = shp->size;
          break;
        case OBJ_ROOM:
          op->size = ComputeRoomBoundingSphere(&pos, &Rooms[op->id]);
          break;

        default:
          PageInPolymodel(new_model);
        }
      }
      op->rtype.pobj_info.model_num = new_model;

      sindex = (int16_t)op->rtype.pobj_info.dying_model_num;
      new_model = (sindex > -1) ? gs_Xlates->model_handles[sindex] : -1;
      if (new_model != op->rtype.pobj_info.dying_model_num) {
        switch (type) {
        case OBJ_PLAYER:
          PageInPolymodel(new_model, OBJ_PLAYER, &shp->size);
          op->size = shp->size;
          break;
        }
      }
      op->rtype.pobj_info.dying_model_num = new_model;

      index = op->rtype.pobj_info.tmap_override;
      op->rtype.pobj_info.tmap_override = (index > -1) ? gs_Xlates->tex_handles[index] : -1;

      memset(&op->rtype.pobj_info.multi_turret_info, 0, sizeof(multi_turret));
      poly_model *pm = &Poly_models[op->rtype.pobj_info.model_num];

      if (pm->n_attach) {
        LOG_DEBUG.printf("*Object %d has %d attach points.", i, pm->n_attach);
      }

      polyobj_info *p_info = &op->rtype.pobj_info;
      int num_wbs = pm->num_wbs;
      int count = 0;
      for (int j = 0; j < num_wbs; j++) {
        ASSERT(pm->poly_wb[j].num_turrets >= 0 && pm->poly_wb[j].num_turrets <= 6400);
        count += pm->poly_wb[j].num_turrets;
      }

      p_info->multi_turret_info.num_turrets = count;

      if ((count > 0) && (p_info->multi_turret_info.keyframes == NULL)) {
        p_info->multi_turret_info.time = 0;
        p_info->multi_turret_info.keyframes = mem_rmalloc<float>(count);
        p_info->multi_turret_info.last_keyframes = mem_rmalloc<float>(count);
        p_info->multi_turret_info.flags = 0;
      }
      // Do Animation stuff
      // BUGFIX #279: Read custom_anim fields individually for portability
      custom_anim multi_anim_info;
      gs_ReadFloat(fp, multi_anim_info.server_time);
      gs_ReadShort(fp, multi_anim_info.server_anim_frame);
      gs_ReadShort(fp, multi_anim_info.anim_start_frame);
      gs_ReadShort(fp, multi_anim_info.anim_end_frame);
      gs_ReadFloat(fp, multi_anim_info.anim_time);
      gs_ReadFloat(fp, multi_anim_info.max_speed);
      gs_ReadShort(fp, multi_anim_info.anim_sound_index);
      gs_ReadByte(fp, multi_anim_info.flags);
      gs_ReadByte(fp, multi_anim_info.next_anim_type);
      ObjSetAnimUpdate(i, &multi_anim_info);
      break;
    }

    case RT_SHARD:
      sindex = (int16_t)op->rtype.shard_info.tmap;
      op->rtype.shard_info.tmap = (sindex > -1) ? gs_Xlates->tex_handles[sindex] : -1;
      break;

    default:
      Int3();
    }

    VERIFY_SAVEFILE;

    //!! dynamic weapon battery info!!
    retval = LGSObjWB(fp, op);
    if (retval != LGS_OK)
      goto lgsobjs_fail;

    VERIFY_SAVEFILE;

    // save effect info!
    retval = LGSObjEffects(fp, op);
    if (retval != LGS_OK)
      goto lgsobjs_fail;

    VERIFY_SAVEFILE;

    // save script stuff.
    if (Objects[i].lighting_render_type != LRT_LIGHTMAPS) {
      // Don't init the scripts for an object with lightmaps, because it was never destroyed!
      InitObjectScripts(op);
    }

    //	special case saves
    retval = LGSObjSpecial(fp, op);
    if (retval != LGS_OK)
      goto lgsobjs_fail;

    // link into mine.
    // turn off big object flags if it is a big object. (ObjLink will take care of this.)
    if (op->flags & OF_BIG_OBJECT)
      op->flags &= (~OF_BIG_OBJECT);

    op->roomnum = roomnum; //-1;
                           // ObjLink(OBJNUM(op), roomnum);
    // ObjSetOrient(op, &objmat);
  }
  for (; i < MAX_OBJECTS; i++) {
    if (Objects[i].type != OBJ_NONE) {
      Osiris_DisableEvents(OEM_OBJECTS | OEM_TRIGGERS | OEM_LEVELS);
      ObjDelete(i);
      Osiris_EnableEvents(OEM_OBJECTS | OEM_TRIGGERS | OEM_LEVELS);
    }
  }

  for (i = 0; i < MAX_OBJECTS; i++) {
    Objects[i].next = -1;
    Objects[i].prev = -1;
  }
  for (i = 0; i < MAX_ROOMS; i++) {
    Rooms[i].objects = -1;
  }
  max_terr = TERRAIN_WIDTH * TERRAIN_DEPTH;
  for (i = 0; i < max_terr; i++) {
    Terrain_seg[i].objects = -1;
  }
  inreadobj = 0;
  Highest_object_index = highest_index;

  // CreateRoomObjects();

  for (i = 0; i <= Highest_object_index; i++) {
    object *op = &Objects[i];
    if (Objects[i].type != OBJ_NONE) {
      int newroom = op->roomnum;
      op->roomnum = -1;
      ObjLink(OBJNUM(op), newroom);
      ObjSetOrient(op, &objmat[i]);
      if (op->type == OBJ_ROOM) {
        LOG_DEBUG.printf("Object %d is a room and Is%s a big object. Size=%f",
                         i, (op->flags & OF_BIG_OBJECT) ? "" : "n't", op->size);
        if ((op->size >= ((TERRAIN_SIZE * (float)1))) && !(op->flags & OF_BIG_OBJECT)) {
          BigObjAdd(i);
        }

        ObjSetAABB(op);
      }
    }
/*
    if ((op->attach_ultimate_handle) && (OBJECT_HANDLE_NONE != op->attach_ultimate_handle)) {
      mprintf(0, "Object %d has an ultimate parent of %d (%d)\n",
              i,
              OBJNUM(ObjGet(op->attach_ultimate_handle)),
              op->attach_parent_handle);
      ASSERT(op->flags & OF_ATTACHED);
    }
    if ((op->attach_ultimate_handle) && (OBJECT_HANDLE_NONE != op->attach_parent_handle)) {
      mprintf(0, "Object %d has a parent of %d (%d)\n",
              i,
              OBJNUM(ObjGet(op->attach_parent_handle)),
              op->attach_parent_handle);
      ASSERT(op->flags & OF_ATTACHED);
    }
    */
  }
  LOG_DEBUG.printf("Objects[121].prev=%d", Objects[121].prev);
  ResetFreeObjects();
  LOG_DEBUG.printf("highest obj index = %d, ", Highest_object_index);
  ObjReInitPositionHistory();

  END_VERIFY_SAVEFILE(fp, "Objects load");
lgsobjs_fail:

  Osiris_EnableCreateEvents();

done:;

  mem_free(objmat);

  return retval;
}

//	loads ai
// BUGFIX #279: Portable ai_frame deserialization.
// Read fields individually instead of raw struct dump for cross-architecture
// compatibility.
int LGSObjAI(CFILE *fp, ai_frame **pai) {
  int16_t size;
  int8_t read_ai;

  *pai = NULL;

  gs_ReadByte(fp, read_ai);
  if (!read_ai)
    return LGS_OK;

  gs_ReadShort(fp, size);
  if (size != sizeof(ai_frame))
    return LGS_OUTDATEDVER;

  auto ai = *pai = mem_rmalloc<ai_frame>();
  memset(ai, 0, sizeof(ai_frame));

  gs_ReadByte(fp, ai->ai_class);
  gs_ReadByte(fp, ai->ai_type);

  // ai_path_info
  gs_ReadShort(fp, ai->path.cur_path);
  gs_ReadShort(fp, ai->path.cur_node);
  gs_ReadShort(fp, ai->path.num_paths);
  gs_ReadInt(fp, ai->path.goal_uid);
  gs_ReadInt(fp, ai->path.goal_index);
  for (int p = 0; p < MAX_JOINED_PATHS; p++) {
    gs_ReadByte(fp, ai->path.path_id[p]);
    gs_ReadByte(fp, ai->path.path_type[p]);
    gs_ReadShort(fp, ai->path.path_start_node[p]);
    gs_ReadShort(fp, ai->path.path_end_node[p]);
    gs_ReadShort(fp, ai->path.path_flags[p]);
  }

  gs_ReadFloat(fp, ai->max_velocity);
  gs_ReadFloat(fp, ai->max_delta_velocity);
  gs_ReadFloat(fp, ai->max_turn_rate);
  gs_ReadFloat(fp, ai->max_delta_turn_rate);

  gs_ReadFloat(fp, ai->attack_vel_percent);
  gs_ReadFloat(fp, ai->flee_vel_percent);
  gs_ReadFloat(fp, ai->dodge_vel_percent);

  gs_ReadFloat(fp, ai->circle_distance);
  gs_ReadFloat(fp, ai->dodge_percent);

  for (int m = 0; m < 2; m++) {
    gs_ReadFloat(fp, ai->melee_damage[m]);
    gs_ReadFloat(fp, ai->melee_latency[m]);
  }

  for (int s = 0; s < MAX_AI_SOUNDS; s++) {
    gs_ReadInt(fp, ai->sound[s]);
    gs_ReadFloat(fp, ai->last_sound_time[s]);
  }
  gs_ReadShort(fp, ai->last_played_sound_index);

  gs_ReadByte(fp, ai->movement_type);
  gs_ReadByte(fp, ai->movement_subtype);

  gs_ReadByte(fp, ai->animation_type);
  gs_ReadByte(fp, ai->next_animation_type);

  gs_ReadByte(fp, ai->next_movement);
  gs_ReadByte(fp, ai->current_wb_firing);
  gs_ReadByte(fp, ai->last_special_wb_firing);

  // goals
  for (int g = 0; g < MAX_GOALS; g++) {
    goal *gl = &ai->goals[g];
    uint8_t used;
    gs_ReadByte(fp, used);
    gl->used = (used != 0);
    if (!gl->used)
      continue;

    gs_ReadInt(fp, gl->type);
    gs_ReadByte(fp, gl->subtype);
    gs_ReadByte(fp, gl->activation_level);
    gs_ReadFloat(fp, gl->creation_time);
    gs_ReadFloat(fp, gl->min_influence);
    gs_ReadFloat(fp, gl->influence);
    for (int r = 0; r < 4; r++) {
      gs_ReadFloat(fp, gl->ramp_influence_dists[r]);
    }

    // goal_info
    gs_ReadInt(fp, gl->g_info.handle);
    gs_ReadVector(fp, gl->g_info.pos);

    // goal enablers
    gs_ReadByte(fp, gl->num_enablers);
    for (int e = 0; e < gl->num_enablers; e++) {
      gs_ReadByte(fp, gl->enabler[e].enabler_type);
      gs_ReadInt(fp, gl->enabler[e].flags);
      gs_ReadFloat(fp, gl->enabler[e].percent_enable);
      gs_ReadFloat(fp, gl->enabler[e].check_interval);
      gs_ReadFloat(fp, gl->enabler[e].last_check_time);
      gs_ReadByte(fp, gl->enabler[e].bool_next_enabler_op);
    }

    gs_ReadFloat(fp, gl->circle_distance);
    gs_ReadInt(fp, gl->status_reg);
    gs_ReadFloat(fp, gl->start_time);
    gs_ReadFloat(fp, gl->next_path_time);
    gs_ReadFloat(fp, gl->dist_to_goal);
    gs_ReadVector(fp, gl->vec_to_target);
    gs_ReadFloat(fp, gl->next_check_see_target_time);
    gs_ReadVector(fp, gl->last_see_target_pos);
    gs_ReadFloat(fp, gl->last_see_target_time);
    gs_ReadFloat(fp, gl->next_target_update_time);
    gs_ReadInt(fp, gl->flags);
    gs_ReadInt(fp, gl->guid);
    gs_ReadInt(fp, gl->goal_uid);
    gs_ReadVector(fp, gl->set_fvec);
    gs_ReadVector(fp, gl->set_uvec);
  }

  gs_ReadInt(fp, ai->target_handle);
  gs_ReadFloat(fp, ai->next_target_update_time);

  gs_ReadFloat(fp, ai->dist_to_target_actual);
  gs_ReadFloat(fp, ai->dist_to_target_perceived);
  gs_ReadVector(fp, ai->vec_to_target_actual);
  gs_ReadVector(fp, ai->vec_to_target_perceived);

  gs_ReadFloat(fp, ai->next_check_see_target_time);
  gs_ReadVector(fp, ai->last_see_target_pos);
  gs_ReadFloat(fp, ai->last_see_target_time);
  gs_ReadFloat(fp, ai->last_hear_target_time);

  gs_ReadFloat(fp, ai->weapon_speed);

  gs_ReadFloat(fp, ai->next_melee_time);
  gs_ReadFloat(fp, ai->last_render_time);
  gs_ReadFloat(fp, ai->next_flinch_time);

  gs_ReadInt(fp, ai->status_reg);
  gs_ReadInt(fp, ai->flags);
  gs_ReadInt(fp, ai->notify_flags);

  gs_ReadVector(fp, ai->movement_dir);
  gs_ReadVector(fp, ai->rot_thrust_vector);

  gs_ReadFloat(fp, ai->fov);

  gs_ReadInt(fp, ai->anim_sound_handle);

  gs_ReadFloat(fp, ai->avoid_friends_distance);

  gs_ReadFloat(fp, ai->frustration);
  gs_ReadFloat(fp, ai->curiousity);
  gs_ReadFloat(fp, ai->life_preservation);
  gs_ReadFloat(fp, ai->aggression);

  gs_ReadFloat(fp, ai->cur_frustration);
  gs_ReadFloat(fp, ai->cur_curiousity);
  gs_ReadFloat(fp, ai->cur_life_preservation);
  gs_ReadFloat(fp, ai->cur_aggression);

  gs_ReadFloat(fp, ai->mem_time_till_next_update);
  for (int mem = 0; mem < AI_MEM_DEPTH; mem++) {
    gs_ReadFloat(fp, ai->memory[mem].shields);
    gs_ReadShort(fp, ai->memory[mem].num_enemies);
    gs_ReadShort(fp, ai->memory[mem].num_friends);
    gs_ReadShort(fp, ai->memory[mem].num_times_hit);
    gs_ReadShort(fp, ai->memory[mem].num_enemy_shots_fired);
    gs_ReadShort(fp, ai->memory[mem].num_hit_enemy);
    gs_ReadShort(fp, ai->memory[mem].num_enemy_shots_dodged);
  }

  gs_ReadFloat(fp, ai->fire_spread);
  gs_ReadFloat(fp, ai->night_vision);
  gs_ReadFloat(fp, ai->fog_vision);
  gs_ReadFloat(fp, ai->lead_accuracy);
  gs_ReadFloat(fp, ai->lead_varience);
  gs_ReadFloat(fp, ai->fight_team);
  gs_ReadFloat(fp, ai->fight_same);
  gs_ReadFloat(fp, ai->hearing);
  gs_ReadFloat(fp, ai->roaming);
  gs_ReadFloat(fp, ai->leadership);
  gs_ReadFloat(fp, ai->coop_same);
  gs_ReadFloat(fp, ai->coop_team);

  gs_ReadFloat(fp, ai->biased_flight_importance);
  gs_ReadFloat(fp, ai->biased_flight_min);
  gs_ReadFloat(fp, ai->biased_flight_max);

  gs_ReadVector(fp, ai->last_dodge_dir);
  gs_ReadFloat(fp, ai->dodge_till_time);

  gs_ReadFloat(fp, ai->awareness);

  gs_ReadMatrix(fp, ai->saved_orient);

  return LGS_OK;
}

//	loads fx
// BUGFIX #279: Portable effect_info_s deserialization.
// Read fields individually instead of raw struct dump for cross-architecture
// compatibility.
int LGSObjEffects(CFILE *fp, object *op) {
  int16_t size;
  int8_t do_read;

  op->effect_info = NULL;

  gs_ReadByte(fp, do_read);
  if (do_read) {
    gs_ReadShort(fp, size);
    if (size != sizeof(effect_info_s))
      return LGS_OUTDATEDVER;

    auto ei = op->effect_info = mem_rmalloc<effect_info_s>();
    memset(ei, 0, sizeof(effect_info_s));

    gs_ReadInt(fp, ei->type_flags);
    gs_ReadFloat(fp, ei->alpha);
    gs_ReadFloat(fp, ei->deform_range);
    gs_ReadFloat(fp, ei->cloak_time);
    gs_ReadFloat(fp, ei->deform_time);
    gs_ReadFloat(fp, ei->color_time);
    gs_ReadFloat(fp, ei->r);
    gs_ReadFloat(fp, ei->g);
    gs_ReadFloat(fp, ei->b);

    gs_ReadFloat(fp, ei->fade_time);
    gs_ReadFloat(fp, ei->fade_max_time);

    gs_ReadFloat(fp, ei->damage_time);
    gs_ReadFloat(fp, ei->damage_per_second);
    gs_ReadFloat(fp, ei->last_damage_time);
    gs_ReadInt(fp, ei->damage_handle);

    gs_ReadFloat(fp, ei->volume_change_time);
    gs_ReadVector(fp, ei->volume_old_pos);
    gs_ReadInt(fp, ei->volume_old_room);

    gs_ReadFloat(fp, ei->last_object_hit_time);
    gs_ReadInt(fp, ei->last_object_hit);

    gs_ReadVector(fp, ei->spec_pos);
    gs_ReadFloat(fp, ei->spec_mag);
    gs_ReadFloat(fp, ei->spec_r);
    gs_ReadFloat(fp, ei->spec_g);
    gs_ReadFloat(fp, ei->spec_b);

    gs_ReadByte(fp, ei->dynamic_this_frame);
    gs_ReadFloat(fp, ei->dynamic_red);
    gs_ReadFloat(fp, ei->dynamic_green);
    gs_ReadFloat(fp, ei->dynamic_blue);

    gs_ReadFloat(fp, ei->liquid_time_left);
    gs_ReadByte(fp, ei->liquid_mag);

    gs_ReadFloat(fp, ei->freeze_scalar);

    gs_ReadInt(fp, ei->attach_line_handle);

    gs_ReadInt(fp, ei->sound_handle);

    gs_ReadFloat(fp, ei->spark_delay);
    gs_ReadFloat(fp, ei->spark_timer);
    gs_ReadFloat(fp, ei->spark_time_left);
  }

  return LGS_OK;
}

//@@//	loads script
//@@vector *LGSScript(CFILE *fp, script_info *script, uint8_t *is_scripted, int *memsize)
//@@{
//@@	int i;
//@@	char name[64];
//@@	uint16_t nparms;
//@@	tScriptParm *parms = NULL;
//@@
//@@//	ugh, write out script info.
//@@	gs_ReadByte(fp, (*is_scripted));
//@@
//@@	cf_ReadString(name, sizeof(name), fp);
//@@	if (name[0])
//@@		script->set_name(name);
//@@
//@@	gs_ReadShort(fp, nparms);
//@@	gs_ReadShort(fp, script->is_custom);
//@@	if (nparms)
//@@		parms = new tScriptParm[nparms];
//@@
//@@	for (i = 0; i < nparms; i++)
//@@	{
//@@		gs_ReadByte(fp, parms[i].type);
//@@		gs_ReadFloat(fp, parms[i].val.x);
//@@		gs_ReadFloat(fp, parms[i].val.y);
//@@		gs_ReadFloat(fp, parms[i].val.z);
//@@	}
//@@
//@@	script->set_parms(nparms, parms);
//@@	if (parms)
//@@		delete[] parms;
//@@
//@@// write out thread data if necessary
//@@	vector *mem = NULL;
//@@	*memsize = 0;
//@@	if (*is_scripted) {
//@@		int mem_size=0,j;
//@@
//@@		gs_ReadShort(fp, mem_size);
//@@		*memsize = mem_size;
//@@
//@@		if (mem_size) {
//@@			mem = new vector[mem_size];
//@@			for (j = 0; j < mem_size; j++)
//@@				gs_ReadVector(fp, mem[j]);
//@@		}
//@@	}
//@@
//@@	return mem;
//@@}

//	loads fx
// BUGFIX #279: Portable dynamic_wb_info deserialization.
// Read fields individually instead of raw struct dump for cross-architecture
// compatibility.
int LGSObjWB(CFILE *fp, object *op) {
  dynamic_wb_info *dwba = NULL;
  int i;
  int8_t num_wbs;

  gs_ReadByte(fp, num_wbs);
  if (!num_wbs)
    return LGS_OK;

  dwba = mem_rmalloc<dynamic_wb_info>(num_wbs);

  for (i = 0; i < num_wbs; i++) {
    dynamic_wb_info *dwb = &dwba[i];
    gs_ReadFloat(fp, dwb->last_fire_time);
    gs_ReadByte(fp, dwb->cur_firing_mask);
    for (int t = 0; t < MAX_WB_TURRETS; t++) {
      gs_ReadFloat(fp, dwb->norm_turret_angle[t]);
      gs_ReadFloat(fp, dwb->turret_next_think_time[t]);
      gs_ReadByte(fp, dwb->turret_direction[t]);
    }
    gs_ReadByte(fp, dwb->wb_anim_mask);
    gs_ReadFloat(fp, dwb->wb_anim_frame);
    gs_ReadVector(fp, dwb->cur_target);
    gs_ReadByte(fp, dwb->upgrade_level);
    gs_ReadInt(fp, dwb->flags);
  }
  op->dynamic_wb = dwba;

  return LGS_OK;
}

// loads special object info
int LGSObjSpecial(CFILE *fp, object *op) {
  int retval = LGS_OK;

  return retval;
}

// load spew
// BUGFIX #279: Portable spewinfo deserialization.
// Read fields individually instead of raw struct dump for cross-architecture
// compatibility.
int LGSSpew(CFILE *fp) {
  int i;

  // read GLOBAL value
  gs_ReadShort(fp, spew_count);

  for (i = 0; i < MAX_SPEW_EFFECTS; i++) {
    uint8_t used;
    gs_ReadByte(fp, used);
    if (used) {
      spewinfo *se = &SpewEffects[i];
      memset(se, 0, sizeof(spewinfo));
      se->inuse = true;
      gs_ReadByte(fp, se->flags);
      gs_ReadByte(fp, se->use_gunpoint);
      gs_ReadByte(fp, se->real_obj);
      if (se->use_gunpoint) {
        gs_ReadInt(fp, se->gp.obj_handle);
        gs_ReadInt(fp, se->gp.gunpoint);
      } else {
        gs_ReadVector(fp, se->pt.origin);
        gs_ReadVector(fp, se->pt.normal);
        gs_ReadInt(fp, se->pt.room_num);
      }
      gs_ReadInt(fp, se->effect_type);
      gs_ReadInt(fp, se->phys_info);
      gs_ReadInt(fp, se->random);
      gs_ReadInt(fp, se->handle);
      gs_ReadFloat(fp, se->drag);
      gs_ReadFloat(fp, se->mass);
      gs_ReadFloat(fp, se->time_int);
      gs_ReadFloat(fp, se->longevity);
      gs_ReadFloat(fp, se->lifetime);
      gs_ReadFloat(fp, se->size);
      gs_ReadFloat(fp, se->speed);
      gs_ReadFloat(fp, se->time_until_next_blob);
      gs_ReadFloat(fp, se->start_time);
      gs_ReadVector(fp, se->gp_normal);
      gs_ReadVector(fp, se->gun_point);
    }
  }

  return LGS_OK;
}

int LGSMatcens(CFILE *fp) {
  int num_matcens = cf_ReadInt(fp);

  for (int i = 0; i < num_matcens; i++) {
    Matcen[i]->LoadData(fp);
  }

  return LGS_OK;
}

int LGSSnapshot(CFILE *fp) {
  int bm_handle = -1;
  int8_t valid_snapshot = 0;

  // get snapshot byte
  valid_snapshot = cf_ReadByte(fp);

  //	if valid, read it in, otherwise just return
  if (valid_snapshot) {
    bm_handle = bm_AllocLoadBitmap(fp, 0);
  }

  return bm_handle;
}

//@@	// movement info.
//@@		switch (op->movement_type)
//@@		{
//@@		case MT_SHOCKWAVE:
//@@			{
//@@				int16_t bound;
//@@				gs_ReadShort(fp, bound);
//@@				if (bound < (sizeof(op_wave->damaged_list)/sizeof(uint32_t))) {
//@@					Int3();
//@@					retval = LGS_OUTDATEDVER;
//@@					goto lgsobjs_fail;
//@@				}
//@@				for (j = 0; j < sizeof(op_wave->damaged_list)/sizeof(uint32_t); j++)
//@@					gs_ReadInt(fp, (uint32_t)op_wave->damaged_list[j]);
//@@			}
//@@			break;
//@@		default:
//@@			gs_ReadVector(fp, op_phys->velocity);
//@@			gs_ReadVector(fp, op_phys->thrust);
//@@			gs_ReadVector(fp, op_phys->rotvel);
//@@			gs_ReadVector(fp, op_phys->rotthrust);
//@@			gs_ReadAngle(fp, op_phys->turnroll);
//@@			gs_ReadFloat(fp, op_phys->last_still_time);
//@@			gs_ReadInt(fp, op_phys->num_bounces);
//@@			gs_ReadFloat(fp, op_phys->coeff_restitution);
//@@			gs_ReadFloat(fp, op_phys->mass);
//@@			gs_ReadFloat(fp, op_phys->drag);
//@@			gs_ReadFloat(fp, op_phys->rotdrag);
//@@			gs_ReadFloat(fp, op_phys->full_thrust);
//@@			gs_ReadFloat(fp, op_phys->full_rotthrust);
//@@			gs_ReadFloat(fp, op_phys->max_turnroll_rate);
//@@			gs_ReadFloat(fp, op_phys->turnroll_ratio);
//@@			gs_ReadFloat(fp, op_phys->wiggle_amplitude);
//@@			gs_ReadFloat(fp, op_phys->wiggles_per_sec);
//@@			gs_ReadVector(fp, op_phys->dest_pos);
//@@			gs_ReadFloat(fp, op_phys->hit_die_dot);
//@@			gs_ReadFloat(fp, op_phys->max_speed_time);
//@@			gs_ReadInt(fp, op_phys->flags);
//@@		}
//@@
//@@	// don't need to load lightmap infomation.  should be in object already at loadtime
//@@
//@@	//Control info, determined by CONTROL_TYPE
//@@		switch (op->control_type)
//@@		{
//@@		case CT_NONE:
//@@		case CT_AI:
//@@		case CT_FLYING:			//the player is flying
//@@		case CT_FLYTHROUGH:		//the flythrough system
//@@		case CT_SLEW:				//slewing
//@@		case CT_PARTICLE:			//Particle
//@@			break;
//@@		case CT_EXPLOSION:		//explosion sequencer
//@@			gs_ReadFloat(fp, op_expl->spawn_time);
//@@			gs_ReadFloat(fp,	op_expl->delete_time);
//@@			gs_ReadShort(fp, op_expl->delete_objnum);
//@@			gs_ReadShort(fp, op_expl->attach_parent);
//@@			gs_ReadShort(fp, op_expl->prev_attach);
//@@			gs_ReadShort(fp, op_expl->next_attach);
//@@			break;
//@@		case CT_DEBRIS:			//this is a piece of debris
//@@			gs_ReadFloat(fp, op->ctype.dying_info.delay_time);
//@@			gs_ReadBool(fp, op->ctype.dying_info.f_death_anim);
//@@			break;
//@@		case CT_POWERUP:			//animating powerup blob
//@@			gs_ReadInt(fp, op->ctype.powerup_info.count);
//@@			gs_ReadInt(fp, op->ctype.powerup_info.flags);
//@@			break;
//@@		case CT_SPLINTER:			//Splinter
//@@			gs_ReadByte(fp, op_splint->subobj_num);
//@@			gs_ReadShort(fp, op_splint->facenum);
//@@			for (j = 0; j < MAX_VERTS_PER_SPLINTER; j++)
//@@				gs_ReadVector(fp, op_splint->verts[j]);
//@@			gs_ReadVector(fp, op_splint->center);
//@@			break;
//@@		case CT_WEAPON:			//laser, etc.
//@@			gs_ReadShort(fp, op_wpn->parent_type);
//@@			gs_ReadShort(fp, op_wpn->src_gun_num);
//@@			gs_ReadInt(fp, op_wpn->last_hit_handle);
//@@			gs_ReadInt(fp, op_wpn->track_handle);
//@@			gs_ReadInt(fp, op_wpn->hit_status);
//@@			gs_ReadVector(fp, op_wpn->hit_pnt);
//@@			gs_ReadVector(fp, op_wpn->hit_wall_pnt);
//@@			gs_ReadVector(fp, op_wpn->hit_wall_normal);
//@@			gs_ReadInt(fp, op_wpn->hit_room);
//@@			gs_ReadInt(fp, op_wpn->hit_pnt_room);
//@@			gs_ReadShort(fp, op_wpn->hit_face);
//@@			gs_ReadFloat(fp, op_wpn->multiplier);
//@@			gs_ReadFloat(fp, op_wpn->thrust_left);
//@@			gs_ReadFloat(fp, op_wpn->last_drop_time);
//@@			break;
//@@
//@@		default:
//@@			Int3();
//@@		}
//@@
//@@		if (op->type == OBJ_FIREBALL) {
//@@			gs_ReadFloat(fp,op->ctype.blast_info.max_size);
//@@			gs_ReadInt(fp,index);
//@@			op->ctype.blast_info.bm_handle = (index > -1) ? gs_Xlates->bm_handles[index] : -1;
//@@		}

// RENDER
//@@		const int N_SHARD_VERTS = 3;
//@@		switch (op->render_type)
//@@		{
//@@		case RT_NONE:
//@@		case RT_EDITOR_SPHERE:
//@@		case RT_FIREBALL:
//@@		case RT_LINE:
//@@		case RT_PARTICLE:
//@@		case RT_SPLINTER:
//@@		case RT_ROOM:
//@@			break;
//@@		case RT_WEAPON:
//@@			if (!(op->flags & OF_POLYGON_OBJECT)) {
//@@				PageInVClip (Weapons[op->id].fire_image_handle);
//@@				break;
//@@			}
//@@		case RT_POLYOBJ:					// be sure to use translated handles for
// polyobjs and textures
//@@			gs_ReadShort(fp, sindex);
//@@			new_model = (sindex > -1) ? gs_Xlates->model_handles[sindex] : -1;
//@@			if (new_model != pobji->model_num)
//@@				PageInPolymodel (new_model, op->type, &op->size);
//@@			pobji->model_num = new_model;
//@@
//@@			gs_ReadShort(fp, sindex);
//@@			new_model = (sindex > -1) ? gs_Xlates->model_handles[sindex] : -1;
//@@			if (new_model != pobji->dying_model_num)
//@@				PageInPolymodel (new_model, op->type, &op->size);
//@@			pobji->dying_model_num = new_model;
//@@
//@@			gs_ReadFloat(fp, pobji->anim_start_frame);
//@@			gs_ReadFloat(fp, pobji->anim_frame);
//@@			gs_ReadFloat(fp, pobji->anim_end_frame);
//@@			gs_ReadFloat(fp, pobji->anim_time);
//@@			gs_ReadInt(fp, pobji->anim_flags);
//@@			gs_ReadFloat(fp, pobji->max_speed);
//@@			gs_ReadInt(fp, pobji->subobj_flags);
//@@
//@@			gs_ReadInt(fp, index);
//@@			pobji->tmap_override = (index>-1) ? gs_Xlates->tex_handles[index] : -1;
//@@
//@@		//!!	multi_anim_save here
//@@		// !@$()*#  SAVE MULTI ANIM TURRET INFORMATION HERE $*#)(*)(!
//@@			break;
//@@
//@@		case RT_SHARD:
//@@			for (j = 0; j < N_SHARD_VERTS; j++)
//@@				gs_ReadVector(fp, op->rtype.shard_info.points[j]);
//@@			for (j = 0; j < N_SHARD_VERTS; j++)
//@@				gs_ReadFloat(fp, op->rtype.shard_info.u[j]);
//@@			for (j = 0; j < N_SHARD_VERTS; j++)
//@@				gs_ReadFloat(fp, op->rtype.shard_info.v[j]);
//@@			gs_ReadVector(fp, op->rtype.shard_info.normal);
//@@			gs_ReadShort(fp, sindex);
//@@			op->rtype.shard_info.tmap = (sindex>-1) ? gs_Xlates->tex_handles[sindex] : -1;
//@@			break;
//@@
//@@		default:
//@@			Int3();
//@@		}
//@@

// AI
//@@// the FUN begins.
//@@	gs_ReadByte(fp, ai->ai_class);
//@@	gs_ReadByte(fp, ai->ai_type);
//@@
//@@// write path info
//@@	ai_path_info *path = &ai->path;
//@@	gs_ReadInt(fp, path->flags);
//@@	gs_ReadShort(fp, path->cur_path);
//@@	gs_ReadShort(fp, path->cur_node);
//@@	gs_ReadShort(fp, path->num_paths);
//@@	gs_ReadShort(fp, path->goal_index);
//@@
//@@	for (i = 0; i < MAX_JOINED_PATHS; i++)
//@@	{
//@@		gs_ReadByte(fp, path->path_id[i]);
//@@		gs_ReadByte(fp, path->path_type[i]);
//@@		gs_ReadShort(fp, path->path_start_node[i]);
//@@		gs_ReadShort(fp, path->path_end_node[i]);
//@@		gs_ReadShort(fp, path->path_flags[i]);
//@@	}
//@@
//@@// continue
//@@	gs_ReadFloat(fp, ai->max_velocity);
//@@	gs_ReadFloat(fp, ai->max_delta_velocity);
//@@	gs_ReadFloat(fp, ai->max_turn_rate);
//@@	gs_ReadFloat(fp, ai->max_delta_turn_rate);
//@@	gs_ReadFloat(fp, ai->attack_vel_percent);
//@@	gs_ReadFloat(fp, ai->flee_vel_percent);
//@@	gs_ReadFloat(fp, ai->dodge_vel_percent);
//@@	gs_ReadFloat(fp, ai->circle_distance);
//@@	gs_ReadFloat(fp, ai->dodge_percent);
//@@
//@@	for (i = 0; i < 2; i++)
//@@	{
//@@		gs_ReadFloat(fp, ai->melee_damage[i]);
//@@		gs_ReadFloat(fp, ai->melee_latency[i]);
//@@	}
//@@
//@@	for (i = 0; i < MAX_AI_SOUNDS; i++)
//@@	{
//@@		gs_ReadInt(fp, ai->sound[i]);
//@@		gs_ReadFloat(fp, ai->last_sound_time[i]);
//@@	}
//@@	gs_ReadShort(fp, ai->last_played_sound_index);
//@@
//@@	gs_ReadByte(fp, ai->movement_type);
//@@	gs_ReadByte(fp, ai->movement_subtype);
//@@	gs_ReadByte(fp, ai->animation_type);
//@@	gs_ReadByte(fp, ai->next_animation_type);
//@@	gs_ReadByte(fp, ai->next_movement);
//@@	gs_ReadByte(fp, ai->current_wb_firing);
//@@	gs_ReadByte(fp, ai->last_wb_firing);
//@@
//@@//	goals
//@@	for (i = 0;i < MAX_GOALS; i++)
//@@	{
//@@		retval = LGSObjAIGoal(fp, &ai->goals[i]);
//@@		if (retval != LGS_OK) goto lgsaigoal_fail;
//@@	}
//@@
//@@//	continue
//@@	gs_ReadInt(fp, ai->target_handle);
//@@	gs_ReadFloat(fp, ai->next_target_update_time);
//@@	gs_ReadFloat(fp, ai->dist_to_target);
//@@	gs_ReadVector(fp, ai->vec_to_target);
//@@	gs_ReadFloat(fp, ai->last_check_see_target_time);
//@@	gs_ReadVector(fp, ai->last_see_target_pos);
//@@	gs_ReadFloat(fp, ai->last_see_target_time);
//@@	gs_ReadFloat(fp, ai->weapon_speed);
//@@	gs_ReadFloat(fp, ai->next_melee_time);
//@@	gs_ReadFloat(fp, ai->last_render_time);
//@@	gs_ReadFloat(fp, ai->next_flinch_time);
//@@	gs_ReadInt(fp, ai->status_reg);
//@@	gs_ReadInt(fp, ai->flags);
//@@	gs_ReadInt(fp, ai->notify_flags);
//@@
//@@// notify events (MUST BE WRITTEN OUT AT A LATER DATE)
//@@
//@@// Normalized movement and facing information
//@@	gs_ReadVector(fp, ai->movement_dir);
//@@	gs_ReadVector(fp, ai->rot_thrust_vector);
//@@	gs_ReadFloat(fp, ai->fov);
//@@	gs_ReadInt(fp, ai->anim_sound_handle);
//@@	gs_ReadFloat(fp, ai->frustration);
//@@	gs_ReadFloat(fp, ai->curiousity);
//@@	gs_ReadFloat(fp, ai->fire_spread);
//@@	gs_ReadFloat(fp, ai->aggression);
//@@	gs_ReadFloat(fp, ai->night_vision);
//@@	gs_ReadFloat(fp, ai->fog_vision);
//@@	gs_ReadFloat(fp, ai->lead_accuracy);
//@@	gs_ReadFloat(fp, ai->lead_varience);
//@@	gs_ReadFloat(fp, ai->fight_team);
//@@	gs_ReadFloat(fp, ai->fight_same);
//@@	gs_ReadFloat(fp, ai->hearing);
//@@	gs_ReadFloat(fp, ai->roaming);
//@@	gs_ReadFloat(fp, ai->life_preservation);
//@@	gs_ReadShort(fp, ai->awareness);
//@@

// saves ai goals
//@@int LGSObjAIGoal(CFILE *fp, goal *g)
//@@{
//@@	int i;
//@@	int8_t used;
//@@
//@@	gs_ReadByte(fp, used);
//@@	g->used = used ? true : false;
//@@	if (!g->used)
//@@		return LGS_OK;
//@@
//@@	gs_ReadInt(fp, g->type);
//@@	gs_ReadByte(fp, g->activation_level);
//@@	gs_ReadShort(fp, g->influence);
//@@
//@@// write goal info
//@@	switch (g->type)
//@@	{
//@@		case AIG_HIDE_FROM_OBJ:
//@@			gs_ReadInt(fp, g->g_info.handle);
//@@			gs_ReadFloat(fp, g->g_info.time);
//@@			break;
//@@
//@@		case AIG_WANDER_AROUND:
//@@			gs_ReadInt(fp, g->g_info.roomnum);
//@@			break;
//@@
//@@		case AIG_DODGE_OBJ:
//@@		case AIG_MOVE_RELATIVE_OBJ:
//@@			gs_ReadInt(fp, g->g_info.handle);
//@@			break;
//@@
//@@		case AIG_MOVE_RELATIVE_OBJ_VEC:
//@@			gs_ReadInt(fp, g->g_info.handle);
//@@			gs_ReadInt(fp, g->g_info.subtype);
//@@			break;
//@@
//@@		case AIG_GUARD_AREA:
//@@			gs_ReadVector(fp, g->g_info.pos);
//@@			break;
//@@
//@@		case AIG_GET_TO_OBJ:
//@@			gs_ReadInt(fp, g->g_info.handle);
//@@			gs_ReadVector(fp, g->g_info.pos);
//@@			break;
//@@
//@@		case AIG_GET_TO_POS:
//@@			gs_ReadInt(fp, g->g_info.roomnum);
//@@			gs_ReadVector(fp, g->g_info.pos);
//@@			break;
//@@
//@@		case AIG_FOLLOW_PATH: // This must be fixed -- chrishack
//@@			gs_ReadInt(fp, g->g_info.id);
//@@			break;
//@@	}
//@@
//@@// write goal enabler
//@@	gs_ReadByte(fp, g->num_enablers);
//@@	for (i = 0; i < g->num_enablers; i++)
//@@	{
//@@		gs_ReadByte(fp, g->enabler[i].enabler_type);
//@@
//@@		switch (g->enabler[i].enabler_type)
//@@		{
//@@		case AIE_AI_STATUS_FLAG:
//@@			gs_ReadInt(fp, g->enabler[i].flags);
//@@			break;
//@@		}
//@@
//@@		gs_ReadFloat(fp, g->enabler[i].percent_enable);
//@@		gs_ReadFloat(fp, g->enabler[i].check_interval);
//@@		gs_ReadFloat(fp, g->enabler[i].last_check_time);
//@@		gs_ReadByte(fp, g->enabler[i].bool_next_enabler_op);
//@@	}
//@@
//@@// continue
//@@	gs_ReadFloat(fp, g->circle_distance);
//@@	gs_ReadInt(fp, g->status_reg);
//@@	gs_ReadFloat(fp, g->start_time);
//@@	gs_ReadFloat(fp, g->next_path_time);
//@@	gs_ReadFloat(fp, g->dist_to_goal);
//@@	gs_ReadVector(fp, g->vec_to_target);
//@@	gs_ReadFloat(fp, g->last_check_see_target_time);
//@@	gs_ReadVector(fp, g->last_see_target_pos);
//@@	gs_ReadFloat(fp, g->last_see_target_time);
//@@	gs_ReadFloat(fp, g->next_target_update_time);
//@@	gs_ReadShort(fp, g->flags);
//@@
//@@	return LGS_OK;
//@@}

// Object effect_info
//@@		gs_ReadInt(fp, ei->type_flags);
//@@		gs_ReadFloat(fp, ei->alpha);
//@@
//@@//		if (op->type == OBJ_POWERUP) {
//@@			gs_ReadFloat(fp, ei->last_object_hit_time);
//@@			gs_ReadInt(fp, ei->last_object_hit);
//@@//		}
//@@//		if (ei->type_flags & EF_DEFORM) {
//@@			gs_ReadFloat(fp, ei->deform_range);
//@@			gs_ReadFloat(fp, ei->deform_time);
//@@//		}
//@@//		if (ei->type_flags & EF_VOLUME_CHANGING) {
//@@			gs_ReadFloat(fp, ei->volume_change_time);
//@@			gs_ReadVector(fp, ei->volume_old_pos);
//@@			gs_ReadFloat(fp, ei->volume_old_room);
//@@//		}
//@@//		if (ei->type_flags & EF_VOLUME_LIT) {
//@@			gs_ReadByte(fp, ei->dynamic_this_frame);
//@@			gs_ReadFloat(fp, ei->dynamic_red);
//@@			gs_ReadFloat(fp, ei->dynamic_green);
//@@			gs_ReadFloat(fp, ei->dynamic_blue);
//@@//		}
//@@//		if (ei->type_flags & EF_CLOAKED) {
//@@			gs_ReadFloat(fp, ei->cloak_time);
//@@//		}
//@@//		if (ei->type_flags & EF_COLORED) {
//@@			gs_ReadFloat(fp, ei->color_time);
//@@			gs_ReadFloat(fp, ei->r);
//@@			gs_ReadFloat(fp, ei->g);
//@@			gs_ReadFloat(fp, ei->b);
//@@//		}
//@@//		if (ei->type_flags & EF_NAPALMED) {
//@@			gs_ReadFloat(fp, ei->damage_time);
//@@			gs_ReadFloat(fp, ei->damage_per_second);
//@@			gs_ReadFloat(fp, ei->last_damage_time);
//@@			gs_ReadInt(fp, ei->damage_handle);
//@@//		}
//@@//		if (ei->type_flags & EF_FREEZE) {
//@@			gs_ReadFloat(fp, ei->freeze_scalar);
//@@//		}
//@@//		if (ei->type_flags & EF_LINE_ATTACH) {
//@@			gs_ReadInt(fp, ei->attach_line_handle);
//@@//		}
//@@	}

// Object WBs
//@@		gs_ReadFloat(fp, dwb->last_fire_time);
//@@		gs_ReadByte(fp, dwb->cur_firing_mask);
//@@
//@@		for (j = 0; j < MAX_WB_TURRETS;j++)
//@@		{
//@@			gs_ReadFloat(fp, dwb->norm_turret_angle[j]);
//@@			gs_ReadFloat(fp, dwb->turret_next_think_time[j]);
//@@			gs_ReadByte(fp, dwb->turret_direction[j]);
//@@		}
//@@
//@@		gs_ReadByte(fp, dwb->wb_anim_mask);
//@@		gs_ReadFloat(fp, dwb->wb_anim_frame);
//@@		gs_ReadVector(fp, dwb->cur_target);
//@@		gs_ReadByte(fp, dwb->upgrade_level);
//@@		gs_ReadInt(fp, dwb->flags);
