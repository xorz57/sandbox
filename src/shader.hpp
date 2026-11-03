#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

class Shader final {
public:
  ~Shader();

  Shader(const Shader &) = delete;
  Shader(Shader &&other) noexcept;

  Shader &operator=(const Shader &) = delete;
  Shader &operator=(Shader &&other) noexcept;

  [[nodiscard]] static std::expected<Shader, std::string> Create(std::string_view vs_source,
                                                                 std::string_view fs_source);
  [[nodiscard]] static std::expected<Shader, std::string> CreateFromFiles(const std::filesystem::path &vs_path,
                                                                          const std::filesystem::path &fs_path);
  [[nodiscard]] static std::expected<Shader, std::string> CreateFromFile(const std::filesystem::path &path);

  void Use() const;

  void SetUniform(const char *name, float v0) const;
  void SetUniform(const char *name, std::int32_t v0) const;
  void SetUniform(const char *name, std::uint32_t v0) const;

  void SetUniform(const char *name, const glm::vec2 &value) const;
  void SetUniform(const char *name, const glm::vec3 &value) const;
  void SetUniform(const char *name, const glm::vec4 &value) const;

  void SetUniform(const char *name, const glm::ivec2 &value) const;
  void SetUniform(const char *name, const glm::ivec3 &value) const;
  void SetUniform(const char *name, const glm::ivec4 &value) const;

  void SetUniform(const char *name, const glm::uvec2 &value) const;
  void SetUniform(const char *name, const glm::uvec3 &value) const;
  void SetUniform(const char *name, const glm::uvec4 &value) const;

  void SetUniform(const char *name, const glm::mat2 &value) const;
  void SetUniform(const char *name, const glm::mat3 &value) const;
  void SetUniform(const char *name, const glm::mat4 &value) const;

private:
  explicit Shader(std::uint32_t handle);

  void Reset() noexcept;

  std::uint32_t handle_;
};
