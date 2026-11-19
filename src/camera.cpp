#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace {

constexpr glm::vec3 world_up{0.0F, 1.0F, 0.0F};
constexpr glm::vec3 world_forward{0.0F, 0.0F, -1.0F};

} // namespace

glm::mat4 Camera::ComputeProjectionMatrix() const { return glm::perspective(yfov, aspect_ratio, znear, zfar); }

glm::mat4 Camera::ComputeViewMatrix() const {
  const glm::quat normalized_orientation{glm::normalize(orientation)};
  const glm::vec3 front{normalized_orientation * world_forward};
  const glm::vec3 up{normalized_orientation * world_up};
  return glm::lookAt(position, position + front, up);
}
