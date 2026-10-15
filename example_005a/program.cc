#include "program.h"

#include "shader.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <utility>

namespace {

[[nodiscard]] std::int32_t GetUniformLocation(std::uint32_t handle, const char *name) {
  if (handle == 0) {
    spdlog::error("Program not initialized");
    return -1;
  }

  const std::int32_t location{glGetUniformLocation(handle, name)};
  if (location == -1) {
    spdlog::warn("Uniform not found: {}", name);
  }
  return location;
}

} // namespace

Program::Program(std::uint32_t handle) : handle_{handle} {}

Program::~Program() { Reset(); }

Program::Program(Program &&other) noexcept : handle_{std::exchange(other.handle_, 0)} {}

Program &Program::operator=(Program &&other) noexcept {
  if (this != &other) {
    Reset();
    handle_ = std::exchange(other.handle_, 0);
  }
  return *this;
}

std::expected<Program, std::string> Program::Create(const Shader &vertex_shader, const Shader &fragment_shader) {
  if (vertex_shader.GetHandle() == 0) {
    return std::unexpected{"Vertex shader not initialized"};
  }
  if (fragment_shader.GetHandle() == 0) {
    return std::unexpected{"Fragment shader not initialized"};
  }

  const std::uint32_t handle{glCreateProgram()};
  if (handle == 0) {
    return std::unexpected{"Failed to create program"};
  }

  glAttachShader(handle, vertex_shader.GetHandle());
  glAttachShader(handle, fragment_shader.GetHandle());
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
    return std::unexpected{"Failed to link program"};
  }

  return Program{handle};
}

void Program::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteProgram(handle_);
    handle_ = 0;
  }
}

std::uint32_t Program::GetHandle() const noexcept { return handle_; }

void Program::Use() const {
  if (handle_ == 0) {
    spdlog::error("Program not initialized");
  }
  glUseProgram(handle_);
}

void Program::SetUniform(const char *name, float v0) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform1f(handle_, location, v0);
  }
}

void Program::SetUniform(const char *name, std::int32_t v0) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform1i(handle_, location, v0);
  }
}

void Program::SetUniform(const char *name, std::uint32_t v0) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform1ui(handle_, location, v0);
  }
}

void Program::SetUniform(const char *name, const glm::vec2 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform2fv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::vec3 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform3fv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::vec4 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform4fv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::ivec2 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform2iv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::ivec3 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform3iv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::ivec4 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform4iv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::uvec2 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform2uiv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::uvec3 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform3uiv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::uvec4 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniform4uiv(handle_, location, 1, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::mat2 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniformMatrix2fv(handle_, location, 1, GL_FALSE, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::mat3 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniformMatrix3fv(handle_, location, 1, GL_FALSE, glm::value_ptr(value));
  }
}

void Program::SetUniform(const char *name, const glm::mat4 &value) const {
  const std::int32_t location{GetUniformLocation(handle_, name)};
  if (location != -1) {
    glProgramUniformMatrix4fv(handle_, location, 1, GL_FALSE, glm::value_ptr(value));
  }
}
