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
    spdlog::error("Shader: failed to create shader object.");
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
    spdlog::error("Shader: compilation failed:\n{}", info_log);
    return std::nullopt;
  }

  shader_guard.Release();
  return std::optional<u32>{handle};
}

[[nodiscard]] std::optional<u32> LinkProgram(u32 vs_handle, u32 fs_handle) {
  const u32 handle{glCreateProgram()};
  if (handle == 0) {
    spdlog::error("Program: failed to create program object.");
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
    spdlog::error("Program: linking failed:\n{}", info_log);
    return std::nullopt;
  }

  program_guard.Release();
  return std::optional<u32>{handle};
}

[[nodiscard]] std::optional<std::string> ReadFile(const std::filesystem::path &path) {
  const std::ifstream file{path, std::ios::in | std::ios::binary};
  if (!file) {
    spdlog::error("Shader: failed to open file '{}'.", path.string());
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
    spdlog::error("Shader: failed to get last write time for '{}': {}.", path.string(), error.message());
    return std::nullopt;
  }
  return std::optional<std::filesystem::file_time_type>{last_write_time};
}

} // namespace

Program::~Program() { Reset(); }

Program::Program(Program &&other) noexcept
    : m_handle{std::exchange(other.m_handle, 0)},
      m_vs_path{std::move(other.m_vs_path)},
      m_fs_path{std::move(other.m_fs_path)},
      m_vs_attempted_write_time{other.m_vs_attempted_write_time},
      m_fs_attempted_write_time{other.m_fs_attempted_write_time},
      m_uniform_locations{std::move(other.m_uniform_locations)} {}

Program &Program::operator=(Program &&other) noexcept {
  if (this != &other) {
    Reset();
    m_handle = std::exchange(other.m_handle, 0);
    m_vs_path = std::move(other.m_vs_path);
    m_fs_path = std::move(other.m_fs_path);
    m_vs_attempted_write_time = other.m_vs_attempted_write_time;
    m_fs_attempted_write_time = other.m_fs_attempted_write_time;
    m_uniform_locations = std::move(other.m_uniform_locations);
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

  program->m_vs_path = vs_path;
  program->m_fs_path = fs_path;

  program->m_vs_attempted_write_time = *vs_write_time;
  program->m_fs_attempted_write_time = *fs_write_time;

  return program;
}

std::optional<Program> Program::Create(const std::filesystem::path &path) {
  return Create(std::filesystem::path{path} += ".vert.glsl", std::filesystem::path{path} += ".frag.glsl");
}

void Program::ReloadIfChanged() {
  if (m_vs_path.empty()) {
    return;
  }

  if (m_fs_path.empty()) {
    return;
  }

  const auto vs_write_time{GetLastWriteTime(m_vs_path)};
  if (!vs_write_time.has_value()) {
    return;
  }

  const auto fs_write_time{GetLastWriteTime(m_fs_path)};
  if (!fs_write_time.has_value()) {
    return;
  }

  bool has_changed{false};

  if (*vs_write_time != m_vs_attempted_write_time) {
    has_changed = true;
    m_vs_attempted_write_time = *vs_write_time;
    spdlog::info("Shader: '{}' changed.", m_vs_path.string());
  }

  if (*fs_write_time != m_fs_attempted_write_time) {
    has_changed = true;
    m_fs_attempted_write_time = *fs_write_time;
    spdlog::info("Shader: '{}' changed.", m_fs_path.string());
  }

  if (!has_changed) {
    return;
  }

  auto program{Create(m_vs_path, m_fs_path)};
  if (!program.has_value()) {
    spdlog::warn("Program: keeping the previous version after a failed reload.");
    return;
  }

  std::swap(m_handle, program->m_handle);

  m_uniform_locations.clear();
}

void Program::Use() const {
  assert(m_handle != 0);
  glUseProgram(m_handle);
}

void Program::SetUniform(const char *name, f32 value) const {
  assert(m_handle != 0);
  glProgramUniform1f(m_handle, GetUniformLocation(name), value);
}

void Program::SetUniform(const char *name, i32 value) const {
  assert(m_handle != 0);
  glProgramUniform1i(m_handle, GetUniformLocation(name), value);
}

void Program::SetUniform(const char *name, u32 value) const {
  assert(m_handle != 0);
  glProgramUniform1ui(m_handle, GetUniformLocation(name), value);
}

void Program::SetUniform(const char *name, const glm::vec2 &value) const {
  assert(m_handle != 0);
  glProgramUniform2fv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::vec3 &value) const {
  assert(m_handle != 0);
  glProgramUniform3fv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::vec4 &value) const {
  assert(m_handle != 0);
  glProgramUniform4fv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec2 &value) const {
  assert(m_handle != 0);
  glProgramUniform2iv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec3 &value) const {
  assert(m_handle != 0);
  glProgramUniform3iv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::ivec4 &value) const {
  assert(m_handle != 0);
  glProgramUniform4iv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec2 &value) const {
  assert(m_handle != 0);
  glProgramUniform2uiv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec3 &value) const {
  assert(m_handle != 0);
  glProgramUniform3uiv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::uvec4 &value) const {
  assert(m_handle != 0);
  glProgramUniform4uiv(m_handle, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat2 &value) const {
  assert(m_handle != 0);
  glProgramUniformMatrix2fv(m_handle, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat3 &value) const {
  assert(m_handle != 0);
  glProgramUniformMatrix3fv(m_handle, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetUniform(const char *name, const glm::mat4 &value) const {
  assert(m_handle != 0);
  glProgramUniformMatrix4fv(m_handle, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

Program::Program(u32 handle)
    : m_handle{handle} {}

i32 Program::GetUniformLocation(const char *name) const {
  const auto it{m_uniform_locations.find(name)};
  if (it != m_uniform_locations.end()) {
    return it->second;
  }

  const i32 location{glGetUniformLocation(m_handle, name)};
  if (location == -1) {
    spdlog::warn("Shader: uniform '{}' not found.", name);
  }
  m_uniform_locations.emplace(name, location);
  return location;
}

void Program::Reset() noexcept {
  if (m_handle != 0) {
    glDeleteProgram(m_handle);
    m_handle = 0;
  }
}
