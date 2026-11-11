#pragma once

#include <cstdint>
#include <optional>

class Shader final {
public:
  ~Shader();

  Shader(const Shader &) = delete;
  Shader(Shader &&other) noexcept;

  Shader &operator=(const Shader &) = delete;
  Shader &operator=(Shader &&) = delete;

  [[nodiscard]] static std::optional<Shader> Create(std::uint32_t type, const char *source);

  [[nodiscard]] auto GetHandle() const noexcept -> std::uint32_t { return handle_; }

private:
  explicit Shader(std::uint32_t handle) noexcept;

  void Reset() noexcept;

  std::uint32_t handle_;
};
