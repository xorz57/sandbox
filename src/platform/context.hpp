#pragma once

#include <SDL3/SDL_video.h>

#include <optional>

class Context final {
public:
  ~Context();

  Context(const Context &) = delete;
  Context(Context &&other) noexcept;

  Context &operator=(const Context &) = delete;
  Context &operator=(Context &&other) noexcept;

  [[nodiscard]] static std::optional<Context> Create(SDL_Window *window);

private:
  explicit Context(SDL_GLContext context);

  SDL_GLContext m_context;
};
