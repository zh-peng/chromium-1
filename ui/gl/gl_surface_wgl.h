// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GL_GL_SURFACE_WGL_H_
#define UI_GL_GL_SURFACE_WGL_H_

#include "ui/gfx/native_widget_types.h"
#include "ui/gl/gl_surface.h"

namespace gfx {

// Base interface for WGL surfaces.
class GLSurfaceWGL : public GLSurface {
 public:
  GLSurfaceWGL();
  ~GLSurfaceWGL() override;

  // Implement GLSurface.
  void* GetDisplay() override;

  static bool InitializeOneOff();
  static HDC GetDisplayDC();

 private:
  DISALLOW_COPY_AND_ASSIGN(GLSurfaceWGL);
};

// A surface used to render to a view.
class NativeViewGLSurfaceWGL : public GLSurfaceWGL {
 public:
  explicit NativeViewGLSurfaceWGL(gfx::AcceleratedWidget window);
  ~NativeViewGLSurfaceWGL() override;

  // Implement GLSurface.
  bool Initialize() override;
  void Destroy() override;
  bool IsOffscreen() override;
  bool SwapBuffers() override;
  gfx::Size GetSize() override;
  void* GetHandle() override;

 private:
  gfx::AcceleratedWidget window_;
  gfx::AcceleratedWidget child_window_;
  HDC device_context_;

  DISALLOW_COPY_AND_ASSIGN(NativeViewGLSurfaceWGL);
};


// A surface used to render to an offscreen pbuffer.
class PbufferGLSurfaceWGL : public GLSurfaceWGL {
 public:
  explicit PbufferGLSurfaceWGL(const gfx::Size& size);
  ~PbufferGLSurfaceWGL() override;

  // Implement GLSurface.
  bool Initialize() override;
  void Destroy() override;
  bool IsOffscreen() override;
  bool SwapBuffers() override;
  gfx::Size GetSize() override;
  void* GetHandle() override;

 private:
  gfx::Size size_;
  HDC device_context_;
  void* pbuffer_;

  DISALLOW_COPY_AND_ASSIGN(PbufferGLSurfaceWGL);
};

}  // namespace gfx

#endif  // UI_GL_GL_SURFACE_WGL_H_
