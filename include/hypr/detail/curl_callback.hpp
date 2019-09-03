#pragma once

#include <string_view>

#include <curl/curl.h>
#include <hypp/detail/syntax.hpp>
#include <hypp/parser/header.hpp>
#include <hypp/parser/response.hpp>

#include <hypr/detail/models.hpp>

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
    case CURLINFO_SSL_DATA_IN:
    case CURLINFO_SSL_DATA_OUT:
      // @TODO
      break;
  }

  return 0;
}

size_t header_callback(char* buffer, size_t size, size_t nitems,
                       void* userdata) {
  const std::string_view line{buffer, size * nitems};

  if (userdata && !line.empty()) {
    auto& response = *static_cast<hypr::detail::Response*>(userdata);

    const auto parse_status_line = [&line]() {
      hypp::Parser parser{line};
      return hypp::ParseStatusLine(parser);
    };
    const auto parse_header_field = [&line]() {
      hypp::Parser parser{line};
      return hypp::ParseHeaderField(parser);
    };

    if (const auto expected = parse_status_line()) {
      response.start_line = std::move(expected.value());
      response.header.fields.clear();

    } else if (const auto expected = parse_header_field()) {
      response.header.fields.emplace_back(std::move(expected.value()));

    } else if (line == hypp::detail::syntax::kCRLF) {
      if (response.body.empty() && response.session) {
        curl_off_t content_length = 0;
        if (response.session->getinfo(CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                                      content_length) == CURLE_OK) {
          if (content_length > 0) {
            response.body.reserve(static_cast<size_t>(content_length));
          }
        }
      }

    } else {
      // @TODO: Handle error
    }
  }

  return line.size();
}

int progress_callback(void* clientp,
                      curl_off_t dltotal, curl_off_t dlnow,
                      curl_off_t ultotal, curl_off_t ulnow) {
  // @TODO
  return 0;
}

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  const std::string_view data{ptr, size * nmemb};

  if (userdata && !data.empty()) {
    auto& response = *static_cast<hypr::detail::Response*>(userdata);
    response.body.append(data);
  }

  return data.size();
}

}  // namespace hypr::detail::curl
