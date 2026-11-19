#include "window.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <array>
#include <optional>
#include <utility>

#include "types.hpp"

Window::Window(SDL_Window *window, SDL_GLContext context)
    : m_window{window},
      m_context{context} {}

Window::~Window() {
  if (m_context != nullptr) {
    SDL_GL_DestroyContext(m_context);
  }
  if (m_window != nullptr) {
    SDL_DestroyWindow(m_window);
  }
}

Window::Window(Window &&other) noexcept
    : m_window{std::exchange(other.m_window, nullptr)},
      m_context{std::exchange(other.m_context, nullptr)} {}

Window &Window::operator=(Window &&other) noexcept {
  if (this != &other) {
    if (m_context != nullptr) {
      SDL_GL_DestroyContext(m_context);
    }
    if (m_window != nullptr) {
      SDL_DestroyWindow(m_window);
    }
    m_window = std::exchange(other.m_window, nullptr);
    m_context = std::exchange(other.m_context, nullptr);
  }
  return *this;
}

std::optional<Window> Window::Create(const char *title, i32 width, i32 height) {
  constexpr std::array<std::pair<SDL_GLAttr, i32>, 8> gl_attribute_settings{{
      {SDL_GL_CONTEXT_MAJOR_VERSION, 4},
      {SDL_GL_CONTEXT_MINOR_VERSION, 6},
      {SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE},
      {SDL_GL_DOUBLEBUFFER, 1},
      {SDL_GL_DEPTH_SIZE, 24},
      {SDL_GL_STENCIL_SIZE, 8},
      {SDL_GL_MULTISAMPLEBUFFERS, 1},
      {SDL_GL_MULTISAMPLESAMPLES, 4},
  }};

  for (const auto &[attribute, value] : gl_attribute_settings) {
    if (!SDL_GL_SetAttribute(attribute, value)) {
      spdlog::error("SDL: failed to set OpenGL attribute {}: {}.", static_cast<i32>(attribute), SDL_GetError());
      return std::nullopt;
    }
  }

  const SDL_WindowFlags flags{SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN};
  SDL_Window *window{SDL_CreateWindow(title, width, height, flags)};
  if (window == nullptr) {
    spdlog::error("SDL: failed to create window: {}.", SDL_GetError());
    return std::nullopt;
  }

  SDL_GLContext context{SDL_GL_CreateContext(window)};
  if (context == nullptr) {
    spdlog::error("OpenGL: failed to create context: {}.", SDL_GetError());
    SDL_DestroyWindow(window);
    return std::nullopt;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
    spdlog::error("OpenGL: failed to initialize GLAD.");
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    return std::nullopt;
  }

  if (!SDL_GL_SetSwapInterval(1)) {
    spdlog::warn("OpenGL: failed to enable vertical synchronization: {}.", SDL_GetError());
  }

  return Window{window, context};
}

void Window::Show() {
  SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(m_window);
}

void Window::ToggleFullscreen() {
  const bool is_fullscreen{static_cast<bool>(SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN)};
  SDL_SetWindowFullscreen(m_window, !is_fullscreen);
}

void Window::SwapBuffers() { SDL_GL_SwapWindow(m_window); }

std::pair<i32, i32> Window::GetSize() const {
  i32 w{};
  i32 h{};
  SDL_GetWindowSizeInPixels(m_window, &w, &h);
  return {w, h};
}
