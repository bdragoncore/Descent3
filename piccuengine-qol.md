# Quality of Life Improvements from PiccuEngine

This document tracks QoL improvements from
[InsanityBringer/PiccuEngine](https://github.com/InsanityBringer/PiccuEngine)
that we can pull into our fork. All credit goes to **InsanityBringer** for
the original work.

PiccuEngine is a Descent 3 fork focused on QoL improvements, licensed
under GPL-3.0 (compatible with our project).

---

## High-Priority (High Impact, Moderate Effort)

### 1. Windowed Mode Improvements
- **Centered window mode** - window opens centered on screen
- **Scaling in window and fullscreen** - content scales to fill window
- **Fullscreen does not change desktop settings** - uses borderless
  fullscreen instead of exclusive mode

**PiccuEngine commits:** `832ccd8`, `f9538b9`, `b0e2d00`
**Our status:** Implemented. Window centered on display, borderless
fullscreen option added via -borderless flag, content scaling works
via SDL3 automatic scaling.

### 2. Adjustable FOV with Widescreen Expansion
- User-adjustable field of view
- Automatic FOV expansion for widescreen monitors (prevents fish-eye)

**PiccuEngine feature:** Listed in v1.0 features
**Our status:** Not implemented. Our renderer uses fixed FOV.

### 3. Widescreen Cockpit Support
- Cockpit HUD elements properly render in widescreen resolutions
- Unlike InjectD3, the cockpit actually works correctly

**PiccuEngine feature:** Listed in v1.0 features
**Our status:** Not implemented. Cockpit may have placement issues
at non-4:3 resolutions.

### 4. MSAA Support
- Multi-Sample Anti-Aliasing support in the renderer

**PiccuEngine commit:** v1.2 release notes
**Our status:** Implemented. -msaa <samples> command line flag
enables multisampling via SDL3 OpenGL attributes.

### 5. OpenGL Profile Selector
- Choose between Compatibility profile (legacy) and Core profile (GL 3.3)
- Core profile can improve performance on newer GPUs

**PiccuEngine commit:** `0206047`, v1.2.1 release
**Our status:** Not implemented. We use GL 3.2 Core by default.

### 6. Gamma Correction via Shaders
- Proper gamma correction using GPU shaders

**PiccuEngine v1.1:** "Gamma correction now works, using shaders"
**Our status:** Implemented. Gamma uses shader-based correction.
Extended range to [0.1, 3.0] for bright monitors.

### 7. HUD Text Scaling
- HUD text scales with resolution

**PiccuEngine feature:** Listed in v1.0 features
**Our status:** Implemented. HUD text scales based on window height
relative to 1080p threshold in RenderHUDItems().

---

## Medium-Priority (Good Impact, Lower Effort)

### 8. Mouse Improvements
- No 20hz polling rate limit (polls at native mouse rate)
- Mouse buttons 4 and 5 usable in-game
- Fixed mouse button 5 reading as scroll wheel input

**PiccuEngine commits:** v1.1 features, `4a7ba93`
**Our status:** Implemented. Removed 20Hz polling limit, remapped side
buttons to MOUSE_B4/MOUSE_B5, remapped scroll wheel to MOUSE_B6/MOUSE_B7,
fixed ddio_MouseGetState to not clear side button bits.

### 9. OpenAL Sound System
- Replaces DirectSound with OpenAL Soft
- No audio crackling
- Environment reverb support
- Toggle doppler and reverb from sound menu

**PiccuEngine v1.0/v1.2 features**
**Our status:** We use SDL audio. OpenAL would be a larger change.

### 10. Sound Channel Improvements
- New scoring algorithm for bumping sounds (prioritizes closer sounds)
- Disqualify looping sounds from being bumped (fixes fusion charge sound)
- Limit player pain sound intervals (prevents spam)
- Limit player-to-generic object collision sounds

**PiccuEngine commits:** `26e9fbc`, `50ca88a`, `c46f780`, `6442a8d`
**Our status:** Partially implemented. Pain sound throttle added.
Looping sounds already protected by SSF_PLAY_LOOPING check in
sdlsound.cpp. Bumping algorithm and collision sounds not changed.

### 11. Multiplayer Weapon Selection
- Don't prioritize weapons that are out of ammo
- When selecting a weapon slot, pick the higher priority weapon

**PiccuEngine commits:** `f3121a8`, `7735520`
**Our status:** Implemented. Added is_primary_weapon_usable() that
checks ammo/energy availability for primary weapons in SelectPrimaryWeapon().

### 12. Videos Expand to Screen Width
- FMV cutscenes use full screen width instead of 4:3 box

**PiccuEngine v1.2:** "Videos are no longer put in a 4:3 box"
**Our status:** Implemented. Removed pow2 texture restriction,
set renderer to match video dimensions, draw at (0,0) to fill screen,
restore renderer state after playback.

### 13. Framerate Limiter Precision
- More precise framerate limiter
- Hard cap for framerate limit

**PiccuEngine commits:** `fee6198`, v1.1 notes
**Our status:** Implemented. Uses millisecond-precision timer with
proper sleep calculation. Framecap default is 60 FPS.

### 14. Version in Title Bar
- Git-extracted version shown in window title

**PiccuEngine commit:** `2a1fee6`
**Our status:** Implemented. Window title now shows "Descent 3 - D3_GIT_HASH".

---

## Lower-Priority (Bug Fixes Worth Porting)

### 15. Fix Homing Missile Warning After Level Change
- Warning sound works correctly after changing levels

**PiccuEngine commit:** `3c0b62d`
**Our status:** Implemented. Reset last_homing_warning_sound_time and
last_hit_wall_sound_time in InitPlayerNewLevel().

### 16. Fix Key Ramping at 0
- Slow movement when keyboard ramping is exactly 0

**PiccuEngine commits:** `f07dcb9`, `bead678`
**Our status:** Implemented. Added unramped_control_value() that
normalizes keyboard input to -1/0/+1 when ramping is disabled.

### 17. Fix Scorch -1 Bug
- Crash when scorch index is -1

**PiccuEngine commit:** `4d16021`
**Our status:** Implemented. DrawScorches() returns early when
Scorch_end == -1 instead of asserting.

### 18. Fix Fusion Damage Scaling
- Fusion damage scales based on framerate (was broken)

**PiccuEngine v1.0.1:** "Fusion damage should now scale based on framerate"
**Our status:** Related to #549 physics timestep fix.

### 19. Fix Viewer_object Null Crash
- Crash when Viewer_object is null

**PiccuEngine commit:** `0fcd088`
**Our status:** Partially implemented. Added null guards to
DoRainEffect() and DoSnowEffect() in weather.cpp.

### 20. Fix OOF Model Polygon Stack Corruption
- Stack corruption when glow attached and more than 30 vertices

**PiccuEngine v1.1:** "OOF model polygons won't cause stack corruption"
**Our status:** Bug may exist in our code.

### 21. Fix SortPostrenders Buggy Quicksort
- Replace original quicksort with std::sort

**PiccuEngine commit:** `c1d3532`
**Our status:** Implemented. Replaced hand-rolled quicksort with
std::sort in postrender.cpp.

### 22. Fix HUD Positioning at Different Resolutions
- HUD elements positioned correctly regardless of resolution

**PiccuEngine v1.1:** "Fix some HUD elements being positioned
differently based on resolution"
**Our status:** Implemented. Related to #685 resolution fix and
HUD text scaling.

### 23. Ship Modification Unload Reminder
- Message reminds player to unload ship modifications

**PiccuEngine commit:** `646c0fb`
**Our status:** Not implemented. Requires UI/messaging changes.

### 24. Mass Driver Zoom Fix
- MD zoom no longer sticks across new ships/levels

**PiccuEngine commit:** `e55c23c`
**Our status:** Implemented. Reset Render_FOV to Render_FOV_setting
in InitPlayerNewShip().

---

## Credit

All improvements listed above originate from
[InsanityBringer/PiccuEngine](https://github.com/InsanityBringer/PiccuEngine).
When porting any of these changes, proper attribution must be given to
InsanityBringer. Both projects are GPL-3.0 licensed, so code can be
shared freely with attribution.

**Primary author:** InsanityBringer
**License:** GPL-3.0
**Repository:** https://github.com/InsanityBringer/PiccuEngine
