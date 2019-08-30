#pragma once

#include <memory>
#include <string_view>

#include <curl/curl.h>

namespace hypr::detail::curl {

class Slist {
public:
  // https://curl.haxx.se/libcurl/c/curl_slist_append.html
  bool append(const std::string_view string) {
    const auto new_slist = curl_slist_append(slist_.get(), string.data());
    if (new_slist) {
      slist_.release();
      slist_.reset(new_slist);
    }
    return new_slist != nullptr;
  }

  // https://curl.haxx.se/libcurl/c/curl_slist_free_all.html
  void free_all() {
    slist_.reset();
  }

  curl_slist* get() const {
    return slist_.get();
  }

private:
  struct Deleter {
    void operator()(curl_slist* p) const {
      curl_slist_free_all(p);
    }
  };

  std::unique_ptr<curl_slist, Deleter> slist_;
};

}  // namespace hypr::detail::curl
