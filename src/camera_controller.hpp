#pragma once

#include "camera.hpp"

#include <functional>

class CameraController final {
public:
  struct Settings {
    float speed{2.0F};
    float sensitivity{0.1F};
  };

  // Camera-local movement direction; normalized before use.
  struct Direction {
    float right{0.0F};
    float up{0.0F};
    float forward{0.0F};
  };

  explicit CameraController(Camera &camera, Settings settings);
  explicit CameraController(Camera &camera);

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  void Move(Direction direction, float delta_time, float speed_multiplier = 1.0F);
  void Look(float x_delta, float y_delta);

private:
  std::reference_wrapper<Camera> camera_;
  Settings settings_;
};
