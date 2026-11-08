#pragma once

#include "vertex.hpp"

#include <cstdint>
#include <vector>

struct MeshData final {
  std::vector<Vertex> vertices;
  std::vector<std::uint32_t> indices;
};
