#pragma once

#include <string_view>

#include <curl/curl.h>

namespace hypr::detail::curl {

class Slist {
public:
  ~Slist() {
    free_all();
  }

  // https://curl.haxx.se/libcurl/c/curl_slist_append.html
  bool append(const std::string_view string) {
    const auto new_slist = curl_slist_append(slist_, string.data());
    if (new_slist) {
      slist_ = new_slist;
    }
    return new_slist != nullptr;
  }

  // https://curl.haxx.se/libcurl/c/curl_slist_free_all.html
  void free_all() {
    if (slist_) {
      curl_slist_free_all(slist_);
      slist_ = nullptr;
    }
  }

  curl_slist* get() const {
    return slist_;
  }

private:
  curl_slist* slist_ = nullptr;
};

}  // namespace hypr::detail::curl
