#pragma once

#include <SDL3/SDL_video.h>

#include <optional>
#include <utility>

#include "types.hpp"

class Window final {
public:
  enum class VSyncMode : u8 {
    Disabled,
    Enabled,
    Adaptive,
  };

  ~Window();

  Window(const Window &) = delete;
  Window(Window &&other) noexcept;

  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&other) noexcept;

  [[nodiscard]] static std::optional<Window> Create(const char *title, i32 width, i32 height);

  static void SetVSync(VSyncMode mode);

  void Show();

  void SetFullscreen(bool fullscreen);

  void ToggleFullscreen();

  void SwapBuffers();

  [[nodiscard]] std::pair<i32, i32> GetSize() const;

private:
  Window(SDL_Window *window, SDL_GLContext context);

  SDL_Window *m_window;
  SDL_GLContext m_context;
};
