#include "camera_controller.h"

#include "camera.h"

#include <glm/glm.hpp>

CameraController::CameraController(Camera &camera, Settings settings) : camera_{camera}, settings_{settings} {}

CameraController::CameraController(Camera &camera) : CameraController{camera, {}} {}

void CameraController::ProcessKeyboard(Movement movement, float delta_time, float speed_multiplier) {
  Camera &camera{camera_.get()};
  const float distance{settings_.speed * speed_multiplier * delta_time};
  switch (movement) {
  case Movement::kForward:
    camera.MoveForward(distance);
    break;
  case Movement::kBackward:
    camera.MoveForward(-distance);
    break;
  case Movement::kLeft:
    camera.MoveRight(-distance);
    break;
  case Movement::kRight:
    camera.MoveRight(distance);
    break;
  case Movement::kUp:
    camera.MoveUp(distance);
    break;
  case Movement::kDown:
    camera.MoveUp(-distance);
    break;
  }
}

void CameraController::ProcessMouseDelta(float x_delta, float y_delta) {
  Camera &camera{camera_.get()};
  camera.Rotate(x_delta * settings_.sensitivity, -y_delta * settings_.sensitivity);
}
