#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

class Shader final {
public:
  ~Shader();

  Shader(const Shader &) = delete;
  Shader(Shader &&other) noexcept;

  Shader &operator=(const Shader &) = delete;
  Shader &operator=(Shader &&other) noexcept;

  [[nodiscard]] static std::optional<Shader> create(const char *vs_source, const char *fs_source);
  [[nodiscard]] static std::optional<Shader> create(const std::filesystem::path &vs_path,
                                                    const std::filesystem::path &fs_path);
  [[nodiscard]] static std::optional<Shader> create(const std::filesystem::path &path);

  void use() const;

  void setUniform(const char *name, float value) const;
  void setUniform(const char *name, std::int32_t value) const;
  void setUniform(const char *name, std::uint32_t value) const;
  void setUniform(const char *name, const glm::vec2 &value) const;
  void setUniform(const char *name, const glm::vec3 &value) const;
  void setUniform(const char *name, const glm::vec4 &value) const;
  void setUniform(const char *name, const glm::ivec2 &value) const;
  void setUniform(const char *name, const glm::ivec3 &value) const;
  void setUniform(const char *name, const glm::ivec4 &value) const;
  void setUniform(const char *name, const glm::uvec2 &value) const;
  void setUniform(const char *name, const glm::uvec3 &value) const;
  void setUniform(const char *name, const glm::uvec4 &value) const;
  void setUniform(const char *name, const glm::mat2 &value) const;
  void setUniform(const char *name, const glm::mat3 &value) const;
  void setUniform(const char *name, const glm::mat4 &value) const;

private:
  explicit Shader(std::uint32_t handle);

  [[nodiscard]] std::int32_t getUniformLocation(const char *name) const;

  void reset() noexcept;

  std::uint32_t handle_;
  mutable std::unordered_map<std::string, std::int32_t> uniform_locations_;
};
