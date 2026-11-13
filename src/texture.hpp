#pragma once

#include <array>
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
  [[nodiscard]] static std::optional<Texture> CreateSolidColor(const std::array<u8, 4> &color);
  [[nodiscard]] static std::optional<Texture>
  CreateCheckerboard(i32 w, i32 h, i32 tile_step, const std::array<u8, 4> &color1, const std::array<u8, 4> &color2);

  void Bind(u32 unit) const;

private:
  explicit Texture(u32 handle);

  void Reset() noexcept;

  u32 m_handle;
};
