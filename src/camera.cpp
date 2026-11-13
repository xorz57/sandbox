#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "types.hpp"

namespace {

constexpr glm::vec3 c_world_up{0.0F, 1.0F, 0.0F};
constexpr glm::vec3 c_world_right{1.0F, 0.0F, 0.0F};
constexpr glm::vec3 c_world_forward{0.0F, 0.0F, -1.0F};

} // namespace

glm::mat4 Camera::ComputeProjectionMatrix(f32 aspect_ratio) const {
  return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
}

glm::mat4 Camera::ComputeViewMatrix() const {
  glm::mat4 rotation{1.0F};
  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  rotation = glm::rotate(rotation, orientation.x, c_world_up);
  rotation = glm::rotate(rotation, orientation.y, c_world_right);
  rotation = glm::rotate(rotation, orientation.z, -c_world_forward);
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
  const glm::vec3 front{rotation * glm::vec4{c_world_forward, 0.0F}};
  const glm::vec3 up{rotation * glm::vec4{c_world_up, 0.0F}};
  return glm::lookAt(position, position + front, up);
}
