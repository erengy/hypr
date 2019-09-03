#pragma once

#include <string_view>

#include <hypr/detail/curl_interface.hpp>
#include <hypr/models.hpp>

namespace hypr {

class Session {
public:
  template <typename... Ts>
  Response request(const std::string_view method,
                   const std::string_view url,
                   const Ts&... args) {
    Request request;

    request.set_method(method);
    request.set_url(url);

    (set_option(args, request), ...);

    interface_.init();
    return interface_.send(request, options);
  }

  Response send(const Request& request) {
    return interface_.send(request, options);
  }

  Options options;

private:
  void set_option(const Headers& headers, Request& request) {
    request.set_headers(headers);
  }

  void set_option(const Params& params, Request& request) {
    request.set_url_params(params);
  }

  void set_option(const Body& body, Request& request) {
    request.set_body(body);
  }

  detail::curl::Interface interface_;
};

}  // namespace hypr
