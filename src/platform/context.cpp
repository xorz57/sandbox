#include "context.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <optional>
#include <utility>

Context::Context(SDL_GLContext handle)
    : m_handle{handle} {}

Context::~Context() {
  if (m_handle != nullptr) {
    SDL_GL_DestroyContext(m_handle);
  }
}

Context::Context(Context &&other) noexcept
    : m_handle{std::exchange(other.m_handle, nullptr)} {}

Context &Context::operator=(Context &&other) noexcept {
  if (this != &other) {
    if (m_handle != nullptr) {
      SDL_GL_DestroyContext(m_handle);
    }
    m_handle = std::exchange(other.m_handle, nullptr);
  }
  return *this;
}

std::optional<Context> Context::Create(SDL_Window *window) {
  SDL_GLContext context{SDL_GL_CreateContext(window)};
  if (context == nullptr) {
    spdlog::error("OpenGL: failed to create context: {}.", SDL_GetError());
    return std::nullopt;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
    spdlog::error("OpenGL: failed to initialize GLAD.");
    SDL_GL_DestroyContext(context);
    return std::nullopt;
  }

  return std::optional<Context>{Context{context}};
}
