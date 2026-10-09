#pragma once

#include <utility>

template <typename Function> class ScopeExit final {
public:
  ScopeExit(Function &&function) : m_Function(std::move(function)) {}
  ~ScopeExit() { m_Function(); }

  ScopeExit(const ScopeExit &) = delete;
  ScopeExit &operator=(const ScopeExit &) = delete;

  ScopeExit(ScopeExit &&) = delete;
  ScopeExit &operator=(ScopeExit &&) = delete;

private:
  Function m_Function;
};