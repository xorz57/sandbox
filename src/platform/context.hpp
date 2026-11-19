#pragma once

#include <SDL3/SDL_video.h>

#include <optional>

#include "types.hpp"

class Context final {
public:
  enum class VSyncMode : u8 {
    Disabled,
    Enabled,
    Adaptive,
  };

  ~Context();

  Context(const Context &) = delete;
  Context(Context &&other) noexcept;

  Context &operator=(const Context &) = delete;
  Context &operator=(Context &&other) noexcept;

  [[nodiscard]] static std::optional<Context> Create(SDL_Window *window);

  static void SetVSync(VSyncMode mode);

  [[nodiscard]] SDL_GLContext GetContext() const;

private:
  explicit Context(SDL_GLContext context);

  SDL_GLContext m_context;
};
