#include "program.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>

#include "scope_exit.hpp"
#include "types.hpp"

namespace {

[[nodiscard]] std::optional<u32> CompileShader(u32 type, const char *source) {
  const u32 handle{glCreateShader(type)};
  if (handle == 0) {
    spdlog::error("Failed to create shader.");
    return std::nullopt;
  }
  ScopeExit shader_guard{[handle] { glDeleteShader(handle); }};

  glShaderSource(handle, 1, &source, nullptr);
  glCompileShader(handle);

  i32 success{};
  glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
  if (success == 0) {
    i32 info_log_length{};
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &info_log_length);
    std::string info_log(static_cast<usize>(info_log_length), '\0');
    glGetShaderInfoLog(handle, info_log_length, nullptr, info_log.data());
    if (info_log_length > 0) {
      info_log.resize(static_cast<usize>(info_log_length) - 1);
    }
    spdlog::error("Failed to compile shader:\n{}", info_log);
    return std::nullopt;
  }

  shader_guard.Release();
  return std::optional<u32>{handle};
}

[[nodiscard]] std::optional<u32> LinkProgram(u32 vs_handle, u32 fs_handle) {
  const u32 handle{glCreateProgram()};
  if (handle == 0) {
    spdlog::error("Failed to create program.");
    return std::nullopt;
  }
  ScopeExit program_guard{[handle] { glDeleteProgram(handle); }};

  glAttachShader(handle, vs_handle);
  glAttachShader(handle, fs_handle);

  glLinkProgram(handle);

  glDetachShader(handle, vs_handle);
  glDetachShader(handle, fs_handle);

  i32 success{};
  glGetProgramiv(handle, GL_LINK_STATUS, &success);
  if (success == 0) {
    i32 info_log_length{};
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &info_log_length);
    std::string info_log(static_cast<usize>(info_log_length), '\0');
    glGetProgramInfoLog(handle, info_log_length, nullptr, info_log.data());
    if (info_log_length > 0) {
      info_log.resize(static_cast<usize>(info_log_length) - 1);
    }
    spdlog::error("Failed to link program:\n{}", info_log);
    return std::nullopt;
  }

  program_guard.Release();
  return std::optional<u32>{handle};
}

[[nodiscard]] std::optional<std::string> ReadFile(const std::filesystem::path &path) {
  const std::ifstream file{path, std::ios::in | std::ios::binary};
  if (!file) {
    spdlog::error("Failed to open file '{}'.", path.string());
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return std::optional<std::string>{buffer.str()};
}

[[nodiscard]] std::optional<std::filesystem::file_time_type> GetLastWriteTime(const std::filesystem::path &path) {
  std::error_code error;
  const std::filesystem::file_time_type last_write_time{std::filesystem::last_write_time(path, error)};
  if (error) {
    spdlog::error("Failed to get last write time for shader file '{}': {}", path.string(), error.message());
    return std::nullopt;
  }
  return std::optional<std::filesystem::file_time_type>{last_write_time};
}

} // namespace

Program::~Program() { Reset(); }

Program::Program(Program &&other) noexcept
    : handle_{std::exchange(other.handle_, 0)},
      vs_path_{std::move(other.vs_path_)},
      fs_path_{std::move(other.fs_path_)},
      vs_attempted_write_time_{other.vs_attempted_write_time_},
      fs_attempted_write_time_{other.fs_attempted_write_time_},
      uniform_locations_{std::move(other.uniform_locations_)} {}

Program &Program::operator=(Program &&other) noexcept {
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

std::optional<Program> Program::Create(const char *vs_source, const char *fs_source) {
  const auto vs_handle{CompileShader(GL_VERTEX_SHADER, vs_source)};
  if (!vs_handle.has_value()) {
    return std::nullopt;
  }
  ScopeExit vs_guard{[&vs_handle] { glDeleteShader(*vs_handle); }};

  const auto fs_handle{CompileShader(GL_FRAGMENT_SHADER, fs_source)};
  if (!fs_handle.has_value()) {
    return std::nullopt;
  }
  ScopeExit fs_guard{[&fs_handle] { glDeleteShader(*fs_handle); }};

  const auto handle{LinkProgram(*vs_handle, *fs_handle)};
  if (!handle.has_value()) {
    return std::nullopt;
  }
  return std::optional<Program>{Program{*handle}};
}

std::optional<Program> Program::Create(const std::filesystem::path &vs_path, const std::filesystem::path &fs_path) {
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

  auto program{Create(vs_source->c_str(), fs_source->c_str())};
  if (!program.has_value()) {
    return std::nullopt;
  }

  program->vs_path_ = vs_path;
  program->fs_path_ = fs_path;

  program->vs_attempted_write_time_ = *vs_write_time;
  program->fs_attempted_write_time_ = *fs_write_time;

  return program;
}

std::optional<Program> Program::Create(const std::filesystem::path &path) {
  return Create(std::filesystem::path{path} += ".vert.glsl", std::filesystem::path{path} += ".frag.glsl");
}

void Program::ReloadIfChanged() {
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

  auto program{Create(vs_path_, fs_path_)};
  if (!program.has_value()) {
    spdlog::warn("Keeping the previous program after a failed reload.");
    return;
  }

  std::swap(handle_, program->handle_);

  uniform_locations_.clear();
}

void Program::Use() const {
  assert(handle_ != 0);
  glUseProgram(handle_);
}

void Program::SetUniform(const char *name, f32 value) const {
  assert(handle_ != 0);
  glProgramUniform1f(handle_, GetUniformLocation(name), value);
}

void Program::SetUniform(const char *name, i32 value) const {
  assert(handle_ != 0);
  glProgramUniform1i(handle_, GetUniformLocation(name), value);
}

void Program::SetUniform(const char *name, u32 value) const {
  assert(handle_ != 0);
  glProgramUniform1ui(handle_, GetUniformLocation(name), value);
}

void Program::SetUniform(const char *name, const glm::vec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2fv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::vec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3fv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::vec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4fv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2iv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3iv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4iv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec2 &value) const {
  assert(handle_ != 0);
  glProgramUniform2uiv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec3 &value) const {
  assert(handle_ != 0);
  glProgramUniform3uiv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec4 &value) const {
  assert(handle_ != 0);
  glProgramUniform4uiv(handle_, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat2 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix2fv(handle_, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat3 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix3fv(handle_, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat4 &value) const {
  assert(handle_ != 0);
  glProgramUniformMatrix4fv(handle_, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

Program::Program(u32 handle)
    : handle_{handle} {}

i32 Program::GetUniformLocation(const char *name) const {
  const auto it{uniform_locations_.find(name)};
  if (it != uniform_locations_.end()) {
    return it->second;
  }

  const i32 location{glGetUniformLocation(handle_, name)};
  if (location == -1) {
    spdlog::warn("Shader uniform '{}' not found.", name);
  }
  uniform_locations_.emplace(name, location);
  return location;
}

void Program::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteProgram(handle_);
    handle_ = 0;
  }
}
