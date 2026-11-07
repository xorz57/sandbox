#pragma once

#include <glm/glm.hpp>

class Camera final {
public:
  Camera(glm::vec3 position, glm::vec3 orientation);

  void set_position(glm::vec3 position);
  void set_orientation(glm::vec3 orientation);
  void set_fov(float fov);
  void set_near_plane(float near_plane);
  void set_far_plane(float far_plane);

  [[nodiscard]] glm::vec3 get_position() const;
  [[nodiscard]] glm::vec3 get_orientation() const;
  [[nodiscard]] float get_fov() const;
  [[nodiscard]] float get_near_plane() const;
  [[nodiscard]] float get_far_plane() const;

  [[nodiscard]] glm::mat4 compute_view_matrix() const;
  [[nodiscard]] glm::mat4 compute_projection_matrix(float aspect_ratio) const;

private:
  glm::vec3 position_;
  glm::vec3 orientation_;
  float fov_{45.0F};
  float near_plane_{0.1F};
  float far_plane_{100.0F};
};
