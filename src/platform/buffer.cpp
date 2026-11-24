#include "buffer.hpp"

#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "types.hpp"

namespace platform {

Buffer::~Buffer() { Reset(); }

Buffer::Buffer(Buffer &&other) noexcept
    : m_label{std::exchange(other.m_label, {})},
      m_size{std::exchange(other.m_size, 0)},
      m_handle{std::exchange(other.m_handle, 0)} {}

Buffer &Buffer::operator=(Buffer &&other) noexcept {
  if (this != &other) {
    Reset();
    m_label = std::exchange(other.m_label, {});
    m_size = std::exchange(other.m_size, 0);
    m_handle = std::exchange(other.m_handle, 0);
  }

  return *this;
}

std::optional<Buffer> Buffer::Create(std::string_view label, usize size) {
  if (size == 0) {
    spdlog::error("Buffer '{}': cannot create buffer with size 0.", label);
    return std::nullopt;
  }

  u32 handle{};
  glCreateBuffers(1, &handle);
  if (handle == 0) {
    spdlog::error("Buffer '{}': failed to create OpenGL buffer object.", label);
    return std::nullopt;
  }

  glNamedBufferStorage(handle, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_STORAGE_BIT);

  glObjectLabel(GL_BUFFER, handle, static_cast<GLsizei>(label.size()), label.data());

  return Buffer{label, size, handle};
}

std::optional<Buffer> Buffer::Create(std::string_view label, std::span<const std::byte> data) {
  if (data.empty()) {
    spdlog::error("Buffer '{}': cannot create buffer from empty data.", label);
    return std::nullopt;
  }

  u32 handle{};
  glCreateBuffers(1, &handle);
  if (handle == 0) {
    spdlog::error("Buffer '{}': failed to create OpenGL buffer object.", label);
    return std::nullopt;
  }

  glNamedBufferStorage(handle, static_cast<GLsizeiptr>(data.size()), data.data(), GL_DYNAMIC_STORAGE_BIT);

  glObjectLabel(GL_BUFFER, handle, static_cast<GLsizei>(label.size()), label.data());

  return Buffer{label, data.size(), handle};
}

void Buffer::Update(std::span<const std::byte> data, usize offset) const {
  if (data.size() > m_size || offset > m_size - data.size()) {
    spdlog::error("Buffer '{}': update out of bounds.", m_label);
    return;
  }

  glNamedBufferSubData(m_handle, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(data.size()), data.data());
}

std::string_view Buffer::GetLabel() const { return m_label; }

usize Buffer::GetSize() const { return m_size; }

u32 Buffer::GetHandle() const { return m_handle; }

Buffer::Buffer(std::string_view label, usize size, u32 handle)
    : m_label{label},
      m_size{size},
      m_handle{handle} {}

void Buffer::Reset() {
  if (m_handle == 0) {
    return;
  }

  glDeleteBuffers(1, &m_handle);
  m_handle = 0;
}

} // namespace platform
