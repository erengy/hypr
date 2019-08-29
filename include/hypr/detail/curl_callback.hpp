#pragma once

#include <string_view>

#include <curl/curl.h>
#include <hypp/parser/response.hpp>

#include <hypr/models.hpp>

namespace hypr::detail::curl {

int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size,
                   void* userptr) {
  const std::string_view str{data, size};

  switch (type) {
    case CURLINFO_TEXT:
    case CURLINFO_HEADER_IN:
    case CURLINFO_HEADER_OUT:
    case CURLINFO_DATA_IN:
    case CURLINFO_DATA_OUT:
    case CURLINFO_SSL_DATA_OUT:
    case CURLINFO_SSL_DATA_IN:
      // @TODO
      break;
  }

  return 0;
}

size_t header_callback(char* buffer, size_t size, size_t nitems,
                       void* userdata) {
  const std::string_view data{buffer, size * nitems};

  if (userdata && !data.empty()) {
    auto& response = *static_cast<Response*>(userdata);
    if (data._Starts_with("HTTP/")) {
      hypp::Parser parser{data};
      const auto expected = hypp::ParseStatusLine(parser);
      response.start_line = std::move(expected.value());
    } else if (data != "\r\n") {
      hypp::Parser parser{data};
      const auto expected = hypp::ParseHeaderField(parser);
      response.header.fields.push_back(std::move(expected.value()));
    }
  }

  return data.size();
}

int progress_callback(void* clientp,
                      curl_off_t dltotal, curl_off_t dlnow,
                      curl_off_t ultotal, curl_off_t ulnow) {
  return 0;
}

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  const std::string_view data{ptr, size * nmemb};

  if (userdata && !data.empty()) {
    auto& response = *static_cast<Response*>(userdata);
    response.body.append(data);
  }

  return data.size();
}

}  // namespace hypr::detail::curl
