#pragma once

#include <glm/glm.hpp>

#include "types.hpp"

struct Material final {
  glm::vec3 ambient{};
  glm::vec3 diffuse{};
  glm::vec3 specular{};
  f32 shininess{};
};
