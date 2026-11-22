#pragma once

#include "mesh_data.hpp"

namespace primitives {

[[nodiscard]] MeshData MakeTriangle();
[[nodiscard]] MeshData MakeQuad();
[[nodiscard]] MeshData MakeCube();

} // namespace primitives
