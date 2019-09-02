#pragma once

#include <chrono>
#include <string>

#include <hypp/response.hpp>

namespace hypr::detail {

struct Response : public hypp::Response {
  std::chrono::microseconds elapsed{0};
  std::string url;
};

}  // namespace hypr::detail
