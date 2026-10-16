#include "mesh.h"

#include "vertex.h"

#include <glad/gl.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <utility>

Mesh::~Mesh() { Reset(); }

Mesh::Mesh(Mesh &&other) noexcept
    : vbo_{std::exchange(other.vbo_, 0)}, ebo_{std::exchange(other.ebo_, 0)}, vao_{std::exchange(other.vao_, 0)},
      indices_size_{std::exchange(other.indices_size_, 0)} {}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    Reset();
    vbo_ = std::exchange(other.vbo_, 0);
    ebo_ = std::exchange(other.ebo_, 0);
    vao_ = std::exchange(other.vao_, 0);
    indices_size_ = std::exchange(other.indices_size_, 0);
  }
  return *this;
}

std::expected<Mesh, std::string> Mesh::Create(std::span<const Vertex> vertices,
                                              std::span<const std::uint32_t> indices) {
  if (vertices.empty()) {
    return std::unexpected{"Mesh has no vertices"};
  }

  if (indices.empty()) {
    return std::unexpected{"Mesh has no indices"};
  }

  std::uint32_t vbo{};
  std::uint32_t ebo{};
  std::uint32_t vao{};

  glCreateBuffers(1, &vbo);
  glCreateBuffers(1, &ebo);

  glCreateVertexArrays(1, &vao);

  // Take ownership immediately so partially created objects are released on failure.
  Mesh mesh{vbo, ebo, vao, indices.size()};
  if (vbo == 0 || ebo == 0 || vao == 0) {
    return std::unexpected{"Failed to create mesh buffers"};
  }

  glNamedBufferStorage(vbo, static_cast<std::ptrdiff_t>(vertices.size() * sizeof(Vertex)), vertices.data(), 0);
  glNamedBufferStorage(ebo, static_cast<std::ptrdiff_t>(indices.size() * sizeof(std::uint32_t)), indices.data(), 0);

  glVertexArrayVertexBuffer(vao, 0, vbo, 0, static_cast<std::int32_t>(sizeof(Vertex)));
  glVertexArrayElementBuffer(vao, ebo);

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));

  glVertexArrayBindingDivisor(vao, 0, 0);

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);

  return mesh;
}

std::uint32_t Mesh::GetVertexArray() const noexcept { return vao_; }

std::size_t Mesh::GetIndicesSize() const noexcept { return indices_size_; }

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
Mesh::Mesh(std::uint32_t vbo, std::uint32_t ebo, std::uint32_t vao, std::size_t indices_size)
    : vbo_{vbo}, ebo_{ebo}, vao_{vao}, indices_size_{indices_size} {}

void Mesh::Reset() noexcept {
  if (vao_ != 0) {
    glDeleteVertexArrays(1, &vao_);
    vao_ = 0;
  }
  if (ebo_ != 0) {
    glDeleteBuffers(1, &ebo_);
    ebo_ = 0;
  }
  if (vbo_ != 0) {
    glDeleteBuffers(1, &vbo_);
    vbo_ = 0;
  }
  indices_size_ = 0;
}
