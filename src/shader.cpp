#include "shader.hpp"

#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "scope_exit.hpp"

namespace {

[[nodiscard]] std::optional<std::uint32_t> CompileShader(std::uint32_t type, const char *source) {
  const std::uint32_t handle{glCreateShader(type)};
  if (handle == 0) {
    spdlog::error("Failed to create shader.");
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

} // namespace

Shader::~Shader() { Reset(); }

Shader::Shader(Shader &&other) noexcept
    : handle_{std::exchange(other.handle_, 0)} {}

std::optional<Shader> Shader::Create(std::uint32_t type, const char *source) {
  const auto handle{CompileShader(type, source)};
  if (!handle.has_value()) {
    return std::nullopt;
  }
  return std::optional<Shader>{Shader{*handle}};
}

Shader::Shader(std::uint32_t handle) noexcept
    : handle_{handle} {}

void Shader::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteShader(handle_);
    handle_ = 0;
  }
}
