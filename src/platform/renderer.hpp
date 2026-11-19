#pragma once

#include <glm/glm.hpp>

#include "types.hpp"

class Renderer final {
public:
  static void SetViewport(i32 x, i32 y, i32 width, i32 height);

  static void SetClearColor(const glm::vec4 &color);

  static void Clear();
};
