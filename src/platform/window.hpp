#pragma once

#include <SDL3/SDL_video.h>

#include <optional>
#include <utility>

#include "types.hpp"

namespace platform {

class Window final {
public:
  ~Window();

  Window(const Window &) = delete;
  Window(Window &&other) noexcept;

  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&other) noexcept;

  [[nodiscard]] static std::optional<Window> Create(const char *title, i32 w, i32 h);

  void Show();

  void SetFullscreen(bool fullscreen);

  void ToggleFullscreen();

  void SwapBuffers();

  [[nodiscard]] std::pair<i32, i32> GetSize() const;

  [[nodiscard]] SDL_Window *GetWindow() const;

private:
  explicit Window(SDL_Window *window);

  void Reset();

  SDL_Window *m_window;
};

} // namespace platform
