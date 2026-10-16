#include "camera_controller.h"

#include "camera.h"

#include <glm/glm.hpp>

namespace {

constexpr glm::vec3 kWorldUp{0.0F, 1.0F, 0.0F};

} // namespace

CameraController::CameraController(Camera &camera, Settings settings)
    : camera_{camera}, speed_{settings.speed}, sensitivity_{settings.sensitivity} {}

void CameraController::ProcessKeyboard(Movement movement, float delta_time, float speed_multiplier) {
  Camera &camera{camera_.get()};
  const float velocity{speed_ * speed_multiplier * delta_time};
  switch (movement) {
  case Movement::kForward:
    camera.Move(camera.GetFront() * velocity);
    break;
  case Movement::kBackward:
    camera.Move(camera.GetFront() * -velocity);
    break;
  case Movement::kLeft:
    camera.Move(camera.GetRight() * -velocity);
    break;
  case Movement::kRight:
    camera.Move(camera.GetRight() * velocity);
    break;
  case Movement::kUp:
    camera.Move(kWorldUp * velocity);
    break;
  case Movement::kDown:
    camera.Move(kWorldUp * -velocity);
    break;
  }
}

void CameraController::ProcessMouseDelta(float x_delta, float y_delta) {
  Camera &camera{camera_.get()};
  camera.Rotate(x_delta * sensitivity_, y_delta * sensitivity_);
}
