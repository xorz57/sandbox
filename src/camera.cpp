#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

constexpr glm::vec3 world_right{1.0F, 0.0F, 0.0F};
constexpr glm::vec3 world_up{0.0F, 1.0F, 0.0F};
constexpr glm::vec3 world_forward{0.0F, 0.0F, -1.0F};

} // namespace

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
Camera::Camera(glm::vec3 position, glm::vec3 orientation) : position_{position}, orientation_{orientation} {}

void Camera::SetPosition(glm::vec3 position) { position_ = position; }

void Camera::SetOrientation(glm::vec3 orientation) { orientation_ = orientation; }

glm::vec3 Camera::GetPosition() const { return position_; }

glm::vec3 Camera::GetOrientation() const { return orientation_; }

glm::mat4 Camera::ComputeViewMatrix() const {
  glm::mat4 rotation{1.0F};
  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  rotation = glm::rotate(rotation, orientation_.x, world_up);
  rotation = glm::rotate(rotation, orientation_.y, world_right);
  rotation = glm::rotate(rotation, orientation_.z, -world_forward);
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
  const glm::vec3 front{rotation * glm::vec4{world_forward, 0.0F}};
  const glm::vec3 up{rotation * glm::vec4{world_up, 0.0F}};
  return glm::lookAt(position_, position_ + front, up);
}

glm::mat4 Camera::ComputeProjectionMatrix(float aspect_ratio) const {
  return glm::perspective(glm::radians(fov_), aspect_ratio, near_plane_, far_plane_);
}
