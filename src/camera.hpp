#pragma once

#include <glm/glm.hpp>

struct Camera final {
  glm::vec3 position{};
  glm::vec3 orientation{};
  float fov{60.0F};
  float near_plane{0.1F};
  float far_plane{100.0F};

  [[nodiscard]] glm::mat4 compute_view_matrix() const;
  [[nodiscard]] glm::mat4 compute_projection_matrix(float aspect_ratio) const;
};
