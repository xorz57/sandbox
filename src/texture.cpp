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

namespace {

bool IsTextureSizeSupported(i32 w, i32 h) {
  i32 max_texture_size{};
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
  if (w > max_texture_size || h > max_texture_size) {
    spdlog::error("Texture: size {}x{} exceeds the maximum supported size of {}.", w, h, max_texture_size);
    return false;
  }
  return true;
}

std::optional<u32> CreateTextureFromPixels(i32 w, i32 h, const u8 *pixels) {
  if (!IsTextureSizeSupported(w, h)) {
    return std::nullopt;
  }

  u32 handle{};
  glCreateTextures(GL_TEXTURE_2D, 1, &handle);
  if (handle == 0) {
    spdlog::error("Texture: failed to create OpenGL texture.");
    return std::nullopt;
  }

  glTextureStorage2D(handle, 1, GL_RGBA8, w, h);
  glTextureSubImage2D(handle, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_REPEAT);

  return handle;
}

} // namespace

Texture::~Texture() { Reset(); }

Texture::Texture(Texture &&other) noexcept
    : m_handle{std::exchange(other.m_handle, 0)} {}

Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    Reset();
    m_handle = std::exchange(other.m_handle, 0);
  }
  return *this;
}

std::optional<Texture> Texture::Create(const std::filesystem::path &path) {
  i32 w{};
  i32 h{};
  stbi_uc *pixels{stbi_load(path.string().c_str(), &w, &h, nullptr, STBI_rgb_alpha)};
  if (pixels == nullptr) {
    const char *reason{stbi_failure_reason()};
    spdlog::error("Texture: failed to load '{}': {}.", path.string(), reason != nullptr ? reason : "unknown error");
    return std::nullopt;
  }
  ScopeExit pixels_guard{[pixels] { stbi_image_free(pixels); }};

  const std::optional<u32> handle{CreateTextureFromPixels(w, h, pixels)};
  if (!handle) {
    return std::nullopt;
  }

  return std::optional<Texture>{Texture{*handle}};
}

void Texture::Bind(u32 unit) const {
  assert(m_handle != 0);
  glBindTextureUnit(unit, m_handle);
}

Texture::Texture(u32 handle)
    : m_handle{handle} {}

void Texture::Reset() noexcept {
  if (m_handle != 0) {
    glDeleteTextures(1, &m_handle);
    m_handle = 0;
  }
}
