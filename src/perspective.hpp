#pragma once

#include <glm/glm.hpp>

class Perspective final {
public:
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  explicit Perspective(float fov, float near_plane, float far_plane);

  void SetFov(float fov);
  void SetNearPlane(float near_plane);
  void SetFarPlane(float far_plane);

  [[nodiscard]] float GetFOV() const;
  [[nodiscard]] float GetNearPlane() const;
  [[nodiscard]] float GetFarPlane() const;

  [[nodiscard]] glm::mat4 ComputeProjectionMatrix(float aspect_ratio) const;

private:
  float fov_;
  float near_plane_;
  float far_plane_;
};
