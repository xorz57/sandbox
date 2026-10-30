#pragma once

#include <glm/glm.hpp>

class Camera final {
public:
  Camera(glm::vec3 position, glm::vec3 orientation);

  void SetPosition(glm::vec3 position);
  void SetOrientation(glm::vec3 orientation);

  [[nodiscard]] glm::vec3 GetPosition() const;
  [[nodiscard]] glm::vec3 GetOrientation() const;

  [[nodiscard]] glm::mat4 ComputeViewMatrix() const;
  [[nodiscard]] glm::mat4 ComputeProjectionMatrix(float aspect_ratio) const;

private:
  glm::vec3 position_;
  glm::vec3 orientation_;
  float fov_{45.0F};
  float near_plane_{0.1F};
  float far_plane_{100.0F};
};
