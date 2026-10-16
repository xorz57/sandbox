#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

constexpr float kMaxPitch{89.0F};
constexpr glm::vec3 kWorldUp{0.0F, 1.0F, 0.0F};

} // namespace

Camera::Camera(glm::vec3 position) : position_{position} { UpdateVectors(); }

glm::mat4 Camera::ComputeViewMatrix() const {
  return glm::lookAt(position_, position_ + front_, glm::cross(right_, front_));
}

glm::vec3 Camera::GetPosition() const { return position_; }

void Camera::MoveForward(float distance) { position_ += front_ * distance; }

void Camera::MoveRight(float distance) { position_ += right_ * distance; }

void Camera::MoveUp(float distance) { position_ += kWorldUp * distance; }

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Camera::Rotate(float yaw_delta, float pitch_delta) {
  yaw_ += yaw_delta;
  pitch_ = glm::clamp(pitch_ + pitch_delta, -kMaxPitch, kMaxPitch);
  UpdateVectors();
}

void Camera::UpdateVectors() {
  const glm::vec3 front{
      glm::cos(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_)),
      glm::sin(glm::radians(pitch_)),
      glm::sin(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_)),
  };
  front_ = glm::normalize(front);
  right_ = glm::normalize(glm::cross(front_, kWorldUp));
}
