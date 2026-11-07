#pragma once

#include <glm/glm.hpp>

class Camera final {
public:
  Camera(glm::vec3 position, glm::vec3 orientation);

  void set_position(glm::vec3 position);
  void set_orientation(glm::vec3 orientation);

  [[nodiscard]] glm::vec3 get_position() const;
  [[nodiscard]] glm::vec3 get_orientation() const;

  [[nodiscard]] glm::mat4 compute_view_matrix() const;
  [[nodiscard]] glm::mat4 compute_projection_matrix(float aspect_ratio) const;

private:
  glm::vec3 position_;
  glm::vec3 orientation_;
  float fov_{45.0F};
  float near_plane_{0.1F};
  float far_plane_{100.0F};
};
