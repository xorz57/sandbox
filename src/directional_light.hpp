#pragma once

#include <glm/glm.hpp>

#include "types.hpp"

struct DirectionalLight final {
  glm::vec3 direction{};
  glm::vec3 color{1.0F, 1.0F, 1.0F};
  f32 intensity{1.0F};
};
