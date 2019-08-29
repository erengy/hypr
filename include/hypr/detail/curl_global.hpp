#pragma once

#include <curl/curl.h>

namespace hypr::detail::curl {

class Global {
public:
  ~Global() {
    cleanup();
  }

  // https://curl.haxx.se/libcurl/c/curl_global_init.html
  CURLcode init() {
    code_ = curl_global_init(CURL_GLOBAL_ALL);
    return code_;
  }

  // https://curl.haxx.se/libcurl/c/curl_global_cleanup.html
  void cleanup() {
    if (initialized()) {
      curl_global_cleanup();
      code_ = CURLE_FAILED_INIT;
    }
  }

  bool initialized() const {
    return code_ == CURLE_OK;
  }

private:
  CURLcode code_ = CURLE_FAILED_INIT;
};

}  // namespace hypr::detail::curl
