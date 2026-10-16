#pragma once

#include "camera.h"

#include <cstdint>
#include <functional>

class CameraController final {
public:
  struct Settings {
    float speed{2.0F};
    float sensitivity{0.1F};
  };

  enum class Movement : std::uint8_t {
    kForward,
    kBackward,
    kLeft,
    kRight,
    kUp,
    kDown,
  };

  explicit CameraController(Camera &camera, Settings settings);
  explicit CameraController(Camera &camera);

  void Move(Movement movement, float delta_time, float speed_multiplier = 1.0F);
  void Look(float x_delta, float y_delta);

private:
  std::reference_wrapper<Camera> camera_;
  Settings settings_;
};
