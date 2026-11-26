#include "context.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <optional>
#include <utility>

#include "platform/window.hpp"
#include "scope_exit.hpp"
#include "types.hpp"

namespace platform {

Context::~Context() { Reset(); }

Context::Context(Context &&other) noexcept
    : m_context{std::exchange(other.m_context, nullptr)} {}

Context &Context::operator=(Context &&other) noexcept {
  if (this != &other) {
    Reset();
    m_context = std::exchange(other.m_context, nullptr);
  }

  return *this;
}

std::optional<Context> Context::Create(const std::optional<Window> &window) {
  if (!window.has_value()) {
    return std::nullopt;
  }

  SDL_GLContext context{SDL_GL_CreateContext(window->GetWindow())};
  if (context == nullptr) {
    spdlog::error("OpenGL: failed to create context: {}.", SDL_GetError());
    return std::nullopt;
  }
  ScopeExit context_guard{[context] { SDL_GL_DestroyContext(context); }};

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
    spdlog::error("OpenGL: failed to initialize GLAD.");
    SDL_GL_DestroyContext(context);
    return std::nullopt;
  }

  context_guard.Release();

  return std::optional<Context>{Context{context}};
}

void Context::SetVSync(VSyncMode mode) {
  i32 interval{};
  switch (mode) {
    using enum VSyncMode;
  case Disabled:
    interval = 0;
    break;
  case Enabled:
    interval = 1;
    break;
  case Adaptive:
    interval = -1;
    break;
  }

  if (!SDL_GL_SetSwapInterval(interval)) {
    spdlog::error("OpenGL: failed to set vertical synchronization mode: {}.", SDL_GetError());
  }
}

void Context::SwapBuffers(const std::optional<Window> &window) {
  if (!window.has_value()) {
    return;
  }

  SDL_GL_SwapWindow(window->GetWindow());
}

SDL_GLContext Context::GetContext() const { return m_context; }

Context::Context(SDL_GLContext context)
    : m_context{context} {}

void Context::Reset() {
  if (m_context == nullptr) {
    return;
  }

  SDL_GL_DestroyContext(m_context);
  m_context = nullptr;
}

} // namespace platform
