#include "shader.h"

#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

Shader::Shader(std::uint32_t handle) : handle_{handle} {}

Shader::~Shader() { Reset(); }

Shader::Shader(Shader &&other) noexcept : handle_{std::exchange(other.handle_, 0)} {}

Shader &Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    Reset();
    handle_ = std::exchange(other.handle_, 0);
  }
  return *this;
}

std::expected<Shader, Shader::Error> Shader::Create(Type type, std::string_view source) {
  const std::uint32_t handle{glCreateShader(type == Type::kVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER)};
  if (handle == 0) {
    spdlog::error("Failed to create {} shader", type == Type::kVertex ? "vertex" : "fragment");
    return std::unexpected{Error::kCreateFailed};
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
    spdlog::error("Failed to compile {} shader: {}", type == Type::kVertex ? "vertex" : "fragment", info_log);
    return std::unexpected{Error::kCompileFailed};
  }

  return Shader{handle};
}

std::expected<Shader, Shader::Error> Shader::CreateFromFile(Type type, const std::filesystem::path &path) {
  const std::ifstream file{path, std::ios::in | std::ios::binary};
  if (!file) {
    spdlog::error("Failed to open shader file: {}", path.string());
    return std::unexpected{Error::kFileOpenFailed};
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return Create(type, buffer.str());
}

void Shader::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteShader(handle_);
    handle_ = 0;
  }
}

std::uint32_t Shader::GetHandle() const noexcept { return handle_; }
