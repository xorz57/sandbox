#pragma once

#include <glm/glm.hpp>

#include "types.hpp"

struct PointLight final {
  glm::vec3 position{};
  glm::vec3 ambient{};
  glm::vec3 diffuse{};
  glm::vec3 specular{};
  f32 kc{};
  f32 kl{};
  f32 kq{};
};
