#include "texture.hpp"

#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <cassert>
#include <filesystem>
#include <optional>
#include <utility>

#include "scope_exit.hpp"
#include "types.hpp"

Texture::~Texture() {
  if (m_handle != 0) {
    glDeleteTextures(1, &m_handle);
  }
}

Texture::Texture(Texture &&other) noexcept
    : m_handle{std::exchange(other.m_handle, 0)} {}

Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    if (m_handle != 0) {
      glDeleteTextures(1, &m_handle);
    }
    m_handle = std::exchange(other.m_handle, 0);
  }
  return *this;
}

std::optional<Texture> Texture::Create(const std::filesystem::path &path) {
  i32 w{};
  i32 h{};
  i32 n{};
  stbi_set_flip_vertically_on_load(1);
  stbi_uc *pixels{stbi_load(path.string().c_str(), &w, &h, &n, 0)};
  if (pixels == nullptr) {
    spdlog::error("Texture: failed to load '{}': {}.", path.string(), stbi_failure_reason());
    return std::nullopt;
  }
  ScopeExit pixels_guard{[pixels] { stbi_image_free(pixels); }};

  i32 max_texture_size{};
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
  if (w > max_texture_size || h > max_texture_size) {
    spdlog::error("Texture: size {}x{} exceeds the maximum supported size of {}.", w, h, max_texture_size);
    return std::nullopt;
  }

  u32 internal_format{};
  u32 format{};

  switch (n) {
  case 3:
    internal_format = GL_RGB8;
    format = GL_RGB;
    break;
  case 4:
    internal_format = GL_RGBA8;
    format = GL_RGBA;
    break;
  default:
    spdlog::error("Texture: unsupported number of channels {} in '{}'.", n, path.string());
    return std::nullopt;
  }

  u32 handle{};
  glCreateTextures(GL_TEXTURE_2D, 1, &handle);
  if (handle == 0) {
    spdlog::error("Texture: failed to create OpenGL texture.");
    return std::nullopt;
  }

  glTextureStorage2D(handle, 1, internal_format, w, h);

  glTextureSubImage2D(handle, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, pixels);

  glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_REPEAT);

  return std::optional<Texture>{Texture{handle}};
}

void Texture::Bind(u32 unit) const {
  assert(m_handle != 0);
  glBindTextureUnit(unit, m_handle);
}

void Texture::Unbind(u32 unit) { glBindTextureUnit(unit, 0); }

Texture::Texture(u32 handle)
    : m_handle{handle} {}
