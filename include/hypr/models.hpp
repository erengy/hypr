#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include <hypp/parser/method.hpp>
#include <hypp/parser/request.hpp>
#include <hypp/header.hpp>
#include <hypp/method.hpp>
#include <hypp/request.hpp>
#include <hypp/response.hpp>
#include <hypp/status.hpp>
#include <hypp/uri.hpp>

#include <hypr/detail/models.hpp>
#include <hypr/detail/util.hpp>

namespace hypr {

using StatusCode = hypp::status::code_t;
using Url = hypp::Uri;

using Callbacks = detail::Callbacks;
using Error = detail::Error;
using Headers = detail::Headers;

struct Options {
  bool allow_redirects = true;
  bool certificate_revocation = true;
  int max_redirects = 30;
  std::chrono::seconds timeout{60};
  bool verbose = false;
  bool verify_certificate = true;
};

struct Proxy {
  std::string host;
  std::string username;
  std::string password;
};

class Query {
public:
  Query() = default;
  Query(const std::string_view str)
      : query_{hypp::detail::uri::encode(str)} {}
  Query(const std::initializer_list<detail::Param>& params)
      : query_{detail::to_string(params)} {}

  bool empty() const {
    return query_.empty();
  }

  std::string_view to_string() const {
    return query_;
  }

private:
  std::string query_;
};

class Body {
public:
  Body() = default;
  Body(const std::string_view str)
      : body_{str}, media_type_{"text/plain"} {}
  Body(const std::initializer_list<detail::Param>& params)
      : body_{detail::to_string(params)},
        media_type_{"application/x-www-form-urlencoded"} {}

  std::string_view media_type() const {
    return media_type_;
  }

  std::string_view to_string() const {
    return body_;
  }

private:
  std::string body_;
  std::string media_type_;
};

class Request {
public:
  Request() {
    request_.start_line.method = hypp::method::kGet;
  }

  const std::string_view method() const {
    return request_.start_line.method;
  }
  bool set_method(const std::string_view method) {
    hypp::Parser parser{method};
    if (const auto expected = hypp::ParseMethod(parser)) {
      request_.start_line.method =
          detail::to_upper_string(std::string{expected.value()});
      return true;
    } else {
      return false;
    }
  }

  const auto target() const {
    return request_.start_line.target;
  }
  bool set_target(const std::string_view target) {
    hypp::Parser parser{target};
    if (auto expected = hypp::ParseRequestTarget(parser)) {
      request_.start_line.target = std::move(expected.value());
      return true;
    } else {
      return false;
    }
  }

  void set_query(const Query& query) {
    if (!query.empty()) {
      request_.start_line.target.uri.query = query.to_string();
    } else {
      request_.start_line.target.uri.query.reset();
    }
  }

  std::string_view header(const std::string_view name) const {
    const auto it = request_.headers.find(std::string{name});
    return it != request_.headers.end() ? it->second : std::string_view{};
  }
  const Headers& headers() const {
    return request_.headers;
  }
  void add_header(const std::string_view name, const std::string_view value) {
    auto& header_value = request_.headers[std::string{name}];
    if (!header_value.empty()) {
      header_value.append(", ");
    }
    header_value.append(value);
  }
  void set_header(const std::string_view name, const std::string_view value) {
    request_.headers[std::string{name}] = value;
  }
  void set_headers(const Headers& headers) {
    request_.headers = headers;
  }

  const std::string_view body() const {
    return request_.body;
  }
  void set_body(const Body& body) {
    request_.body = body.to_string();
    if (header("content-type").empty() && !body.media_type().empty()) {
      set_header("Content-Type", body.media_type());
    }
  }

private:
  detail::Request request_;
};

class Response {
public:
  Response() = default;
  Response(detail::Response&& response) : response_{response} {}

  StatusCode status_code() const {
    return response_.start_line.code;
  }

  std::string_view reason_phrase() const {
    return hypp::status::to_phrase(response_.start_line.code);
  }

  std::string_view url() const {
    return response_.url;
  }

  std::string_view header(const std::string_view name) const {
    const auto it = response_.headers.find(std::string{name});
    return it != response_.headers.end() ? it->second : std::string_view{};
  }
  const Headers& headers() const {
    return response_.headers;
  }

  std::string_view body() const {
    return response_.body;
  }

  std::chrono::microseconds elapsed() const {
    return response_.elapsed;
  }

  const Error& error() const {
    return response_.error;
  }

private:
  detail::Response response_;
};

}  // namespace hypr
