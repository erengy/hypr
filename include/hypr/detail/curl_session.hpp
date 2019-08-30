#pragma once

#include <curl/curl.h>

#include <hypr/detail/curl_slist.hpp>

namespace hypr::detail::curl {

class Session {
public:
  ~Session() {
    cleanup();
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_init.html
  bool init() {
    if (!handle_) {
      handle_ = curl_easy_init();
    }
    return handle_ != nullptr;
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_cleanup.html
  void cleanup() {
    if (handle_) {
      curl_easy_cleanup(handle_);
      handle_ = nullptr;
    }
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_getinfo.html
  template <typename T>
  CURLcode getinfo(CURLINFO info, T& arg) const {
    return curl_easy_getinfo(handle_, info, &arg);
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_setopt.html
  template <typename T>
  CURLcode setopt(CURLoption option, const T& arg) const {
    return curl_easy_setopt(handle_, option, arg);
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_perform.html
  CURLcode perform() const {
    return curl_easy_perform(handle_);
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_reset.html
  void reset() const {
    curl_easy_reset(handle_);
  }

  Slist header_list;

private:
  CURL* handle_ = nullptr;
};

}  // namespace hypr::detail::curl
