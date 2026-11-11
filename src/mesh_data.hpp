#pragma once

#include <cstdint>
#include <vector>

#include "vertex.hpp"

struct MeshData final {
  std::vector<Vertex> vertices;
  std::vector<std::uint32_t> indices;
};
