#pragma once

#include <chrono>
#include <string>

#include <hypp/response.hpp>

namespace hypr::detail {

struct Response : public hypp::Response {
  std::string url;
};

}  // namespace hypr::detail
