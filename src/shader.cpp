#include "shader.hpp"

#include <glad/gl.h>

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

Shader::~Shader() { Reset(); }

Shader::Shader(Shader &&other) noexcept : handle_{std::exchange(other.handle_, 0)} {}

Shader &Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    Reset();
    handle_ = std::exchange(other.handle_, 0);
  }
  return *this;
}

std::expected<Shader, std::string> Shader::Create(Type type, std::string_view source) {
  const std::uint32_t handle{glCreateShader((type == Type::kVertex) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER)};
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

  return Shader{handle};
}

std::expected<Shader, std::string> Shader::CreateFromFile(Type type, const std::filesystem::path &path) {
  const std::ifstream file{path, std::ios::in | std::ios::binary};
  if (!file) {
    return std::unexpected{std::format("Failed to open shader file '{}'", path.string())};
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return Create(type, buffer.str());
}

std::uint32_t Shader::GetHandle() const noexcept { return handle_; }

Shader::Shader(std::uint32_t handle) : handle_{handle} {}

void Shader::Reset() noexcept {
  if (handle_ != 0) {
    glDeleteShader(handle_);
    handle_ = 0;
  }
}
