#pragma once

#include "vertex.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

class Mesh final {
public:
  enum class Error : std::uint8_t {
    kVerticesEmpty,
    kIndicesEmpty,
  };

  ~Mesh();

  Mesh(const Mesh &) = delete;
  Mesh(Mesh &&other) noexcept;

  Mesh &operator=(const Mesh &) = delete;
  Mesh &operator=(Mesh &&other) noexcept;

  [[nodiscard]] static std::expected<Mesh, Error> Create(std::span<const Vertex> vertices,
                                                         std::span<const std::uint32_t> indices);

  void Reset() noexcept;

  [[nodiscard]] std::uint32_t GetVertexArray() const;

  [[nodiscard]] std::size_t GetIndicesSize() const;

private:
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  Mesh(std::uint32_t vbo, std::uint32_t ebo, std::uint32_t vao, std::size_t indices_size);

  std::uint32_t vbo_;
  std::uint32_t ebo_;
  std::uint32_t vao_;
  std::size_t indices_size_;
};
