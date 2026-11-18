#pragma once

#include <glm/glm.hpp>

#include "types.hpp"

struct Camera final {
  glm::vec3 position{};
  glm::vec3 orientation{};
  f32 fov{60.0F};
  f32 near_plane{0.1F};
  f32 far_plane{100.0F};

  [[nodiscard]] glm::mat4 ComputeProjectionMatrix(f32 aspect_ratio) const;

  [[nodiscard]] glm::mat4 ComputeViewMatrix() const;
};
