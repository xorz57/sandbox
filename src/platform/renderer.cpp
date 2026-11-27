#include "renderer.hpp"

#include <glad/gl.h>

#include "types.hpp"

namespace platform {

namespace {

void SetCapability(u32 capability, bool enabled) {
  if (enabled) {
    glEnable(capability);
  } else {
    glDisable(capability);
  }
}

} // namespace

void Renderer::SetViewport(i32 x, i32 y, i32 width, i32 height) {
  glViewport(x, y, width, height);
}

void Renderer::SetClearColor(const glm::vec4 &color) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
  glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::SetMultisampling(bool enabled) { SetCapability(GL_MULTISAMPLE, enabled); }

void Renderer::SetFaceCulling(bool enabled) { SetCapability(GL_CULL_FACE, enabled); }

void Renderer::SetDepthTesting(bool enabled) { SetCapability(GL_DEPTH_TEST, enabled); }

void Renderer::SetFramebufferSRGB(bool enabled) { SetCapability(GL_FRAMEBUFFER_SRGB, enabled); }

void Renderer::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

} // namespace platform
