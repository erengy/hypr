#pragma once

#include <chrono>
#include <map>
#include <string>

#include <hypp/response.hpp>

#include <hypr/detail/util.hpp>

namespace hypr::detail {

using Headers = std::map<std::string, std::string,
    detail::CaseInsensitiveCompare>;

struct Response : public hypp::Response {
  Headers headers;
  std::chrono::microseconds elapsed{0};
  std::string url;
};

}  // namespace hypr::detail
