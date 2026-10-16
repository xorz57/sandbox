#pragma once

#include <glm/glm.hpp>

class Camera final {
public:
  explicit Camera(glm::vec3 position = {});

  [[nodiscard]] glm::mat4 ComputeViewMatrix() const;
  [[nodiscard]] glm::vec3 GetPosition() const;

  void SetPosition(glm::vec3 position);

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  void SetOrientation(float yaw, float pitch);

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
};
