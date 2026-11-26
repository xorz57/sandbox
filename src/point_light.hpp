#pragma once

#include <glm/glm.hpp>

#include "types.hpp"

struct PointLight final {
  glm::vec3 position{};
  glm::vec3 color{1.0F, 1.0F, 1.0F};
  f32 intensity{1.0F};
  f32 kc{};
  f32 kl{};
  f32 kq{};
};
