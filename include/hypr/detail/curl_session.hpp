#pragma once

#include <memory>

#include <curl/curl.h>

#include <hypr/detail/curl_slist.hpp>

namespace hypr::detail::curl {

class Session {
public:
  // https://curl.haxx.se/libcurl/c/curl_easy_init.html
  bool init() {
    if (!handle_) {
      handle_.reset(curl_easy_init());
    }
    return handle_ != nullptr;
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_cleanup.html
  void cleanup() {
    handle_.reset();
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_getinfo.html
  template <typename T>
  CURLcode getinfo(CURLINFO info, T& arg) const {
    return curl_easy_getinfo(handle_.get(), info, &arg);
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_setopt.html
  template <typename T>
  CURLcode setopt(CURLoption option, const T& arg) const {
    return curl_easy_setopt(handle_.get(), option, arg);
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_perform.html
  CURLcode perform() const {
    return curl_easy_perform(handle_.get());
  }

  // https://curl.haxx.se/libcurl/c/curl_easy_reset.html
  void reset() const {
    curl_easy_reset(handle_.get());
  }

  Slist header_list;

private:
  struct Deleter {
    void operator()(CURL* p) const {
      curl_easy_cleanup(p);
    }
  };

  std::unique_ptr<CURL, Deleter> handle_;
};

}  // namespace hypr::detail::curl
