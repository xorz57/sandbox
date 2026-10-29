#include "program.hpp"

#include "scope_exit.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace {

[[nodiscard]] std::expected<std::uint32_t, std::string> CompileShader(std::uint32_t type, std::string_view source) {
  const std::uint32_t handle{glCreateShader(type)};
  if (handle == 0) {
    return std::unexpected{"Failed to create shader"};
  }

  const char *const data{source.data()};
  const std::int32_t length{static_cast<std::int32_t>(source.size())};
  glShaderSource(handle, 1, &data, &length);
  glCompileShader(handle);

  std::int32_t success{};
  glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
  if (success == 0) {
    std::int32_t info_log_length{};
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &info_log_length);
    std::string info_log(static_cast<std::size_t>(info_log_length), '\0');
    glGetShaderInfoLog(handle, info_log_length, nullptr, info_log.data());
    if (info_log_length > 0) {
      info_log.resize(static_cast<std::size_t>(info_log_length) - 1);
    }

    glDeleteShader(handle);
    return std::unexpected{std::format("Failed to compile shader:\n{}", info_log)};
  }

  return handle;
}

[[nodiscard]] std::expected<std::string, std::string> ReadFile(const std::filesystem::path &path) {
  const std::ifstream file{path, std::ios::in | std::ios::binary};
  if (!file) {
    return std::unexpected{std::format("Failed to open shader file '{}'", path.string())};
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return buffer.str();
}

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

std::expected<Program, std::string> Program::Create(std::string_view vs_source, std::string_view fs_source) {
  const auto vs{CompileShader(GL_VERTEX_SHADER, vs_source)};
  if (!vs.has_value()) {
    return std::unexpected{vs.error()};
  }
  const ScopeExit vs_guard{[&vs] { glDeleteShader(*vs); }};

  const auto fs{CompileShader(GL_FRAGMENT_SHADER, fs_source)};
  if (!fs.has_value()) {
    return std::unexpected{fs.error()};
  }
  const ScopeExit fs_guard{[&fs] { glDeleteShader(*fs); }};

  const std::uint32_t handle{glCreateProgram()};
  if (handle == 0) {
    return std::unexpected{"Failed to create program"};
  }

  glAttachShader(handle, *vs);
  glAttachShader(handle, *fs);
  glLinkProgram(handle);
  glDetachShader(handle, *vs);
  glDetachShader(handle, *fs);

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

std::expected<Program, std::string> Program::CreateFromFiles(const std::filesystem::path &vs_path,
                                                             const std::filesystem::path &fs_path) {
  const auto vs_source{ReadFile(vs_path)};
  if (!vs_source.has_value()) {
    return std::unexpected{vs_source.error()};
  }

  const auto fs_source{ReadFile(fs_path)};
  if (!fs_source.has_value()) {
    return std::unexpected{fs_source.error()};
  }

  return Create(*vs_source, *fs_source);
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

void Program::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteProgram(handle_);
    handle_ = 0;
  }
}
