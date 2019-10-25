#pragma once

#include <memory>
#include <mutex>

#include <curl/curl.h>

namespace hypr::detail::curl {

class Share {
public:
  // https://curl.haxx.se/libcurl/c/curl_share_init.html
  bool init(const curl_lock_data lock_data) {
    if (!share_) {
      share_.reset(curl_share_init());
      if (share_) {
        lock_data_ = lock_data;
        setopt(CURLSHOPT_LOCKFUNC, Lock);
        setopt(CURLSHOPT_UNLOCKFUNC, Unlock);
        setopt(CURLSHOPT_SHARE, lock_data_);
        setopt(CURLSHOPT_USERDATA, &mutex_);
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

  curl_lock_data lock_data_ = CURL_LOCK_DATA_CONNECT;
  std::mutex mutex_;
  std::unique_ptr<CURLSH, Deleter> share_;
};

}  // namespace hypr::detail::curl
