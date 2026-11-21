#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "types.hpp"

namespace platform {

class Buffer final {
public:
  Buffer(std::string_view label, usize size);

  ~Buffer();

  Buffer(const Buffer &) = delete;
  Buffer(Buffer &&other) noexcept;

  Buffer &operator=(const Buffer &) = delete;
  Buffer &operator=(Buffer &&other) noexcept;

  void Write(std::span<const std::byte> data, usize offset) const;

  [[nodiscard]] std::string_view GetLabel() const noexcept;

  [[nodiscard]] usize GetSize() const noexcept;

  [[nodiscard]] u32 GetHandle() const noexcept;

private:
  std::string m_label;
  usize m_size{};
  u32 m_handle{};
};

} // namespace platform
