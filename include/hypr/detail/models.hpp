#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <hypp/detail/uri.hpp>
#include <hypp/header.hpp>
#include <hypp/response.hpp>

#include <hypr/detail/curl_error.hpp>
#include <hypr/detail/curl_session.hpp>
#include <hypr/detail/util.hpp>

namespace hypr::detail {

using Headers = std::map<std::string, std::string,
    detail::CaseInsensitiveCompare>;

using Param = std::pair<std::string_view, std::string_view>;

class Request : public hypp::Request {
public:
  Headers headers;
};

class Response : public hypp::Response {
public:
  Response() = default;
  Response(const CURLcode code) : error{code} {}

  curl::Error error;
  Headers headers;
  std::chrono::microseconds elapsed{0};
  std::string url;

  curl::Session* session = nullptr;
};

inline std::string to_string(const std::initializer_list<detail::Param>& params) {
  std::string str;
  for (const auto& [name, value] : params) {
    if (!str.empty()) {
      str.push_back('&');
    }
    str += std::string{name} + '=' + hypp::detail::uri::encode(value);
  }
  return str;
}

}  // namespace hypr::detail
