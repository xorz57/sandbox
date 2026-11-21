#include "renderer.hpp"

#include <glad/gl.h>

#include "types.hpp"

namespace platform {

void Renderer::SetViewport(i32 x, i32 y, i32 width, i32 height) {
  glViewport(x, y, width, height);
}

void Renderer::SetClearColor(const glm::vec4 &color) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
  glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

} // namespace platform
