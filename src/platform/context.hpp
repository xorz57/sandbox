#pragma once

#include <SDL3/SDL_video.h>

#include <optional>

#include "platform/window.hpp"
#include "types.hpp"

namespace platform {

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

  [[nodiscard]] static std::optional<Context> Create(const std::optional<Window> &window);

  static void SetVSync(VSyncMode mode);

  static void SwapBuffers(const std::optional<Window> &window);

  [[nodiscard]] SDL_GLContext GetContext() const;

private:
  explicit Context(SDL_GLContext context);

  void Reset();

  SDL_GLContext m_context;
};

} // namespace platform
