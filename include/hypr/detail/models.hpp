#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <hypp/header.hpp>
#include <hypp/response.hpp>

#include <hypr/detail/curl_error.hpp>
#include <hypr/detail/curl_session.hpp>
#include <hypr/detail/util.hpp>

namespace hypr::detail {

using Error = curl::Error;

using Headers = std::map<std::string, std::string,
    detail::CaseInsensitiveCompare>;

struct Transfer {
  int64_t current = 0;
  int64_t total = 0;
};

struct Callbacks {
  std::function<void(const curl_infotype, std::string_view)> debug;
  std::function<bool(const Transfer&)> transfer;
};

class Request : public hypp::Request {
public:
  Headers headers;
};

class Response : public hypp::Response {
public:
  Response() = default;
  Response(const CURLcode code) : error{code} {}

  Callbacks callbacks;
  Error error;
  Headers headers;
  Transfer transfer;
  std::chrono::microseconds elapsed{0};
  std::string url;

  curl::Session* session = nullptr;
};

}  // namespace hypr::detail
