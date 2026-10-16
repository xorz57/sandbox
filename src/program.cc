#include "program.h"

#include "shader.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <utility>

namespace {

[[nodiscard]] std::int32_t GetUniformLocation(std::uint32_t handle, const char *name) {
  assert(handle != 0);

  return glGetUniformLocation(handle, name);
}

} // namespace

Program::~Program() { Reset(); }

Program::Program(Program &&other) noexcept : handle_{std::exchange(other.handle_, 0)} {}

Program &Program::operator=(Program &&other) noexcept {
  if (this != &other) {
    Reset();
    handle_ = std::exchange(other.handle_, 0);
  }
  return *this;
}

std::expected<Program, std::string> Program::Create(const Shader &vs, const Shader &fs) {
  assert(vs.GetHandle() != 0);
  assert(fs.GetHandle() != 0);

  const std::uint32_t handle{glCreateProgram()};
  if (handle == 0) {
    return std::unexpected{"Failed to create program"};
  }

  glAttachShader(handle, vs.GetHandle());
  glAttachShader(handle, fs.GetHandle());
  glLinkProgram(handle);

  std::int32_t success{};
  glGetProgramiv(handle, GL_LINK_STATUS, &success);
  if (success == 0) {
    std::int32_t info_log_length{};
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &info_log_length);
    std::string info_log(static_cast<std::size_t>(info_log_length), '\0');
    glGetProgramInfoLog(handle, info_log_length, nullptr, info_log.data());
    if (info_log_length > 0) {
      info_log.resize(static_cast<std::size_t>(info_log_length) - 1);
    }

    glDeleteProgram(handle);
    return std::unexpected{std::format("Failed to link program:\n{}", info_log)};
  }

  return Program{handle};
}

void Program::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteProgram(handle_);
    handle_ = 0;
  }
}

void Program::Use() const {
  assert(handle_ != 0);
  glUseProgram(handle_);
}

void Program::SetUniform(const char *name, float v0) const {
  glProgramUniform1f(handle_, GetUniformLocation(handle_, name), v0);
}

void Program::SetUniform(const char *name, std::int32_t v0) const {
  glProgramUniform1i(handle_, GetUniformLocation(handle_, name), v0);
}

void Program::SetUniform(const char *name, std::uint32_t v0) const {
  glProgramUniform1ui(handle_, GetUniformLocation(handle_, name), v0);
}

void Program::SetUniform(const char *name, const glm::vec2 &value) const {
  glProgramUniform2fv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::vec3 &value) const {
  glProgramUniform3fv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::vec4 &value) const {
  glProgramUniform4fv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec2 &value) const {
  glProgramUniform2iv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec3 &value) const {
  glProgramUniform3iv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec4 &value) const {
  glProgramUniform4iv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec2 &value) const {
  glProgramUniform2uiv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec3 &value) const {
  glProgramUniform3uiv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec4 &value) const {
  glProgramUniform4uiv(handle_, GetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat2 &value) const {
  glProgramUniformMatrix2fv(handle_, GetUniformLocation(handle_, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat3 &value) const {
  glProgramUniformMatrix3fv(handle_, GetUniformLocation(handle_, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat4 &value) const {
  glProgramUniformMatrix4fv(handle_, GetUniformLocation(handle_, name), 1, GL_FALSE, glm::value_ptr(value));
}

std::uint32_t Program::GetHandle() const noexcept { return handle_; }

Program::Program(std::uint32_t handle) : handle_{handle} {}
