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

  // TODO: Consider using std::expected
  [[nodiscard]] static std::optional<Texture> Create(const std::filesystem::path &path);

  void Bind(u32 unit) const;

  static void Unbind(u32 unit);

private:
  explicit Texture(u32 handle);

  u32 m_handle;
};
