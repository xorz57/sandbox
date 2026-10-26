#pragma once

#include "vertex.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>

class Mesh final {
public:
  ~Mesh();

  Mesh(const Mesh &) = delete;
  Mesh(Mesh &&other) noexcept;

  Mesh &operator=(const Mesh &) = delete;
  Mesh &operator=(Mesh &&other) noexcept;

  [[nodiscard]] static std::expected<Mesh, std::string> Create(std::span<const Vertex> vertices,
                                                               std::span<const std::uint32_t> indices);

  [[nodiscard]] std::uint32_t GetVertexArray() const noexcept;
  [[nodiscard]] std::size_t GetIndexCount() const noexcept;

private:
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  Mesh(std::uint32_t vbo, std::uint32_t ebo, std::uint32_t vao, std::size_t index_count);

  void Reset() noexcept;

  std::uint32_t vbo_;
  std::uint32_t ebo_;
  std::uint32_t vao_;
  std::size_t index_count_;
};
