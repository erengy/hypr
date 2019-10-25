#pragma once

#include <string_view>

#include <hypr/detail/curl_interface.hpp>
#include <hypr/models.hpp>

namespace hypr {

class Session {
public:
  template <typename... Ts>
  Response request(const std::string_view method,
                   const std::string_view target,
                   const Ts&... args) {
    Request request;

    request.set_method(method);
    request.set_target(target);

    (set_option(args, request), ...);

    return send(request);
  }

  Response send(const Request& request) {
    return detail::curl::Interface::send(
        request, options, proxy, curl_session_);
  }

  Options options;
  Proxy proxy;

private:
  void set_option(const Headers& headers, Request& request) {
    request.set_headers(headers);
  }

  void set_option(const Query& params, Request& request) {
    request.set_query(params);
  }

  void set_option(const Body& body, Request& request) {
    request.set_body(body);
  }

  void set_option(const Proxy& proxy, Request&) {
    this->proxy = proxy;
  }

  detail::curl::Session curl_session_;
};

}  // namespace hypr
