#pragma once

namespace mercy {

class NonCopyable {
public:
  NonCopyable() = default;
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable(NonCopyable &&) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;
  NonCopyable &operator=(NonCopyable &&) = delete;
  ~NonCopyable() = default;
};

} // namespace mercy
