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
#include <system_error>
#include <utility>

namespace {

[[nodiscard]] std::optional<std::uint32_t> CompileShader(std::uint32_t type, const char *source) {
  const std::uint32_t handle{glCreateShader(type)};
  if (handle == 0) {
    spdlog::error("Failed to create OpenGL shader.");
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

  return std::optional<std::uint32_t>{handle};
}

[[nodiscard]] std::optional<std::uint32_t> LinkProgram(std::uint32_t vs, std::uint32_t fs) {
  const std::uint32_t handle{glCreateProgram()};
  if (handle == 0) {
    spdlog::error("Failed to create OpenGL program.");
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
    spdlog::error("Failed to link shader program:\n{}", info_log);
    return std::nullopt;
  }

  program_guard.Release();

  return std::optional<std::uint32_t>{handle};
}

[[nodiscard]] std::optional<std::string> ReadFile(const std::filesystem::path &path) {
  const std::ifstream file{path, std::ios::in | std::ios::binary};
  if (!file) {
    spdlog::error("Failed to open shader file '{}'.", path.string());
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return std::optional<std::string>{buffer.str()};
}

[[nodiscard]] std::optional<std::filesystem::file_time_type> GetLastWriteTime(const std::filesystem::path &path) {
  std::error_code error;
  const std::filesystem::file_time_type write_time{std::filesystem::last_write_time(path, error)};
  if (error) {
    spdlog::error("Failed to get last write time for shader file '{}': {}", path.string(), error.message());
    return std::nullopt;
  }
  return std::optional<std::filesystem::file_time_type>{write_time};
}

} // namespace

Shader::~Shader() { Reset(); }

Shader::Shader(Shader &&other) noexcept
    : handle_{std::exchange(other.handle_, 0)},
      vs_path_{std::move(other.vs_path_)},
      fs_path_{std::move(other.fs_path_)},
      vs_attempted_write_time_{other.vs_attempted_write_time_},
      fs_attempted_write_time_{other.fs_attempted_write_time_},
      uniform_locations_{std::move(other.uniform_locations_)} {}

Shader &Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    Reset();
    handle_ = std::exchange(other.handle_, 0);
    vs_path_ = std::move(other.vs_path_);
    fs_path_ = std::move(other.fs_path_);
    vs_attempted_write_time_ = other.vs_attempted_write_time_;
    fs_attempted_write_time_ = other.fs_attempted_write_time_;
    uniform_locations_ = std::move(other.uniform_locations_);
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

  const auto handle{LinkProgram(*vs, *fs)};
  if (!handle.has_value()) {
    return std::nullopt;
  }

  return std::optional<Shader>{Shader{*handle}};
}

std::optional<Shader> Shader::Create(const std::filesystem::path &vs_path, const std::filesystem::path &fs_path) {
  const auto vs_write_time{GetLastWriteTime(vs_path)};
  if (!vs_write_time.has_value()) {
    return std::nullopt;
  }

  const auto fs_write_time{GetLastWriteTime(fs_path)};
  if (!fs_write_time.has_value()) {
    return std::nullopt;
  }

  const auto vs_source{ReadFile(vs_path)};
  if (!vs_source.has_value()) {
    return std::nullopt;
  }

  const auto fs_source{ReadFile(fs_path)};
  if (!fs_source.has_value()) {
    return std::nullopt;
  }

  auto shader{Create(vs_source->c_str(), fs_source->c_str())};
  if (!shader.has_value()) {
    return std::nullopt;
  }

  shader->vs_path_ = vs_path;
  shader->fs_path_ = fs_path;

  shader->vs_attempted_write_time_ = *vs_write_time;
  shader->fs_attempted_write_time_ = *fs_write_time;

  return shader;
}

std::optional<Shader> Shader::Create(const std::filesystem::path &path) {
  return Create(std::filesystem::path{path} += ".vert.glsl", std::filesystem::path{path} += ".frag.glsl");
}

void Shader::ReloadIfChanged() {
  if (vs_path_.empty()) {
    return;
  }

  if (fs_path_.empty()) {
    return;
  }

  const auto vs_write_time{GetLastWriteTime(vs_path_)};
  if (!vs_write_time.has_value()) {
    return;
  }

  const auto fs_write_time{GetLastWriteTime(fs_path_)};
  if (!fs_write_time.has_value()) {
    return;
  }

  bool has_changed{false};

  if (*vs_write_time != vs_attempted_write_time_) {
    has_changed = true;
    vs_attempted_write_time_ = *vs_write_time;
    spdlog::info("Shader '{}' changed.", vs_path_.string());
  }

  if (*fs_write_time != fs_attempted_write_time_) {
    has_changed = true;
    fs_attempted_write_time_ = *fs_write_time;
    spdlog::info("Shader '{}' changed.", fs_path_.string());
  }

  if (!has_changed) {
    return;
  }

  auto shader{Create(vs_path_, fs_path_)};
  if (!shader.has_value()) {
    spdlog::warn("Keeping the previous shader program after a failed reload.");
    return;
  }

  std::swap(handle_, shader->handle_);

  uniform_locations_.clear();
}

void Shader::Use() const {
  assert(handle_ != 0);
  glUseProgram(handle_);
}

void Shader::SetUniform(const char *name, float value) const {
  assert(handle_ != 0);
  glProgramUniform1f(handle_, GetUniformLocation(name), value);
}

void Shader::SetUniform(const char *name, std::int32_t value) const {
  assert(handle_ != 0);
  glProgramUniform1i(handle_, GetUniformLocation(name), value);
}

void Shader::SetUniform(const char *name, std::uint32_t value) const {
  assert(handle_ != 0);
  glProgramUniform1ui(handle_, GetUniformLocation(name), value);
}

void Shader::SetUniform(const char *name, const glm::vec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2fv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::vec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3fv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::vec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4fv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::ivec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2iv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::ivec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3iv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::ivec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4iv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::uvec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2uiv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::uvec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3uiv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::uvec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4uiv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::mat2 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix2fv(handle_, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::mat3 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix3fv(handle_, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, const glm::mat4 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix4fv(handle_, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

Shader::Shader(std::uint32_t handle)
    : handle_{handle} {}

std::int32_t Shader::GetUniformLocation(const char *name) const {
  const auto it{uniform_locations_.find(name)};
  if (it != uniform_locations_.end()) {
    return it->second;
  }

  const std::int32_t location{glGetUniformLocation(handle_, name)};
  if (location == -1) {
    spdlog::warn("Shader uniform '{}' not found.", name);
  }
  uniform_locations_.emplace(name, location);
  return location;
}

void Shader::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteProgram(handle_);
    handle_ = 0;
  }
}
