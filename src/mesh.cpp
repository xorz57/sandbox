#include "mesh.hpp"

#include "mesh_data.hpp"
#include "scope_exit.hpp"
#include "vertex.hpp"

#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

Mesh::~Mesh() { Reset(); }

Mesh::Mesh(Mesh &&other) noexcept
    : vao_{std::exchange(other.vao_, 0)},
      vbo_{std::exchange(other.vbo_, 0)},
      ebo_{std::exchange(other.ebo_, 0)},
      index_count_{std::exchange(other.index_count_, 0)} {}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    Reset();
    vao_ = std::exchange(other.vao_, 0);
    vbo_ = std::exchange(other.vbo_, 0);
    ebo_ = std::exchange(other.ebo_, 0);
    index_count_ = std::exchange(other.index_count_, 0);
  }
  return *this;
}

std::optional<Mesh> Mesh::Create(const MeshData &data) {
  std::uint32_t vbo{};
  glCreateBuffers(1, &vbo);
  if (vbo == 0) {
    spdlog::error("Failed to create OpenGL vertex buffer.");
    return std::nullopt;
  }
  ScopeExit vbo_guard{[vbo] { glDeleteBuffers(1, &vbo); }};

  std::uint32_t ebo{};
  glCreateBuffers(1, &ebo);
  if (ebo == 0) {
    spdlog::error("Failed to create OpenGL index buffer.");
    return std::nullopt;
  }
  ScopeExit ebo_guard{[ebo] { glDeleteBuffers(1, &ebo); }};

  std::uint32_t vao{};
  glCreateVertexArrays(1, &vao);
  if (vao == 0) {
    spdlog::error("Failed to create OpenGL vertex array.");
    return std::nullopt;
  }
  ScopeExit vao_guard{[vao] { glDeleteVertexArrays(1, &vao); }};

  const std::ptrdiff_t vertices_bytes{static_cast<std::ptrdiff_t>(data.vertices.size() * sizeof(Vertex))};
  glNamedBufferStorage(vbo, vertices_bytes, data.vertices.data(), 0);

  const std::ptrdiff_t indices_bytes{static_cast<std::ptrdiff_t>(data.indices.size() * sizeof(std::uint32_t))};
  glNamedBufferStorage(ebo, indices_bytes, data.indices.data(), 0);

  glVertexArrayVertexBuffer(vao, 0, vbo, 0, static_cast<std::int32_t>(sizeof(Vertex)));
  glVertexArrayElementBuffer(vao, ebo);

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, position)));
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, normal)));
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, uv)));

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);

  vao_guard.Release();
  ebo_guard.Release();
  vbo_guard.Release();
  return Mesh{vao, vbo, ebo, static_cast<std::int32_t>(data.indices.size())};
}

void Mesh::Draw() const {
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);
}

Mesh::Mesh(std::uint32_t vao, std::uint32_t vbo, std::uint32_t ebo, std::int32_t index_count)
    : vao_{vao},
      vbo_{vbo},
      ebo_{ebo},
      index_count_{index_count} {}

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
  index_count_ = 0;
}
