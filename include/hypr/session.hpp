#pragma once

#include <string_view>

#include <hypp/parser/method.hpp>
#include <hypp/parser/request.hpp>

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

    {
      hypp::Parser parser{method};
      if (const auto expected = hypp::ParseMethod(parser)) {
        request.set_method(expected.value());
      } else {
        // @TODO: Error: Invalid method
      }
    }

    {
      hypp::Parser parser{url};
      if (const auto expected = hypp::ParseRequestTarget(parser)) {
        request.set_url(expected.value().uri);
      } else {
        // @TODO: Error: Invalid URL
      }
    }

    (set_option(args, request), ...);

    interface_.init();
    return interface_.send(request, options);
  }

  Options options;

private:
  void set_option(const Header& header, Request& request) {
    request.set_headers(header);
  }

  detail::curl::Interface interface_;
};

}  // namespace hypr
