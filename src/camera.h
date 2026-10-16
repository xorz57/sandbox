#pragma once

#include <glm/glm.hpp>

class Camera final {
public:
  explicit Camera(glm::vec3 position = {});

  [[nodiscard]] glm::mat4 ComputeViewMatrix() const;
  [[nodiscard]] glm::vec3 GetPosition() const;
  [[nodiscard]] glm::vec3 GetFront() const;
  [[nodiscard]] glm::vec3 GetRight() const;

  void Move(glm::vec3 offset);

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  void Rotate(float yaw, float pitch);

private:
  void UpdateVectors();

  glm::vec3 position_;
  glm::vec3 front_{0.0F, 0.0F, -1.0F};
  glm::vec3 right_{1.0F, 0.0F, 0.0F};
  float yaw_{-90.0F};
  float pitch_{0.0F};
};
