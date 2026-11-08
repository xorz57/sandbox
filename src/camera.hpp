#pragma once

#include <glm/glm.hpp>

struct Camera final {
  glm::vec3 position{};
  glm::vec3 orientation{};
  float fov{60.0F};
  float near_plane{0.1F};
  float far_plane{100.0F};

  [[nodiscard]] glm::mat4 ComputeProjectionMatrix(float aspect_ratio) const;
  [[nodiscard]] glm::mat4 ComputeViewMatrix() const;
};
