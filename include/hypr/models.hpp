#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include <hypp/detail/uri.hpp>
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

using Body = std::string_view;
using Headers = detail::Headers;
using StatusCode = hypp::status::code_t;
using Url = hypp::Uri;

struct Options {
  bool allow_redirects = true;
  int max_redirects = 30;
  std::chrono::seconds timeout{60};
  bool verbose = false;
  bool verify_certificate = true;
};

class Params {
private:
  struct Param {
    std::string key;
    std::string value;
  };

  std::vector<Param> params_;

public:
  Params() = default;
  Params(const std::initializer_list<Param>& params) : params_{params} {}

  bool empty() const {
    return params_.empty();
  }

  std::string to_string() const {
    std::string str;
    for (const auto& [key, value] : params_) {
      if (!str.empty()) {
        str.push_back('&');
      }
      str += key + '=' + hypp::detail::uri::encode(value);
    }
    return str;
  }
};

class Request {
public:
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

  const Url url() const {
    return request_.start_line.target.uri;
  }
  bool set_url(const std::string_view url) {
    hypp::Parser parser{url};
    if (const auto expected = hypp::ParseRequestTarget(parser)) {
      request_.start_line.target = expected.value();
      return true;
    } else {
      return false;
    }
  }
  void set_url_params(const Params& params) {
    if (!params.empty()) {
      request_.start_line.target.uri.query = params.to_string();
    } else {
      request_.start_line.target.uri.query.reset();
    }
  }

  const auto& headers() const {
    return request_.header.fields;
  }
  void set_headers(const Headers& headers) {
    request_.header.fields.clear();
    for (const auto& [name, value] : headers) {
      request_.header.fields.push_back({name, value});
    }
  }

  const std::string_view body() const {
    return request_.body;
  }
  void set_body(const std::string_view body) {
    request_.body = body;
  }

private:
  hypp::Request request_;
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

  const detail::curl::Error& error() const {
    return response_.error;
  }

private:
  detail::Response response_;
};

}  // namespace hypr
