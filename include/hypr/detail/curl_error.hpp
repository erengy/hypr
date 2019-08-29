#pragma once

#include <string>

#include <curl/curl.h>

namespace hypr::detail::curl {

class Error {
public:
  // https://curl.haxx.se/libcurl/c/curl_easy_strerror.html
  std::string str() const {
    return curl_easy_strerror(code);
  }

  // https://curl.haxx.se/libcurl/c/libcurl-errors.html
  CURLcode code = CURLE_OK;
};

}  // namespace hypr::detail::curl
