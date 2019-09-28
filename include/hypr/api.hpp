#pragma once

#include <string_view>

#include <hypp/method.hpp>

#include <hypr/session.hpp>

namespace hypr {

template <typename... Ts>
Response request(const std::string_view method,
                 const std::string_view target,
                 const Ts&... args) {
  Session session;
  return session.request(method, target, args...);
}

template <typename... Ts>
Response get(const std::string_view target, const Ts&... args) {
  return request(hypp::method::kGet, target, args...);
}

template <typename... Ts>
Response post(const std::string_view target, const Ts&... args) {
  return request(hypp::method::kPost, target, args...);
}

}  // namespace hypr
