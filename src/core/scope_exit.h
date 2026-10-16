#pragma once

#include <utility>

template <typename Function>
class ScopeExit final {
public:
  explicit ScopeExit(Function function) : function_{std::move(function)} {}

  ~ScopeExit() {
    if (active_) {
      function_();
    }
  }

  ScopeExit(const ScopeExit &) = delete;
  ScopeExit(ScopeExit &&) noexcept = delete;

  ScopeExit &operator=(const ScopeExit &) = delete;
  ScopeExit &operator=(ScopeExit &&) noexcept = delete;

  void Release() noexcept { active_ = false; }

private:
  bool active_{true};
  Function function_;
};
