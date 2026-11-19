#include "window.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cassert>
#include <optional>
#include <utility>

#include "types.hpp"

Window::Window(SDL_Window *handle) : m_handle{handle} {}

Window::~Window() {
  if (m_handle != nullptr) {
    SDL_DestroyWindow(m_handle);
  }
}

Window::Window(Window &&other) noexcept
    : m_handle{std::exchange(other.m_handle, nullptr)} {}

Window &Window::operator=(Window &&other) noexcept {
  if (this != &other) {
    if (m_handle != nullptr) {
      SDL_DestroyWindow(m_handle);
    }
    m_handle = std::exchange(other.m_handle, nullptr);
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

  return std::optional<Window>{Window{window}};
}

void Window::SetVSync(Window::VSyncMode mode) {
  i32 interval{};
  switch (mode) {
    using enum Window::VSyncMode;
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
    spdlog::warn("OpenGL: failed to set vertical synchronization mode: {}.", SDL_GetError());
  }
}

void Window::Show() {
  SDL_SetWindowPosition(m_handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(m_handle);
}

void Window::SetFullscreen(bool fullscreen) {
  if (!SDL_SetWindowFullscreen(m_handle, fullscreen)) {
    spdlog::error("SDL: failed to set fullscreen mode: {}.", SDL_GetError());
  }
}

void Window::ToggleFullscreen() {
  const bool is_fullscreen{static_cast<bool>(SDL_GetWindowFlags(m_handle) & SDL_WINDOW_FULLSCREEN)};
  SetFullscreen(!is_fullscreen);
}

void Window::SwapBuffers() { SDL_GL_SwapWindow(m_handle); }

std::pair<i32, i32> Window::GetSize() const {
  i32 width{};
  i32 height{};
  SDL_GetWindowSizeInPixels(m_handle, &width, &height);
  return {width, height};
}

SDL_Window *Window::GetNativeHandle() const { return m_handle; }
