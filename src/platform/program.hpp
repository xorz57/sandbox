#pragma once

#include <glm/glm.hpp>

#include <filesystem>
#include <optional>
#include <string_view>
#include <unordered_map>

#include "types.hpp"

namespace platform {

class Program final {
public:
  ~Program();

  Program(const Program &) = delete;
  Program(Program &&other) noexcept;

  Program &operator=(const Program &) = delete;
  Program &operator=(Program &&other) noexcept;

  [[nodiscard]] static std::optional<Program> Create(const char *vs_source, const char *fs_source);
  [[nodiscard]] static std::optional<Program> Create(const std::filesystem::path &vs_path,
                                                     const std::filesystem::path &fs_path);
  [[nodiscard]] static std::optional<Program> Create(const std::filesystem::path &path);

  void Use() const;

  void SetUniform(const char *name, f32 value) const;
  void SetUniform(const char *name, i32 value) const;
  void SetUniform(const char *name, u32 value) const;
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

  [[nodiscard]] u32 GetHandle() const;

private:
  explicit Program(u32 handle);

  [[nodiscard]] i32 GetUniformLocation(const char *name) const;

  u32 m_handle;
  mutable std::unordered_map<std::string_view, i32> m_uniform_locations;
};

} // namespace platform
