#pragma once

#include "shader.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <expected>
#include <string>

class Program final {
public:
  ~Program();

  Program(const Program &) = delete;
  Program(Program &&other) noexcept;

  Program &operator=(const Program &) = delete;
  Program &operator=(Program &&other) noexcept;

  [[nodiscard]] static std::expected<Program, std::string> Create(const Shader &vertex_shader,
                                                                  const Shader &fragment_shader);

  void Reset() noexcept;

  [[nodiscard]] std::uint32_t GetHandle() const noexcept;

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
  explicit Program(std::uint32_t handle);

  std::uint32_t handle_;
};
