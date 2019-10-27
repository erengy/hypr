#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include <curl/curl.h>
#include <hypp/generator/request.hpp>
#include <hypp/method.hpp>

#include <hypr/detail/curl_callback.hpp>
#include <hypr/detail/curl_global.hpp>
#include <hypr/detail/curl_session.hpp>
#include <hypr/detail/curl_share.hpp>
#include <hypr/detail/models.hpp>
#include <hypr/models.hpp>

namespace hypr::detail::curl {

#define HYPR_CURL_CHECK_OK(arg) \
    if (auto code = arg; code != CURLE_OK) return hypr::Response(code)
#define HYPR_CURL_SETOPT(option, arg) \
    if (auto code = session.setopt(option, arg); code != CURLE_OK) return code

class Interface {
public:
  Interface() = delete;

  static bool init() {
    if (!global_) {
      global_ = std::make_unique<Global>();
    }
    if (!cache_) {
      cache_ = std::make_unique<Share>();
    }
    return global_ && global_->init() &&
           cache_ && cache_->init(CURL_LOCK_DATA_DNS, CURL_LOCK_DATA_CONNECT);
  }

  static hypr::Response send(const hypr::Request& request,
                             const hypr::Callbacks& callbacks,
                             const hypr::Options& options,
                             const hypr::Proxy& proxy,
                             Session& session) {
    hypr::detail::Response response;

    response.callbacks = callbacks;
    response.session = &session;

    HYPR_CURL_CHECK_OK(init() ? CURLE_OK : CURLE_FAILED_INIT);
    HYPR_CURL_CHECK_OK(session.init() ? CURLE_OK : CURLE_FAILED_INIT);
    HYPR_CURL_CHECK_OK(prepare_session(response, session));
    HYPR_CURL_CHECK_OK(prepare_session(options, session));
    HYPR_CURL_CHECK_OK(prepare_session(proxy, session));
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

  static CURLcode prepare_session(const hypr::detail::Response& response,
                                  Session& session) {
    // Behavior options
    session.setopt(CURLOPT_NOPROGRESS, 0L);

    // Callback options
    session.setopt(CURLOPT_WRITEFUNCTION, write_callback);
    session.setopt(CURLOPT_WRITEDATA, &response);
    session.setopt(CURLOPT_XFERINFOFUNCTION, progress_callback);
    session.setopt(CURLOPT_XFERINFODATA, &response);
    session.setopt(CURLOPT_HEADERFUNCTION, header_callback);
    session.setopt(CURLOPT_HEADERDATA, &response);
    session.setopt(CURLOPT_DEBUGFUNCTION, debug_callback);
    session.setopt(CURLOPT_DEBUGDATA, &response);

    // Network options
    HYPR_CURL_SETOPT(CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    HYPR_CURL_SETOPT(CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);

    // HTTP options
#ifdef HAVE_ZLIB_H
    HYPR_CURL_SETOPT(CURLOPT_ACCEPT_ENCODING, "");
#endif
    HYPR_CURL_SETOPT(CURLOPT_USERAGENT, get_default_user_agent().c_str());
    HYPR_CURL_SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    // Connection options
    HYPR_CURL_SETOPT(CURLOPT_LOW_SPEED_LIMIT, 1024L);

    // Other options
    if (cache_) {
      HYPR_CURL_SETOPT(CURLOPT_SHARE, cache_->get());
    }

    return CURLE_OK;
  }

  static CURLcode prepare_session(const hypr::Options& options,
                                  Session& session) {
    HYPR_CURL_SETOPT(CURLOPT_FOLLOWLOCATION, options.allow_redirects);
    HYPR_CURL_SETOPT(CURLOPT_MAXREDIRS,
        std::max(static_cast<long>(options.max_redirects), -1L));
    HYPR_CURL_SETOPT(CURLOPT_LOW_SPEED_TIME,
        std::max(static_cast<long>(options.timeout.count()), 0L));
    HYPR_CURL_SETOPT(CURLOPT_CONNECTTIMEOUT,
        std::max(static_cast<long>(options.timeout.count()), 0L));
    HYPR_CURL_SETOPT(CURLOPT_VERBOSE, options.verbose);
    HYPR_CURL_SETOPT(CURLOPT_SSL_VERIFYHOST,
        options.verify_certificate ? 2L : 0L);
    HYPR_CURL_SETOPT(CURLOPT_SSL_VERIFYPEER, options.verify_certificate);
    HYPR_CURL_SETOPT(CURLOPT_SSL_OPTIONS,
        options.certificate_revocation ? 0L : CURLSSLOPT_NO_REVOKE);

    return CURLE_OK;
  }

  static CURLcode prepare_session(const hypr::Proxy& proxy, Session& session) {
    const auto get_value = [](const std::string& value) {
      return !value.empty() ? value.c_str() : nullptr;
    };

    HYPR_CURL_SETOPT(CURLOPT_PROXY, get_value(proxy.host));
    HYPR_CURL_SETOPT(CURLOPT_PROXYUSERNAME, get_value(proxy.username));
    HYPR_CURL_SETOPT(CURLOPT_PROXYPASSWORD, get_value(proxy.password));

    return CURLE_OK;
  }

  static CURLcode prepare_session(const hypr::Request& request,
                                  Session& session) {
    // Method
    //
    // libcurl uses this string even if we set CURLOPT_HTTPGET or CURLOPT_POST
    // later on. Note that CURLOPT_CUSTOMREQUEST only changes the string, not
    // how libcurl behaves.
    HYPR_CURL_SETOPT(CURLOPT_CUSTOMREQUEST, request.method().data());

    // Target
    HYPR_CURL_SETOPT(CURLOPT_URL, hypp::to_string(request.target()).c_str());

    // Headers
    session.header_list.free_all();
    for (const auto& [name, value] : request.headers()) {
      session.header_list.append(!value.empty() ? name + ": " + value
                                                : name + ";");
    }
    HYPR_CURL_SETOPT(CURLOPT_HTTPHEADER, session.header_list.get());

    // Body
    HYPR_CURL_SETOPT(CURLOPT_POSTFIELDSIZE_LARGE,
        static_cast<curl_off_t>(request.body().size()));
    HYPR_CURL_SETOPT(CURLOPT_POSTFIELDS,
        !request.body().empty() ? request.body().data() : nullptr);

    // CURLOPT_POSTFIELDS automatically sets the request to HTTPREQ_POST, so
    // we need to set the correct behavior afterwards.
    if (!request.body().empty() || request.method() == hypp::method::kPost) {
      HYPR_CURL_SETOPT(CURLOPT_POST, 1L);
    } else {
      HYPR_CURL_SETOPT(CURLOPT_HTTPGET, 1L);
    }

    return CURLE_OK;
  }

  static void prepare_response(const Session& session,
                               hypr::detail::Response& response) {
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

  static inline std::unique_ptr<Global> global_;
  static inline std::unique_ptr<Share> cache_;
};

#undef HYPR_CURL_CHECK_OK
#undef HYPR_CURL_SETOPT

}  // namespace hypr::detail::curl
