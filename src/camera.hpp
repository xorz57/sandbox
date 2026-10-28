#pragma once

#include <glm/glm.hpp>

class Camera final {
public:
  struct Orientation {
    float yaw{-90.0F};
    float pitch{0.0F};
  };

  explicit Camera(glm::vec3 position);

  void SetPosition(glm::vec3 position);
  void SetOrientation(Orientation orientation);

  [[nodiscard]] glm::vec3 GetPosition() const;
  [[nodiscard]] Orientation GetOrientation() const;

  [[nodiscard]] glm::mat4 ComputeViewMatrix() const;
  [[nodiscard]] glm::mat4 ComputeProjectionMatrix(float aspect_ratio) const;

  void MoveForward(float distance);
  void MoveRight(float distance);
  void MoveUp(float distance);

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  void Rotate(float yaw_delta, float pitch_delta);

private:
  void UpdateVectors();

  glm::vec3 position_;
  glm::vec3 front_{0.0F, 0.0F, -1.0F};
  glm::vec3 right_{1.0F, 0.0F, 0.0F};
  float yaw_{-90.0F};
  float pitch_{0.0F};
  float fov_{45.0F};
  float near_plane_{0.1F};
  float far_plane_{100.0F};
};
