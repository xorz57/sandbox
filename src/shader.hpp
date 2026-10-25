#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

class Shader final {
public:
  enum class Type : std::uint8_t {
    kVertex,
    kFragment,
  };

  ~Shader();

  Shader(const Shader &) = delete;
  Shader(Shader &&other) noexcept;

  Shader &operator=(const Shader &) = delete;
  Shader &operator=(Shader &&other) noexcept;

  [[nodiscard]] static std::expected<Shader, std::string> Create(Type type, std::string_view source);
  [[nodiscard]] static std::expected<Shader, std::string> CreateFromFile(Type type, const std::filesystem::path &path);

  [[nodiscard]] std::uint32_t GetHandle() const noexcept;

private:
  explicit Shader(std::uint32_t handle);

  void Reset() noexcept;

  std::uint32_t handle_;
};
