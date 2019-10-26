#pragma once

#include <memory>
#include <mutex>
#include <type_traits>

#include <curl/curl.h>

namespace hypr::detail::curl {

class Share {
public:
  // https://curl.haxx.se/libcurl/c/curl_share_init.html
  template <typename... Ts>
  bool init(const Ts... lock_data) {
    static_assert(sizeof...(Ts));
    static_assert((std::is_same_v<curl_lock_data, Ts> && ...));
    if (!share_) {
      share_.reset(curl_share_init());
      if (share_) {
        setopt(CURLSHOPT_LOCKFUNC, Lock);
        setopt(CURLSHOPT_UNLOCKFUNC, Unlock);
        setopt(CURLSHOPT_USERDATA, &mutex_);
        (setopt(CURLSHOPT_SHARE, lock_data), ...);
      }
    }
    return share_ != nullptr;
  }

  // https://curl.haxx.se/libcurl/c/curl_share_cleanup.html
  void cleanup() {
    share_.reset();
  }

  // https://curl.haxx.se/libcurl/c/curl_share_setopt.html
  template <typename T>
  CURLSHcode setopt(CURLSHoption option, const T& arg) const {
    return curl_share_setopt(share_.get(), option, arg);
  }

  CURLSH* get() const {
    return share_.get();
  }

private:
  struct Deleter {
    void operator()(CURLSH* p) const {
      curl_share_cleanup(p);
    }
  };

  static void Lock(CURL*, curl_lock_data, curl_lock_access, void* userptr) {
    static_cast<std::mutex*>(userptr)->lock();
  };
  static void Unlock(CURL*, curl_lock_data, void* userptr) {
    static_cast<std::mutex*>(userptr)->unlock();
  };

  std::mutex mutex_;
  std::unique_ptr<CURLSH, Deleter> share_;
};

}  // namespace hypr::detail::curl
