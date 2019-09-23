#pragma once

#include <algorithm>
#include <string>
#include <string_view>

#include <curl/curl.h>
#include <hypp/generator/request.hpp>
#include <hypp/method.hpp>

#include <hypr/detail/curl_callback.hpp>
#include <hypr/detail/curl_global.hpp>
#include <hypr/detail/curl_session.hpp>
#include <hypr/detail/models.hpp>
#include <hypr/models.hpp>

namespace hypr::detail::curl {

#define HYPR_CURL_CHECK_OK(arg) \
    if (auto code = arg; code != CURLE_OK) return hypr::Response(code)
#define HYPR_CURL_SETOPT(option, arg) \
    if (auto code = session.setopt(option, arg); code != CURLE_OK) return code

class Interface {
public:
  bool init() const {
    static Global global;
    return global.init() == CURLE_OK;
  }

  hypr::Response send(const hypr::Request& request,
                      const hypr::Options& options) const {
    Session session;

    hypr::detail::Response response;
    response.session = &session;

    HYPR_CURL_CHECK_OK(session.init() ? CURLE_OK : CURLE_FAILED_INIT);
    HYPR_CURL_CHECK_OK(prepare_session(response, session));
    HYPR_CURL_CHECK_OK(prepare_session(options, session));
    HYPR_CURL_CHECK_OK(prepare_session(request, session));
    HYPR_CURL_CHECK_OK(session.perform());  // blocks

    prepare_response(session, response);

    return hypr::Response(std::move(response));
  }

private:
  static std::string get_default_user_agent() {
    static const auto default_user_agent = std::string{"hypr/0.1 libcurl/"} +
        std::to_string(LIBCURL_VERSION_MAJOR) + "." +
        std::to_string(LIBCURL_VERSION_MINOR) + "." +
        std::to_string(LIBCURL_VERSION_PATCH);
    return default_user_agent;
  }

  CURLcode prepare_session(const hypr::detail::Response& response,
                           Session& session) const {
    // Behavior options
    session.setopt(CURLOPT_NOPROGRESS, 0L);

    // Callback options
    session.setopt(CURLOPT_WRITEFUNCTION, write_callback);
    session.setopt(CURLOPT_WRITEDATA, &response);
    session.setopt(CURLOPT_XFERINFOFUNCTION, progress_callback);
    session.setopt(CURLOPT_XFERINFODATA, nullptr);  // @TODO
    session.setopt(CURLOPT_HEADERFUNCTION, header_callback);
    session.setopt(CURLOPT_HEADERDATA, &response);
    session.setopt(CURLOPT_DEBUGFUNCTION, debug_callback);
    session.setopt(CURLOPT_DEBUGDATA, nullptr);  // @TODO

    // Network options
    HYPR_CURL_SETOPT(CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    HYPR_CURL_SETOPT(CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);

    // HTTP options
#ifdef HAVE_ZLIB_H
    HYPR_CURL_SETOPT(CURLOPT_ACCEPT_ENCODING, "");
#endif
    HYPR_CURL_SETOPT(CURLOPT_USERAGENT, get_default_user_agent().c_str());
    HYPR_CURL_SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    return CURLE_OK;
  }

  CURLcode prepare_session(const hypr::Options& options,
                           Session& session) const {
    HYPR_CURL_SETOPT(CURLOPT_FOLLOWLOCATION, options.allow_redirects);
    HYPR_CURL_SETOPT(CURLOPT_MAXREDIRS,
        std::max(static_cast<long>(options.max_redirects), -1L));
    HYPR_CURL_SETOPT(CURLOPT_CONNECTTIMEOUT,
        std::max(static_cast<long>(options.timeout.count()), 0L));
    HYPR_CURL_SETOPT(CURLOPT_VERBOSE, options.verbose);
    HYPR_CURL_SETOPT(CURLOPT_SSL_VERIFYHOST,
        options.verify_certificate ? 2L : 0L);
    HYPR_CURL_SETOPT(CURLOPT_SSL_VERIFYPEER, options.verify_certificate);

    return CURLE_OK;
  }

  CURLcode prepare_session(const hypr::Request& request,
                           Session& session) const {
    if (request.method() == hypp::method::kGet) {
      HYPR_CURL_SETOPT(CURLOPT_HTTPGET, 1L);
    } else if (request.method() == hypp::method::kPost) {
      HYPR_CURL_SETOPT(CURLOPT_POST, 1L);
      HYPR_CURL_SETOPT(CURLOPT_POSTFIELDS, request.body().data());
      HYPR_CURL_SETOPT(CURLOPT_POSTFIELDSIZE, request.body().size());
    } else {
      HYPR_CURL_SETOPT(CURLOPT_CUSTOMREQUEST, request.method().data());
    }

    HYPR_CURL_SETOPT(CURLOPT_URL, hypp::to_string(request.url()).c_str());

    session.header_list.free_all();
    for (const auto& [name, value] : request.headers()) {
      session.header_list.append(
          !value.empty() ? name + ": " + value : name + ";");
    }
    HYPR_CURL_SETOPT(CURLOPT_HTTPHEADER, session.header_list.get());

    return CURLE_OK;
  }

  void prepare_response(const Session& session,
                        hypr::detail::Response& response) const {
    for (auto&& [name, value] : response.header_fields) {
      response.headers.emplace(std::move(name), std::move(value));
    }
    response.header_fields.clear();

    // Last used URL
    char* url = nullptr;
    if (session.getinfo(CURLINFO_EFFECTIVE_URL, url) == CURLE_OK && url) {
      response.url = url;
    }

    // Last received response code
    long code = 0;
    if (session.getinfo(CURLINFO_RESPONSE_CODE, code) == CURLE_OK && code) {
      response.start_line.code = code;
    }

    // Total time of previous transfer
    curl_off_t total = 0;
    if (session.getinfo(CURLINFO_TOTAL_TIME_T, total) == CURLE_OK && total) {
      response.elapsed = std::chrono::microseconds{total};
    }
  }
};

#undef HYPR_CURL_CHECK_OK
#undef HYPR_CURL_SETOPT

}  // namespace hypr::detail::curl
