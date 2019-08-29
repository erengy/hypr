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
    static const auto default_user_agent = std::string{"hypr/0.1 libcurl/"} +
        std::to_string(LIBCURL_VERSION_MAJOR) + "." +
        std::to_string(LIBCURL_VERSION_MINOR) + "." +
        std::to_string(LIBCURL_VERSION_PATCH);
    return default_user_agent;
  }

  static void build_header_list(const hypr::Request& request, Slist& list) {
    for (const auto& field : request.header.fields) {
      list.append(!field.value.empty() ?
          field.name + ": " + field.value :
          field.name + ";");
    }
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

    HYPR_CURL_SETOPT(CURLOPT_XFERINFOFUNCTION, progress_callback);
    HYPR_CURL_SETOPT(CURLOPT_XFERINFODATA, nullptr);
    HYPR_CURL_SETOPT(CURLOPT_NOPROGRESS, 0L);

    HYPR_CURL_SETOPT(CURLOPT_WRITEFUNCTION, write_callback);
    HYPR_CURL_SETOPT(CURLOPT_WRITEDATA, &response);

    HYPR_CURL_SETOPT(CURLOPT_DEBUGFUNCTION, debug_callback);
    HYPR_CURL_SETOPT(CURLOPT_DEBUGDATA, nullptr);

    // Network options

    const auto url = hypp::to_string(request.start_line.target);
    HYPR_CURL_SETOPT(CURLOPT_URL, url.c_str());

    constexpr int protocols = CURLPROTO_HTTP | CURLPROTO_HTTPS;
    HYPR_CURL_SETOPT(CURLOPT_PROTOCOLS, protocols);
    HYPR_CURL_SETOPT(CURLOPT_REDIR_PROTOCOLS, protocols);

    // HTTP options

    HYPR_CURL_SETOPT(CURLOPT_FOLLOWLOCATION, options.allow_redirects ? 1L : 0L);
    HYPR_CURL_SETOPT(CURLOPT_MAXREDIRS, options.max_redirects);

    if (request.start_line.method == hypp::method::kPost) {
      session.post_data = request.body;
      HYPR_CURL_SETOPT(CURLOPT_POST, 1L);
      HYPR_CURL_SETOPT(CURLOPT_POSTFIELDS, session.post_data.c_str());
      HYPR_CURL_SETOPT(CURLOPT_POSTFIELDSIZE, session.post_data.size());
    }

    HYPR_CURL_SETOPT(CURLOPT_USERAGENT, get_default_user_agent().c_str());

    build_header_list(request, session.header_list);
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
    // Last used URL
    char* url = nullptr;
    if (session.getinfo(CURLINFO_EFFECTIVE_URL, url) == CURLE_OK && url) {
      response.effective_url = url;
    }

    // Last received response code
    long code = 0;
    if (session.getinfo(CURLINFO_RESPONSE_CODE, code) == CURLE_OK && code) {
      response.start_line.code = code;
    }
  }
};

}  // namespace hypr::detail::curl
