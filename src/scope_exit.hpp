#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

template <typename F>
  requires std::invocable<F &>
class [[nodiscard]] ScopeExit {
public:
  explicit ScopeExit(F f) noexcept(std::is_nothrow_move_constructible_v<F>)
      : m_f{std::move(f)} {}

  ~ScopeExit() {
    if (m_active) {
      m_f();
    }
  }

  ScopeExit(const ScopeExit &) = delete;
  ScopeExit(ScopeExit &&) = delete;

  auto operator=(const ScopeExit &) -> ScopeExit & = delete;
  auto operator=(ScopeExit &&) -> ScopeExit & = delete;

  auto Release() noexcept -> void { m_active = false; }

private:
  F m_f;
  bool m_active{true};
};
