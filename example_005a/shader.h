#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string_view>

class Shader final {
public:
  enum class Type : std::uint8_t {
    kVertex,
    kFragment,
  };

  enum class Error : std::uint8_t {
    kFileOpenFailed, // Required for Shader::CreateFromFile
    kCreateFailed,
    kCompileFailed,
  };

  ~Shader();

  Shader(const Shader &) = delete;
  Shader(Shader &&other) noexcept;

  Shader &operator=(const Shader &) = delete;
  Shader &operator=(Shader &&other) noexcept;

  [[nodiscard]] static std::expected<Shader, Error> Create(Type type, std::string_view source);
  [[nodiscard]] static std::expected<Shader, Error> CreateFromFile(Type type, const std::filesystem::path &path);

  void Reset() noexcept;

  [[nodiscard]] std::uint32_t GetHandle() const noexcept;

private:
  explicit Shader(std::uint32_t handle);

  std::uint32_t handle_;
};
