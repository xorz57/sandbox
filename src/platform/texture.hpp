#pragma once

#include <filesystem>
#include <optional>

#include "types.hpp"

namespace platform {

class Texture final {
public:
  struct PixelFormat {
    u32 internal_format{};
    u32 format{};
    u32 type{};
  };

  ~Texture();

  Texture(const Texture &) = delete;
  Texture(Texture &&other) noexcept;

  Texture &operator=(const Texture &) = delete;
  Texture &operator=(Texture &&other) noexcept;

  [[nodiscard]] static std::optional<Texture> Create(const std::filesystem::path &path);

  void Bind(u32 unit) const;

  static void Unbind(u32 unit);

  [[nodiscard]] u32 GetHandle() const;

private:
  explicit Texture(u32 handle);

  void Reset();

  u32 m_handle;
};

} // namespace platform
