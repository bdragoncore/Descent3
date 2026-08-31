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

protected:
  SDL_Window *window_ = nullptr;
  SDL_GLContext context_ = nullptr;
  module *opengl_dll_handle_ = nullptr;
  GLuint framebuffer_ = 0;
  GLuint color_buffer_ = 0;
  GLuint depth_buffer_ = 0;
  GLuint framebuffer_width_ = 0;
  GLuint framebuffer_height_ = 0;
  oeApplication *parent_application_ = nullptr;
};
