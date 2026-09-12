#pragma once

#include <cstring>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "application.h"
#include "module.h"

class NewBitmap;

class HardwareOpenGL {
public:
  virtual ~HardwareOpenGL() = default;

  void SetParentApplication(oeApplication *app) { parent_application_ = app; }

  virtual bool SetupContext(int width, int height);
  virtual void DestroyContext(bool just_resizing);
  virtual void PresentFrame() const;
  virtual std::unique_ptr<NewBitmap> Screenshot(int width, int height) const;
  virtual void SetFullScreen(bool fullscreen);
  virtual bool InitWindowMode();

  // Fog state capture for the volumetric fog post-process pass.
  // Called by free functions rend_SetFogState/Borders/Color.
  void setSceneFogActive(bool active) { scene_fog_active_ = active; }
  void setSceneFogBorders(float nearz, float farz) { scene_fog_start_ = nearz; scene_fog_end_ = farz; }
  void setSceneFogColor(float r, float g, float b) { scene_fog_color_[0] = r; scene_fog_color_[1] = g; scene_fog_color_[2] = b; }
  bool getSceneFogActive() const { return scene_fog_active_; }
  float getSceneFogStart() const { return scene_fog_start_; }
  float getSceneFogEnd() const { return scene_fog_end_; }
  const float *getSceneFogColor() const { return scene_fog_color_; }

  // Sun light for the volumetric fog pass (Phase 2).  Used for in-scattering
  // and god rays.  Called by free function rend_SetSunLight.
  void setSunLight(float dir_x, float dir_y, float dir_z, float r, float g, float b) {
    sun_dir_[0] = dir_x;
    sun_dir_[1] = dir_y;
    sun_dir_[2] = dir_z;
    sun_color_[0] = r;
    sun_color_[1] = g;
    sun_color_[2] = b;
  }
  const float *getSunDir() const { return sun_dir_; }
  const float *getSunColor() const { return sun_color_; }

  // Per-sector fog volume: an AABB with a density and color.  The fog shader
  // adds the density and uses the color where a ray-march sample is inside
  // the box.
  struct FogVolume {
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
    float density;
    float r, g, b;
  };
  static constexpr int kMaxFogVolumes = 16;

  // Per-sector fog volumes for the volumetric fog pass (Phase 4).  Each
  // volume is an AABB with a density and color; the fog shader adds the
  // density and uses the color where a ray-march sample is inside the box.
  // Called by free functions rend_ClearFogVolumes/rend_AddFogVolume.
  void clearFogVolumes() { num_fog_volumes_ = 0; }
  void addFogVolume(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z, float density,
                    float r, float g, float b) {
    if (num_fog_volumes_ >= kMaxFogVolumes)
      return;
    FogVolume &v = fog_volumes_[num_fog_volumes_++];
    v.min_x = min_x;
    v.min_y = min_y;
    v.min_z = min_z;
    v.max_x = max_x;
    v.max_y = max_y;
    v.max_z = max_z;
    v.density = density;
    v.r = r;
    v.g = g;
    v.b = b;
  }
  int getNumFogVolumes() const { return num_fog_volumes_; }
  const FogVolume *getFogVolume(int index) const {
    return (index >= 0 && index < num_fog_volumes_) ? &fog_volumes_[index] : nullptr;
  }

  // BUGFIX #10: capture the 3D projection and camera modelview from the 3D
  // rendering path (rend_TransformSetProjection/rend_TransformSetModelView,
  // which are only called by g3_RefreshTransforms(false)).  The volumetric fog
  // pass runs at rend_Flip time, after the 2D HUD/UI rendering has overwritten
  // the renderer's projection/view with the ortho passthru, so the pass must
  // use these captured matrices to reconstruct view rays and world positions.
  void setFogProjection(const float proj[16]) {
    memcpy(fog_projection_, proj, sizeof(fog_projection_));
    fog_matrices_captured_ = true;
  }
  void setFogModelView(const float mv[16]) { memcpy(fog_modelview_, mv, sizeof(fog_modelview_)); }

protected:
  SDL_Window *window_ = nullptr;
  SDL_GLContext context_ = nullptr;
  module *opengl_dll_handle_ = nullptr;
  GLuint framebuffer_ = 0;
  GLuint color_buffer_ = 0;
  GLuint depth_buffer_ = 0;
  GLuint framebuffer_width_ = 0;
  GLuint framebuffer_height_ = 0;
  // MSAA resolve target: when msaa_samples_ > 0, framebuffer_ holds
  // multisampled renderbuffers and each frame is resolved into
  // resolve_framebuffer_ (single-sample) before the window blit.
  GLuint resolve_framebuffer_ = 0;
  GLuint resolve_color_buffer_ = 0;
  GLuint resolve_depth_buffer_ = 0;
  GLuint msaa_samples_ = 0;
  oeApplication *parent_application_ = nullptr;
  // Actual depth internal format of the main framebuffer's depth renderbuffer,
  // queried after creation.  glBlitFramebuffer requires the source and
  // destination depth formats to match EXACTLY (Mesa enforces this), and some
  // drivers promote a requested GL_DEPTH_COMPONENT16 to 24/32-bit, so the fog
  // pass scene depth texture must be created with the same format the driver
  // actually allocated.
  GLenum depth_format_ = GL_DEPTH_COMPONENT16;

  // Volumetric fog pass (Phase 1).  When vfog_level_ > 0 the resolved scene
  // is copied into scene_texture_fbo_ (color + depth textures), ray-marched
  // by the fog shader into fog_fbo_, then blitted to the window.  The GL
  // objects are created once in SetupContext and reused every frame, so the
  // members are mutable to allow PresentFrame() (const) to bind them.
  mutable GLuint scene_texture_fbo_ = 0;
  mutable GLuint scene_color_texture_ = 0;
  mutable GLuint scene_depth_texture_ = 0;
  mutable GLuint fog_fbo_ = 0;
  mutable GLuint fog_color_texture_ = 0;
  mutable GLuint fog_vao_ = 0;
  mutable GLuint fog_vbo_ = 0;
  mutable GLuint fog_shader_program_ = 0;
  mutable GLint fog_uniform_scene_color_ = -1;
  mutable GLint fog_uniform_scene_depth_ = -1;
  mutable GLint fog_uniform_fog_color_ = -1;
  mutable GLint fog_uniform_fog_start_ = -1;
  mutable GLint fog_uniform_fog_end_ = -1;
  mutable GLint fog_uniform_fog_density_ = -1;
  mutable GLint fog_uniform_noise_scale_ = -1;
  mutable GLint fog_uniform_noise_freq_ = -1;
  mutable GLint fog_uniform_steps_ = -1;
  mutable GLint fog_uniform_proj00_ = -1;
  mutable GLint fog_uniform_proj11_ = -1;
  mutable GLint fog_uniform_sun_dir_ = -1;
  mutable GLint fog_uniform_sun_color_ = -1;
  mutable GLint fog_uniform_sun_screen_ = -1;
  mutable GLint fog_uniform_god_rays_ = -1;
  mutable GLint fog_uniform_god_ray_samples_ = -1;
  mutable GLint fog_uniform_time_ = -1;
  mutable GLint fog_uniform_wind_ = -1;
  mutable GLint fog_uniform_num_volumes_ = -1;
  mutable GLint fog_uniform_volume_min_ = -1;
  mutable GLint fog_uniform_volume_max_ = -1;
  mutable GLint fog_uniform_volume_color_ = -1;
  mutable GLint fog_uniform_inv_view_ = -1;
  mutable GLint fog_uniform_enable_ = -1;
  mutable GLint fog_attrib_pos_ = -1;
  mutable GLint fog_attrib_uv_ = -1;
  // Fog state captured from rend_SetFog* calls for the post-process pass.
  mutable bool scene_fog_active_ = false;
  mutable float scene_fog_start_ = 0.0f;
  mutable float scene_fog_end_ = 0.0f;
  mutable float scene_fog_color_[3] = {1.0f, 1.0f, 1.0f};
  // Sun light captured from rend_SetSunLight for in-scattering and god rays.
  // Defaults match the Phase 1 hardcoded direction so behavior is unchanged
  // when the game does not set a sun.
  mutable float sun_dir_[3] = {0.371391f, 0.742782f, 0.557086f};
  mutable float sun_color_[3] = {1.0f, 1.0f, 1.0f};
  // Per-sector fog volumes captured from rend_AddFogVolume.
  mutable FogVolume fog_volumes_[kMaxFogVolumes] = {};
  mutable int num_fog_volumes_ = 0;
  uint8_t vfog_level_ = 0; // 0 = off, 1 = low (16 steps), 2 = high (32 steps)
  // BUGFIX #10: 3D projection/modelview captured from the 3D rendering path
  // for the volumetric fog pass (see setFogProjection/setFogModelView).
  mutable float fog_projection_[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  mutable float fog_modelview_[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  mutable bool fog_matrices_captured_ = false;

  // Compiles and links the volumetric fog pass shader, caching uniform and
  // attribute locations. Returns 0 on failure.
  GLuint CompileFogShader() const;
  // Creates the full-screen triangle VAO/VBO used by the fog pass.
  void SetupFogFullScreenTriangle() const;
  // Runs the volumetric fog pass: copies the resolved scene into the scene
  // texture FBO, ray-marches it into fog_fbo_, and leaves fog_fbo_ bound as
  // the blit source for the window present. Returns false (and leaves the
  // scene unmodified) if the scene blit fails, so the caller can present the
  // unfogged scene instead of a black screen.
  bool RenderFogPass() const;
};
