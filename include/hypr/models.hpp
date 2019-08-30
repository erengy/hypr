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

class Request {
public:
  const std::string_view method() const {
    return request_.start_line.method;
  }
  void set_method(const std::string_view method) {
    request_.start_line.method = method;
  }

  const Url url() const {
    return request_.start_line.target.uri;
  }
  void set_url(const Url& url) {
    request_.start_line.target.uri = url;
  }

  const auto& headers() const {
    return request_.header.fields;
  }
  void set_headers(const Header& headers) {
    request_.header = headers;
  }
  void set_headers(const std::initializer_list<Header::Field>& headers) {
    request_.header = hypp::Header{headers};
  }

  const std::string_view content() const {
    return request_.body;
  }
  void set_content(const std::string_view content) {
    request_.body = content;
  }

private:
  hypp::Request request_;
};

class Response : public hypp::Response {
public:
  std::string effective_url;
};

}  // namespace hypr
