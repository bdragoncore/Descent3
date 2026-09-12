#pragma once

#include "3d.h"
#include "vecmat.h"

// WeaponImpact: procedural 3D mesh generation for plasma weapon impact effects
// BUGFIX: Replace 2D scorch sprites with 3D procedural meshes when the
// "Weapon Impact" video detail is set to "New".

// Generate a procedural plasma impact mesh aligned to the given face normal.
// The mesh is a ring of quads + center vertex forming a glow dome.
//
// Parameters:
//   center    - impact point on the wall (world space)
//   normal    - wall face normal (world space)
//   right     - face's right vector (world space)
//   up        - face's up vector (world space)
//   size      - visual size of the impact
//   age       - normalized age (0 = just created, 1 = fully faded)
//   outVerts  - output array of 3D vertex positions (center + 24 ring + 1 closing dup)
//   outUvs    - output array of UV coordinates
//
// Returns: number of vertices generated
int GeneratePlasmaImpactMesh(const vector& center, const vector& normal,
                              const vector& right, const vector& up,
                              float size, float age,
                              vector* outVerts, g3UVL* outUvs);

// Draw a single 3D plasma impact on a wall face.
// Uses the procedural mesh + plasma glow shader uniforms.
// Returns true if the 3D path was used (false = fall back to 2D sprite).
bool DrawPlasmaImpact3D(int roomnum, int facenum, const g3Point* quadPoints,
                        float size, float age);

// Draw a 3D plasma trail segment (volumetric blob + glow) for a vis effect.
// pos/end_pos define the trail axis, width is the blob radius, age is the
// normalized lifetime (0 = born, 1 = dead), color16 is a 16-bit GR_RGB16 color.
// bm_handle is the resolved bitmap to sample (TT_LINEAR textured path); pass
// -1 for untextured flat rendering.
void DrawPlasmaTrailSegment(const vector& pos, const vector& end_pos,
                            float width, float age, uint16_t color16,
                            float alpha, int bm_handle);

// Draw a 3D plasma ball (sphere mesh) at the given position.
// Renders a UV sphere with the given bitmap and glow, sized by `size`
// (diameter) and faded by `age` (0 = born, 1 = dead). color16 is a 16-bit
// GR_RGB16 tint, alpha is the vertex alpha multiplier.
void DrawPlasmaBall3D(const vector& pos, float size, float age, uint16_t color16,
                       float alpha, int bm_handle);

// Procedural gradient textures for the 3D plasma effects. Generated once
// on first use and cached:
//   ball texture      — white-hot core fading to green edge (for the bolt)
//   explosion texture — circular soft-falloff white-to-green gradient
// Returns the bitmap handle, or -1 on failure.
int GetPlasmaBallTexture();
int GetPlasmaExplosionTexture();

// Frees the cached procedural textures (call on shutdown/level unload).
void FreePlasmaImpactTextures();

// Draw a 3D wall-hit shockball: an expanding sphere plus a wall-aligned
// impact fan. pos is the impact point, wall_normal is the wall face normal
// (may be zero for open-air explosions, in which case only the ball draws).
// size is the peak diameter, age is normalized lifetime (0 = impact,
// 1 = fully faded). color16 is a 16-bit GR_RGB16 tint, alpha scales fade.
void DrawPlasmaShockball3D(const vector& pos, const vector& wall_normal,
                           float size, float age, uint16_t color16,
                           float alpha, int bm_handle);

// Draw a 3D plasma blob (point sprite replacement) for smoke-puff fireballs.
// Renders a camera-facing octahedron with the given bitmap and glow, sized
// by `size` and faded by `age` (0 = born, 1 = dead). color16 is a 16-bit
// GR_RGB16 tint, alpha is the vertex alpha multiplier.
void DrawPlasmaBlob3D(const vector& pos, float size, float age, uint16_t color16,
                       float alpha, int bm_handle);
