#pragma once

#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>

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
        (locks_[lock_data], ...);
        setopt(CURLSHOPT_LOCKFUNC, Lock);
        setopt(CURLSHOPT_UNLOCKFUNC, Unlock);
        setopt(CURLSHOPT_USERDATA, &locks_);
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

  using locks_t = std::unordered_map<curl_lock_data, std::mutex>;

  static void Lock(CURL*, curl_lock_data lock_data, curl_lock_access,
                   void* userptr) {
    auto& locks = *static_cast<locks_t*>(userptr);
    locks[lock_data].lock();
  };
  static void Unlock(CURL*, curl_lock_data lock_data, void* userptr) {
    auto& locks = *static_cast<locks_t*>(userptr);
    locks[lock_data].unlock();
  };

  locks_t locks_;
  std::unique_ptr<CURLSH, Deleter> share_;
};

}  // namespace hypr::detail::curl
