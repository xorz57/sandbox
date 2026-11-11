#pragma once

#include <cstdint>
#include <optional>

#include "mesh_data.hpp"

class Mesh final {
public:
  ~Mesh();

  Mesh(const Mesh &) = delete;
  Mesh(Mesh &&other) noexcept;

  Mesh &operator=(const Mesh &) = delete;
  Mesh &operator=(Mesh &&other) noexcept;

  [[nodiscard]] static std::optional<Mesh> Create(const MeshData &data);

  void Draw() const;

private:
  Mesh(std::uint32_t vao, std::uint32_t vbo, std::uint32_t ebo, std::int32_t index_count);

  void Reset() noexcept;

  std::uint32_t vao_{};
  std::uint32_t vbo_{};
  std::uint32_t ebo_{};
  std::int32_t index_count_{};
};
