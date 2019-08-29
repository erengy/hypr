#pragma once

#include <string>
#include <string_view>

#include <curl/curl.h>

#include <hypp/generator/request.hpp>

#include <hypr/detail/curl_callback.hpp>
#include <hypr/detail/curl_error.hpp>
#include <hypr/detail/curl_global.hpp>
#include <hypr/detail/curl_session.hpp>
#include <hypr/detail/curl_error.hpp>
#include <hypr/models.hpp>

namespace hypr::detail::curl {

class Interface {
public:
  bool init() {
    static Global global;
    return global.init() == CURLE_OK;
  }

  Response send(const hypr::Request& request, const hypr::Options& options) {
    Response response;

    // @TODO: Handle errors
    Session session;
    session.init();
    build_session(request, options, session, response);
    session.perform();  // blocks

    build_response(session, response);

    return response;
  }

private:
  static std::string get_default_user_agent() {
    static const std::string default_user_agent =
        "hypr/0.1 " + std::string{curl_version()};
    return default_user_agent;
  }

  Slist build_header_list(const hypr::Request& request) const {
    Slist list;

    for (const auto& field : request.header.fields) {
      list.append(!field.value.empty() ?
          field.name + ": " + field.value :
          field.name + ";");
    }

    return std::move(list);
  }

  CURLcode build_session(const hypr::Request& request,
                         const hypr::Options& options,
                         Session& session,
                         Response& response) {
    CURLcode code = CURLE_OK;

    #define HYPR_CURL_SETOPT(option, value) \
        if ((code = session.setopt(option, value)) != CURLE_OK) return code;

    // Callback options

    HYPR_CURL_SETOPT(CURLOPT_HEADERFUNCTION, header_callback);
    HYPR_CURL_SETOPT(CURLOPT_HEADERDATA, &response);

    HYPR_CURL_SETOPT(CURLOPT_WRITEFUNCTION, write_callback);
    HYPR_CURL_SETOPT(CURLOPT_WRITEDATA, &response);

    // Network options

    const auto url = hypp::to_string(request.start_line.target);
    HYPR_CURL_SETOPT(CURLOPT_URL, url.c_str());

    constexpr int protocols = CURLPROTO_HTTP | CURLPROTO_HTTPS;
    HYPR_CURL_SETOPT(CURLOPT_PROTOCOLS, protocols);
    HYPR_CURL_SETOPT(CURLOPT_REDIR_PROTOCOLS, protocols);

    // HTTP options

    HYPR_CURL_SETOPT(CURLOPT_FOLLOWLOCATION, options.allow_redirects ? 1L : 0L);
    HYPR_CURL_SETOPT(CURLOPT_MAXREDIRS, options.max_redirects);

    HYPR_CURL_SETOPT(CURLOPT_USERAGENT, get_default_user_agent().c_str());

    session.header_list = build_header_list(request);
    HYPR_CURL_SETOPT(CURLOPT_HTTPHEADER, session.header_list.get());

    HYPR_CURL_SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    // Connection options

    HYPR_CURL_SETOPT(CURLOPT_CONNECTTIMEOUT, options.timeout.count());

    // Security options

    if (!options.verify_certificate) {
      HYPR_CURL_SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
      HYPR_CURL_SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
    }

    #undef HYPR_CURL_SETOPT

    return code;
  }

  void build_response(const Session& session, Response& response) const {
    char* url = nullptr;
    session.getinfo(CURLINFO_EFFECTIVE_URL, &url);
    if (url) {
      response.effective_url = url;
    }
  }
};

}  // namespace hypr::detail::curl
