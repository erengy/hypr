#pragma once

#include <string_view>

#include <curl/curl.h>
#include <hypp/detail/syntax.hpp>
#include <hypp/parser/header.hpp>
#include <hypp/parser/response.hpp>

#include <hypr/detail/models.hpp>

namespace hypr::detail::curl {

inline int debug_callback(CURL*, curl_infotype type, char* data, size_t size,
                          void* userptr) {
  if (userptr) {
    const auto& response = *static_cast<hypr::detail::Response*>(userptr);
    if (response.callbacks.debug) {
      response.callbacks.debug(type, std::string_view{data, size});
    }
  }

  return 0;
}

inline size_t header_callback(char* buffer, size_t size, size_t nitems,
                              void* userdata) {
  const std::string_view line{buffer, size * nitems};

  const auto parse_status_line = [&line]() {
    hypp::Parser parser{line};
    return hypp::ParseStatusLine(parser);
  };
  const auto parse_header_field = [&line]() {
    hypp::Parser parser{line};
    return hypp::ParseHeaderField(parser);
  };

  if (userdata && !line.empty()) {
    auto& response = *static_cast<hypr::detail::Response*>(userdata);

    if (const auto expected = parse_status_line()) {
      response.start_line = std::move(expected.value());
      response.header_fields.clear();

    } else if (const auto expected = parse_header_field()) {
      response.header_fields.emplace_back(std::move(expected.value()));

    } else if (line == hypp::detail::syntax::kCRLF) {
      if (response.body.empty() && response.session) {
        curl_off_t content_length = 0;
        const auto curl_code = response.session->getinfo(
            CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, content_length);
        if (curl_code == CURLE_OK && content_length > 0) {
          response.body.reserve(static_cast<size_t>(content_length));
        }
      }

    } else {
      // @TODO: Handle error
    }
  }

  return line.size();
}

inline int progress_callback(void* clientp,
                             curl_off_t dltotal, curl_off_t dlnow,
                             curl_off_t, curl_off_t) {
  if (clientp && (dltotal || dlnow)) {
    auto& response = *static_cast<hypr::detail::Response*>(clientp);

    if (response.transfer.current != dlnow ||
        response.transfer.total != dltotal) {
      response.transfer = {dlnow, dltotal};
      if (response.callbacks.transfer) {
        if (!response.callbacks.transfer(response.transfer)) {
          return 1;  // abort
        }
      }
    }
  }

  return 0;
}

inline size_t write_callback(char* ptr, size_t size, size_t nmemb,
                             void* userdata) {
  const std::string_view data{ptr, size * nmemb};

  if (userdata && !data.empty()) {
    auto& response = *static_cast<hypr::detail::Response*>(userdata);
    response.body.append(data);
  }

  return data.size();
}

}  // namespace hypr::detail::curl
