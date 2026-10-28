#include "perspective.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 ComputeProjectionMatrix(const Perspective &perspective, float aspect_ratio) {
  return glm::perspective(glm::radians(perspective.fov), aspect_ratio, perspective.near_plane, perspective.far_plane);
}
