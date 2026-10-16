#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

constexpr float kMaxPitch{89.0F};
constexpr glm::vec3 kWorldUp{0.0F, 1.0F, 0.0F};

} // namespace

Camera::Camera(glm::vec3 position) : position_{position} { UpdateVectors(); }

glm::mat4 Camera::ComputeViewMatrix() const {
  return glm::lookAt(position_, position_ + front_, glm::normalize(glm::cross(right_, front_)));
}

glm::vec3 Camera::GetPosition() const { return position_; }

glm::vec3 Camera::GetFront() const { return front_; }

glm::vec3 Camera::GetRight() const { return right_; }

void Camera::Move(glm::vec3 offset) { position_ += offset; }

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Camera::Rotate(float yaw, float pitch) {
  yaw_ += yaw;
  pitch_ = glm::clamp(pitch_ + pitch, -kMaxPitch, kMaxPitch);
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
