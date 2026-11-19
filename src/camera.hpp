#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#include "types.hpp"

struct Camera final {
  glm::vec3 position{};
  glm::quat orientation{1.0F, 0.0F, 0.0F, 0.0F};
  f32 aspect_ratio{};
  f32 yfov{glm::pi<float>() / 3.0F};
  f32 zfar{100.0F};
  f32 znear{0.1F};

  [[nodiscard]] glm::mat4 ComputeProjectionMatrix() const;

  [[nodiscard]] glm::mat4 ComputeViewMatrix() const;
};
