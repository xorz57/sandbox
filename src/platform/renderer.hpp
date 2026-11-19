#pragma once

#include <glm/glm.hpp>

class Renderer final {
public:
  static void SetClearColor(const glm::vec4 &color);

  static void Clear();
};
