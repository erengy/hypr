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

  void set_option(const Query& params, Request& request) {
    request.set_url_query(params);
  }

  void set_option(const Body& body, Request& request) {
    request.set_body(body);

    if (request.header("content-type").empty()) {
      switch (body.content_type()) {
        case Body::Type::application_x_www_form_urlencoded:
          request.set_header(
              "Content-Type", "application/x-www-form-urlencoded");
          break;
        case Body::Type::text_plain:
          request.set_header("Content-Type", "text/plain");
          break;
      }
    }
  }

  detail::curl::Interface interface_;
};

}  // namespace hypr
