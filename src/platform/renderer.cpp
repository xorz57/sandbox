#include "renderer.hpp"

#include <glad/gl.h>

void Renderer::SetClearColor(const glm::vec4 &color) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
  glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
