#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

template <typename F>
  requires std::invocable<F &>
class [[nodiscard]] ScopeExit {
public:
  explicit ScopeExit(F f) noexcept(std::is_nothrow_move_constructible_v<F>) : f_{std::move(f)} {}

  ~ScopeExit() {
    if (active_) {
      f_();
    }
  }

  ScopeExit(const ScopeExit &) = delete;
  ScopeExit(ScopeExit &&) = delete;

  auto operator=(const ScopeExit &) -> ScopeExit & = delete;
  auto operator=(ScopeExit &&) -> ScopeExit & = delete;

  auto Release() noexcept -> void { active_ = false; }

private:
  F f_;
  bool active_{true};
};
