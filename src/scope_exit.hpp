#pragma once

#include <utility>

template <typename Function> class ScopeExit final {
public:
  ScopeExit(Function &&function) : function_(std::move(function)) {}
  ~ScopeExit() { function_(); }

  ScopeExit(const ScopeExit &) = delete;
  ScopeExit &operator=(const ScopeExit &) = delete;

  ScopeExit(ScopeExit &&) = delete;
  ScopeExit &operator=(ScopeExit &&) = delete;

private:
  Function function_;
};
