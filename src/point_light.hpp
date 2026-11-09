#pragma once

#include <glm/glm.hpp>

struct PointLight final {
  glm::vec3 position{};
  glm::vec3 ambient{};
  glm::vec3 diffuse{};
  glm::vec3 specular{};
  float kc{};
  float kl{};
  float kq{};
};
