#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

constexpr glm::vec3 kWorldUp{0.0F, 1.0F, 0.0F};

} // namespace

Camera::Camera(const glm::vec3 &position) : position_{position} { UpdateCameraVectors(); }

glm::mat4 Camera::ComputeViewMatrix() const {
  return glm::lookAt(position_, position_ + front_, glm::normalize(glm::cross(right_, front_)));
}

glm::mat4 Camera::ComputeProjectionMatrix(float aspect_ratio) {
  constexpr float kFov{45.0F};
  constexpr float kNearPlane{0.1F};
  constexpr float kFarPlane{100.0F};
  return glm::perspective(glm::radians(kFov), aspect_ratio, kNearPlane, kFarPlane);
}

void Camera::ProcessKeyboard(Movement movement, float delta_time, float speed_multiplier) {
  constexpr float kMovementSpeed{2.0F};
  const float speed{kMovementSpeed * speed_multiplier * delta_time};
  switch (movement) {
  case Movement::kForward:
    position_ += front_ * speed;
    break;
  case Movement::kBackward:
    position_ -= front_ * speed;
    break;
  case Movement::kLeft:
    position_ -= right_ * speed;
    break;
  case Movement::kRight:
    position_ += right_ * speed;
    break;
  case Movement::kUp:
    position_ += kWorldUp * speed;
    break;
  case Movement::kDown:
    position_ -= kWorldUp * speed;
    break;
  }
}

void Camera::ProcessMouse(float x_offset, float y_offset, bool constrain_pitch) {
  constexpr float kMouseSensitivity{0.1F};
  yaw_ += x_offset * kMouseSensitivity;
  pitch_ += y_offset * kMouseSensitivity;
  if (constrain_pitch) {
    constexpr float kMaxPitch{89.0F};
    pitch_ = glm::clamp(pitch_, -kMaxPitch, kMaxPitch);
  }
  UpdateCameraVectors();
}

glm::vec3 Camera::GetPosition() const { return position_; }

void Camera::UpdateCameraVectors() {
  const glm::vec3 front{
      glm::cos(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_)),
      glm::sin(glm::radians(pitch_)),
      glm::sin(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_)),
  };
  front_ = glm::normalize(front);
  right_ = glm::normalize(glm::cross(front_, kWorldUp));
}
