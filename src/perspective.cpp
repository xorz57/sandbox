#include "perspective.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
Perspective::Perspective(float fov, float near_plane, float far_plane)
    : fov_{fov}, near_plane_{near_plane}, far_plane_{far_plane} {}

void Perspective::SetFov(float fov) { fov_ = fov; }

void Perspective::SetNearPlane(float near_plane) { near_plane_ = near_plane; }

void Perspective::SetFarPlane(float far_plane) { far_plane_ = far_plane; }

float Perspective::GetFOV() const { return fov_; }

float Perspective::GetNearPlane() const { return near_plane_; }

float Perspective::GetFarPlane() const { return far_plane_; }

glm::mat4 Perspective::ComputeProjectionMatrix(float aspect_ratio) const {
  return glm::perspective(glm::radians(fov_), aspect_ratio, near_plane_, far_plane_);
}
