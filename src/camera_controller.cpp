#include "camera_controller.hpp"

#include "camera.hpp"

#include <glm/glm.hpp>

CameraController::CameraController(Camera &camera, Settings settings) : camera_{camera}, settings_{settings} {}

CameraController::CameraController(Camera &camera) : CameraController{camera, {}} {}

void CameraController::Move(Direction direction, float delta_time, float speed_multiplier) {
  const float length{glm::length(glm::vec3{direction.right, direction.up, direction.forward})};
  if (length == 0.0F) {
    return;
  }
  Camera &camera{camera_.get()};
  const float distance{settings_.speed * speed_multiplier * delta_time / length};
  camera.MoveRight(direction.right * distance);
  camera.MoveUp(direction.up * distance);
  camera.MoveForward(direction.forward * distance);
}

void CameraController::Look(float x_delta, float y_delta) {
  Camera &camera{camera_.get()};
  camera.Rotate(x_delta * settings_.sensitivity, -y_delta * settings_.sensitivity);
}
