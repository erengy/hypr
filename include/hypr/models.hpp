#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include <hypp/header.hpp>
#include <hypp/method.hpp>
#include <hypp/request.hpp>
#include <hypp/response.hpp>
#include <hypp/status.hpp>
#include <hypp/uri.hpp>

namespace hypr {

struct Options {
  bool allow_redirects = true;
  int max_redirects = 30;
  std::chrono::seconds timeout{60};
  bool verify_certificate = true;
};

using Code = hypp::status::code_t;
using Url = hypp::Uri;

class Header : public hypp::Header {
public:
  Header(const std::initializer_list<Field>& fields)
      : hypp::Header{fields} {}
};

using Request = hypp::Request;

class Response : public hypp::Response {
public:
  std::string effective_url;
};

}  // namespace hypr
