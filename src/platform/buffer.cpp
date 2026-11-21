#include "buffer.hpp"

#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "types.hpp"

namespace platform {

Buffer::Buffer(std::string_view label, usize size)
    : m_label{label},
      m_size{size} {
  glCreateBuffers(1, &m_handle);
  glNamedBufferStorage(m_handle, static_cast<GLsizeiptr>(m_size), nullptr, GL_DYNAMIC_STORAGE_BIT);
  glObjectLabel(GL_BUFFER, m_handle, static_cast<GLsizei>(m_label.size()), m_label.data());
}

Buffer::Buffer(std::string_view label, std::span<const std::byte> data)
    : m_label{label},
      m_size{data.size()} {
  glCreateBuffers(1, &m_handle);
  glNamedBufferStorage(m_handle, static_cast<GLsizeiptr>(m_size), data.data(), GL_DYNAMIC_STORAGE_BIT);
  glObjectLabel(GL_BUFFER, m_handle, static_cast<GLsizei>(m_label.size()), m_label.data());
}

Buffer::~Buffer() {
  if (m_handle != 0) {
    glDeleteBuffers(1, &m_handle);
  }
}

Buffer::Buffer(Buffer &&other) noexcept
    : m_label{std::exchange(other.m_label, {})},
      m_size{std::exchange(other.m_size, 0)},
      m_handle{std::exchange(other.m_handle, 0)} {}

Buffer &Buffer::operator=(Buffer &&other) noexcept {
  if (this != &other) {
    if (m_handle != 0) {
      glDeleteBuffers(1, &m_handle);
    }
    m_label = std::exchange(other.m_label, {});
    m_size = std::exchange(other.m_size, 0);
    m_handle = std::exchange(other.m_handle, 0);
  }
  return *this;
}

void Buffer::Update(std::span<const std::byte> data, usize offset) const {
  const usize end{offset + data.size()};
  if (end < offset) {
    spdlog::error("Buffer '{}': update range overflow.", m_label);
    return;
  }

  if (end > m_size) {
    spdlog::error("Buffer '{}': update out of bounds.", m_label);
    return;
  }

  glNamedBufferSubData(m_handle, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(data.size()), data.data());
}

std::string_view Buffer::GetLabel() const noexcept { return m_label; }

usize Buffer::GetSize() const noexcept { return m_size; }

u32 Buffer::GetHandle() const noexcept { return m_handle; }

} // namespace platform
