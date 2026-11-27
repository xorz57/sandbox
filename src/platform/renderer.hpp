#pragma once

#include <glm/glm.hpp>

#include "types.hpp"

namespace platform {

class Renderer final {
public:
  static void SetViewport(i32 x, i32 y, i32 width, i32 height);

  static void SetClearColor(const glm::vec4 &color);

  static void SetMultisampling(bool enabled);

  static void SetFaceCulling(bool enabled);

  static void SetDepthTesting(bool enabled);

  static void SetFramebufferSRGB(bool enabled);

  static void Clear();
};

} // namespace platform
