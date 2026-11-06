#include "shader.hpp"

#include "scope_exit.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

namespace {

[[nodiscard]] std::optional<std::uint32_t> CompileShader(std::uint32_t type, const char *source) {
  const std::uint32_t handle{glCreateShader(type)};
  if (handle == 0) {
    spdlog::error("glCreateShader failed");
    return std::nullopt;
  }
  ScopeExit shader_guard{[handle] { glDeleteShader(handle); }};

  glShaderSource(handle, 1, &source, nullptr);
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
    spdlog::error("Failed to compile shader:\n{}", info_log);
    return std::nullopt;
  }

  shader_guard.Release();
  return handle;
}

[[nodiscard]] std::optional<std::uint32_t> LinkShader(std::uint32_t vs, std::uint32_t fs) {
  const std::uint32_t handle{glCreateProgram()};
  if (handle == 0) {
    spdlog::error("glCreateProgram failed");
    return std::nullopt;
  }
  ScopeExit program_guard{[handle] { glDeleteProgram(handle); }};

  glAttachShader(handle, vs);
  glAttachShader(handle, fs);
  glLinkProgram(handle);
  glDetachShader(handle, vs);
  glDetachShader(handle, fs);

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
    spdlog::error("Failed to link program:\n{}", info_log);
    return std::nullopt;
  }

  program_guard.Release();
  return handle;
}

[[nodiscard]] std::optional<std::string> ReadFile(const std::filesystem::path &path) {
  const std::ifstream file{path, std::ios::in | std::ios::binary};
  if (!file) {
    spdlog::error("Failed to open shader file {}", path.string());
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return buffer.str();
}

} // namespace

Shader::~Shader() { Reset(); }

Shader::Shader(Shader &&other) noexcept : handle_{std::exchange(other.handle_, 0)} {}

Shader &Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    Reset();
    handle_ = std::exchange(other.handle_, 0);
  }
  return *this;
}

std::optional<Shader> Shader::Create(const char *vs_source, const char *fs_source) {
  const auto vs{CompileShader(GL_VERTEX_SHADER, vs_source)};
  if (!vs.has_value()) {
    return std::nullopt;
  }
  const ScopeExit vs_guard{[&vs] { glDeleteShader(*vs); }};

  const auto fs{CompileShader(GL_FRAGMENT_SHADER, fs_source)};
  if (!fs.has_value()) {
    return std::nullopt;
  }
  const ScopeExit fs_guard{[&fs] { glDeleteShader(*fs); }};

  const auto handle{LinkShader(*vs, *fs)};
  if (!handle.has_value()) {
    return std::nullopt;
  }

  return Shader{*handle};
}

std::optional<Shader> Shader::CreateFromFiles(const std::filesystem::path &vs_path,
                                              const std::filesystem::path &fs_path) {
  const auto vs_source{ReadFile(vs_path)};
  if (!vs_source.has_value()) {
    return std::nullopt;
  }

  const auto fs_source{ReadFile(fs_path)};
  if (!fs_source.has_value()) {
    return std::nullopt;
  }

  return Create(vs_source->c_str(), fs_source->c_str());
}

std::optional<Shader> Shader::CreateFromFile(const std::filesystem::path &path) {
  return CreateFromFiles(std::filesystem::path{path} += ".vert.glsl", std::filesystem::path{path} += ".frag.glsl");
}

void Shader::Use() const {
  assert(handle_ != 0);
  glUseProgram(handle_);
}

void Shader::SetUniform(const char *name, float value) const {
  assert(handle_ != 0);
  glProgramUniform1f(handle_, glGetUniformLocation(handle_, name), value);
}

void Shader::SetUniform(const char *name, std::int32_t value) const {
  assert(handle_ != 0);
  glProgramUniform1i(handle_, glGetUniformLocation(handle_, name), value);
}

void Shader::SetUniform(const char *name, std::uint32_t value) const {
  assert(handle_ != 0);
  glProgramUniform1ui(handle_, glGetUniformLocation(handle_, name), value);
}

void Shader::SetUniform(const char *name, const glm::vec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2fv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::vec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3fv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::vec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4fv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::ivec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2iv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::ivec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3iv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::ivec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4iv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::uvec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2uiv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::uvec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3uiv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::uvec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4uiv(handle_, glGetUniformLocation(handle_, name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::mat2 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix2fv(handle_, glGetUniformLocation(handle_, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::mat3 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix3fv(handle_, glGetUniformLocation(handle_, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::mat4 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix4fv(handle_, glGetUniformLocation(handle_, name), 1, GL_FALSE, glm::value_ptr(value));
}

Shader::Shader(std::uint32_t handle) : handle_{handle} {}

void Shader::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteProgram(handle_);
    handle_ = 0;
  }
}
