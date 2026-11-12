#pragma once

#include <glm/glm.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

#include "types.hpp"

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

  void ReloadIfChanged();

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

private:
  explicit Program(u32 handle);

  [[nodiscard]] i32 GetUniformLocation(const char *name) const;

  void Reset() noexcept;

  u32 handle_;
  std::filesystem::path vs_path_;
  std::filesystem::path fs_path_;
  std::filesystem::file_time_type vs_attempted_write_time_;
  std::filesystem::file_time_type fs_attempted_write_time_;
  mutable std::unordered_map<std::string, i32> uniform_locations_;
};
