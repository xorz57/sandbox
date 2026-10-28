#pragma once

#include <glm/glm.hpp>

struct Perspective final {
  float fov{45.0F};
  float near_plane{0.1F};
  float far_plane{100.0F};
};

[[nodiscard]] glm::mat4 ComputeProjectionMatrix(const Perspective &perspective, float aspect_ratio);
