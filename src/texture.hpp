#pragma once

#include <filesystem>
#include <optional>

#include "types.hpp"

class Texture final {
public:
  ~Texture();

  Texture(const Texture &) = delete;
  Texture(Texture &&other) noexcept;

  Texture &operator=(const Texture &) = delete;
  Texture &operator=(Texture &&other) noexcept;

  [[nodiscard]] static std::optional<Texture> Create(const std::filesystem::path &path);

  void Bind(u32 unit) const;

private:
  explicit Texture(u32 handle);

  void Reset() noexcept;

  u32 m_handle;
};
