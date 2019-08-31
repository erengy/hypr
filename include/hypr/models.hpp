#pragma once

#include <chrono>
#include <map>
#include <string>
#include <string_view>

#include <hypp/detail/uri.hpp>
#include <hypp/header.hpp>
#include <hypp/method.hpp>
#include <hypp/request.hpp>
#include <hypp/response.hpp>
#include <hypp/status.hpp>
#include <hypp/uri.hpp>

#include <hypr/detail/util.hpp>

namespace hypr {

struct Options {
  bool allow_redirects = true;
  int max_redirects = 30;
  std::chrono::seconds timeout{60};
  bool verify_certificate = true;
};

using Code = hypp::status::code_t;
using Url = hypp::Uri;

using Headers = std::multimap<std::string, std::string,
    detail::CaseInsensitiveCompare>;

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
  void set_method(const std::string_view method) {
    request_.start_line.method = method;
  }

  const Url url() const {
    return request_.start_line.target.uri;
  }
  void set_url(const Url& url) {
    request_.start_line.target.uri = url;
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
