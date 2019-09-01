#pragma once

#include <algorithm>
#include <string>
#include <string_view>

namespace hypr::detail {

[[nodiscard]] constexpr char to_lower(const char c) {
  return ('A' <= c && c <= 'Z') ? c + ('a' - 'A') : c;
}

[[nodiscard]] constexpr char to_upper(const char c) {
  return ('a' <= c && c <= 'z') ? c - ('a' - 'A') : c;
}

[[nodiscard]] std::string to_upper_string(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), to_upper);
  return std::move(str);
}

struct CaseInsensitiveCompare {
  bool operator()(const std::string_view lhs,
                  const std::string_view rhs) const {
    return std::lexicographical_compare(
        lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(),
        [](const char a, const char b) {
          return to_lower(a) < to_lower(b);
        });
  }
};

}  // namespace hypr::detail
