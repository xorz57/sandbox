#pragma once

#include <utility>

template <typename Function> class ScopeExit final {
public:
  explicit ScopeExit(Function function) : function_{std::move(function)} {}

  ~ScopeExit() {
    if (active_) {
      function_();
    }
  }

  ScopeExit(const ScopeExit &) = delete;
  ScopeExit(ScopeExit &&) noexcept = delete;

  auto operator=(const ScopeExit &) -> ScopeExit & = delete;
  auto operator=(ScopeExit &&) noexcept -> ScopeExit & = delete;

  auto Release() noexcept -> void { active_ = false; }

private:
  bool active_{true};
  Function function_;
};
