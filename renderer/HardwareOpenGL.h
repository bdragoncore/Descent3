#pragma once

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
  GLuint msaa_samples_ = 0;
  oeApplication *parent_application_ = nullptr;

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
  uint8_t vfog_level_ = 0; // 0 = off, 1 = low (16 steps), 2 = high (32 steps)

  // Compiles and links the volumetric fog pass shader, caching uniform and
  // attribute locations. Returns 0 on failure.
  GLuint CompileFogShader() const;
  // Creates the full-screen triangle VAO/VBO used by the fog pass.
  void SetupFogFullScreenTriangle() const;
  // Runs the volumetric fog pass: copies the resolved scene into the scene
  // texture FBO, ray-marches it into fog_fbo_, and leaves fog_fbo_ bound as
  // the blit source for the window present.
  void RenderFogPass() const;
};
