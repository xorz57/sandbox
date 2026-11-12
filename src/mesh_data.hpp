#pragma once

#include <vector>

#include "types.hpp"
#include "vertex.hpp"

struct MeshData final {
  std::vector<Vertex> vertices;
  std::vector<u32> indices;
};
