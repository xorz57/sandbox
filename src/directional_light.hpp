#pragma once

#include <glm/glm.hpp>

#include "types.hpp"

struct DirectionalLight final {
  glm::vec3 direction{};
  glm::vec3 color{};
  f32 intensity{};
};
