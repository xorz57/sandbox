#include "primitives.hpp"

#include "mesh.hpp"
#include "vertex.hpp"

#include <cstdint>
#include <utility>
#include <vector>

Mesh MakeCube() {
  std::vector<Vertex> vertices{
      // Front (+z)
      {.position = {-0.5F, -0.5F, 0.5F}, .normal = {0.0F, 0.0F, 1.0F}, .uv = {0.0F, 0.0F}},
      {.position = {0.5F, -0.5F, 0.5F}, .normal = {0.0F, 0.0F, 1.0F}, .uv = {1.0F, 0.0F}},
      {.position = {0.5F, 0.5F, 0.5F}, .normal = {0.0F, 0.0F, 1.0F}, .uv = {1.0F, 1.0F}},
      {.position = {-0.5F, 0.5F, 0.5F}, .normal = {0.0F, 0.0F, 1.0F}, .uv = {0.0F, 1.0F}},
      // Back (-z)
      {.position = {0.5F, -0.5F, -0.5F}, .normal = {0.0F, 0.0F, -1.0F}, .uv = {0.0F, 0.0F}},
      {.position = {-0.5F, -0.5F, -0.5F}, .normal = {0.0F, 0.0F, -1.0F}, .uv = {1.0F, 0.0F}},
      {.position = {-0.5F, 0.5F, -0.5F}, .normal = {0.0F, 0.0F, -1.0F}, .uv = {1.0F, 1.0F}},
      {.position = {0.5F, 0.5F, -0.5F}, .normal = {0.0F, 0.0F, -1.0F}, .uv = {0.0F, 1.0F}},
      // Left (-x)
      {.position = {-0.5F, -0.5F, -0.5F}, .normal = {-1.0F, 0.0F, 0.0F}, .uv = {0.0F, 0.0F}},
      {.position = {-0.5F, -0.5F, 0.5F}, .normal = {-1.0F, 0.0F, 0.0F}, .uv = {1.0F, 0.0F}},
      {.position = {-0.5F, 0.5F, 0.5F}, .normal = {-1.0F, 0.0F, 0.0F}, .uv = {1.0F, 1.0F}},
      {.position = {-0.5F, 0.5F, -0.5F}, .normal = {-1.0F, 0.0F, 0.0F}, .uv = {0.0F, 1.0F}},
      // Right (+x)
      {.position = {0.5F, -0.5F, 0.5F}, .normal = {1.0F, 0.0F, 0.0F}, .uv = {0.0F, 0.0F}},
      {.position = {0.5F, -0.5F, -0.5F}, .normal = {1.0F, 0.0F, 0.0F}, .uv = {1.0F, 0.0F}},
      {.position = {0.5F, 0.5F, -0.5F}, .normal = {1.0F, 0.0F, 0.0F}, .uv = {1.0F, 1.0F}},
      {.position = {0.5F, 0.5F, 0.5F}, .normal = {1.0F, 0.0F, 0.0F}, .uv = {0.0F, 1.0F}},
      // Top (+y)
      {.position = {-0.5F, 0.5F, 0.5F}, .normal = {0.0F, 1.0F, 0.0F}, .uv = {0.0F, 0.0F}},
      {.position = {0.5F, 0.5F, 0.5F}, .normal = {0.0F, 1.0F, 0.0F}, .uv = {1.0F, 0.0F}},
      {.position = {0.5F, 0.5F, -0.5F}, .normal = {0.0F, 1.0F, 0.0F}, .uv = {1.0F, 1.0F}},
      {.position = {-0.5F, 0.5F, -0.5F}, .normal = {0.0F, 1.0F, 0.0F}, .uv = {0.0F, 1.0F}},
      // Bottom (-y)
      {.position = {-0.5F, -0.5F, -0.5F}, .normal = {0.0F, -1.0F, 0.0F}, .uv = {0.0F, 0.0F}},
      {.position = {0.5F, -0.5F, -0.5F}, .normal = {0.0F, -1.0F, 0.0F}, .uv = {1.0F, 0.0F}},
      {.position = {0.5F, -0.5F, 0.5F}, .normal = {0.0F, -1.0F, 0.0F}, .uv = {1.0F, 1.0F}},
      {.position = {-0.5F, -0.5F, 0.5F}, .normal = {0.0F, -1.0F, 0.0F}, .uv = {0.0F, 1.0F}},
  };

  // clang-format off
  std::vector<std::uint32_t> indices{
     0,  1,  2,  2,  3,  0,  // Front
     4,  5,  6,  6,  7,  4,  // Back
     8,  9, 10, 10, 11,  8,  // Left
    12, 13, 14, 14, 15, 12,  // Right
    16, 17, 18, 18, 19, 16,  // Top
    20, 21, 22, 22, 23, 20,  // Bottom
  };
  // clang-format on

  return {.vertices = std::move(vertices), .indices = std::move(indices)};
}
