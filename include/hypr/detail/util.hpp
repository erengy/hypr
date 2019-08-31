#pragma once

#include <algorithm>
#include <string_view>

namespace hypr::detail {

struct CaseInsensitiveCompare {
  bool operator()(const std::string_view lhs,
                  const std::string_view rhs) const {
    return std::lexicographical_compare(
        lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(),
        [this](const char a, const char b) {
          return to_lower(a) < to_lower(b);
        });
  }

private:
  [[nodiscard]] constexpr char to_lower(const char c) const {
    return ('A' <= c && c <= 'Z') ? c + ('a' - 'A') : c;
  }
};

}  // namespace hypr::detail
