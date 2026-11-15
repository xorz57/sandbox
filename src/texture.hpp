#pragma once

#include <array>
#include <filesystem>
#include <optional>

#include "types.hpp"

class Texture final {
public:
  using Color = std::array<u8, 4>;

  ~Texture();

  Texture(const Texture &) = delete;
  Texture(Texture &&other) noexcept;

  Texture &operator=(const Texture &) = delete;
  Texture &operator=(Texture &&other) noexcept;

  [[nodiscard]] static std::optional<Texture> Create(const std::filesystem::path &path);
  [[nodiscard]] static std::optional<Texture> CreateSolidColor(Color color);
  [[nodiscard]] static std::optional<Texture> CreateCheckerboard(Color color1, Color color2);

  void Bind(u32 unit) const;

private:
  explicit Texture(u32 handle);

  void Reset() noexcept;

  u32 m_handle;
};
