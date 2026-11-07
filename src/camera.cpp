#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

constexpr glm::vec3 kWorldUp{0.0F, 1.0F, 0.0F};
constexpr glm::vec3 kWorldRight{1.0F, 0.0F, 0.0F};
constexpr glm::vec3 kWorldForward{0.0F, 0.0F, -1.0F};

} // namespace

Camera::Camera(glm::vec3 position, glm::vec3 orientation) : position_{position}, orientation_{orientation} {}

#pragma region Setters

void Camera::set_position(glm::vec3 position) { position_ = position; }

void Camera::set_orientation(glm::vec3 orientation) { orientation_ = orientation; }

void Camera::set_fov(float fov) { fov_ = fov; }

void Camera::set_near_plane(float near_plane) { near_plane_ = near_plane; }

void Camera::set_far_plane(float far_plane) { far_plane_ = far_plane; }

#pragma endregion Setters

#pragma region Getters

glm::vec3 Camera::get_position() const { return position_; }

glm::vec3 Camera::get_orientation() const { return orientation_; }

float Camera::get_fov() const { return fov_; }

float Camera::get_near_plane() const { return near_plane_; }

float Camera::get_far_plane() const { return far_plane_; }

#pragma endregion Getters

#pragma region Computations

glm::mat4 Camera::compute_view_matrix() const {
  glm::mat4 rotation{1.0F};
  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  rotation = glm::rotate(rotation, orientation_.x, kWorldUp);
  rotation = glm::rotate(rotation, orientation_.y, kWorldRight);
  rotation = glm::rotate(rotation, orientation_.z, -kWorldForward);
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
  const glm::vec3 front{rotation * glm::vec4{kWorldForward, 0.0F}};
  const glm::vec3 up{rotation * glm::vec4{kWorldUp, 0.0F}};
  return glm::lookAt(position_, position_ + front, up);
}

glm::mat4 Camera::compute_projection_matrix(float aspect_ratio) const {
  return glm::perspective(glm::radians(fov_), aspect_ratio, near_plane_, far_plane_);
}

#pragma endregion Computations
