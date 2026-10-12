#pragma once

#include <glm/glm.hpp>

#include <cstdint>

class Camera final {
public:
  enum class Movement : std::uint8_t {
    kForward,
    kBackward,
    kLeft,
    kRight,
    kUp,
    kDown,
  };

  explicit Camera(const glm::vec3 &position = {});

  [[nodiscard]] glm::mat4 ComputeViewMatrix() const;
  [[nodiscard]] static glm::mat4 ComputeProjectionMatrix(float aspect_ratio);

  void ProcessKeyboard(Movement movement, float delta_time, float speed_multiplier = 1.0F);
  void ProcessMouseMovement(float x_offset, float y_offset, bool constrain_pitch = true);

  [[nodiscard]] glm::vec3 GetPosition() const;

private:
  void UpdateCameraVectors();

  glm::vec3 position_;
  glm::vec3 front_{0.0F, 0.0F, -1.0F};
  glm::vec3 right_{1.0F, 0.0F, 0.0F};
  float yaw_{-90.0F};
  float pitch_{0.0F};
};
