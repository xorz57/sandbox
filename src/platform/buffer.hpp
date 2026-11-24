#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "types.hpp"

namespace platform {

class Buffer final {
public:
  ~Buffer();

  Buffer(const Buffer &) = delete;
  Buffer(Buffer &&other) noexcept;

  Buffer &operator=(const Buffer &) = delete;
  Buffer &operator=(Buffer &&other) noexcept;

  [[nodiscard]] static std::optional<Buffer> Create(std::string_view label, usize size);
  [[nodiscard]] static std::optional<Buffer> Create(std::string_view label, std::span<const std::byte> data);

  void Update(std::span<const std::byte> data, usize offset) const;

  [[nodiscard]] std::string_view GetLabel() const;

  [[nodiscard]] usize GetSize() const;

  [[nodiscard]] u32 GetHandle() const;

private:
  Buffer(std::string_view label, usize size, u32 handle);

  void Reset();

  std::string m_label;
  usize m_size{};
  u32 m_handle{};
};

} // namespace platform
