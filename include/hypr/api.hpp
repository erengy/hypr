#pragma once

#include <string_view>

#include <hypp/method.hpp>

#include <hypr/session.hpp>

namespace hypr {

template <typename... Ts>
Response request(const std::string_view method,
                 const std::string_view url,
                 const Ts&... args) {
  Session session;
  return session.request(method, url, args...);
}

template <typename... Ts>
Response get(const std::string_view url, const Ts&... args) {
  return request(hypp::method::kGet, url, args...);
}

}  // namespace hypr
