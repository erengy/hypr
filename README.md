# hypr

hypr is an HTTP client library for C++. Provides an interface similar to [Requests](https://requests.readthedocs.io/). Uses [hypp](https://github.com/erengy/hypp/) and [libcurl](https://curl.haxx.se/) under the hood.

## Usage

***This is a work in progress. Usage in public applications is not yet recommended.***

```cpp
#include <iostream>
#include <hypr.hpp>

int main() {
  const auto r = hypr::get("https://example.com");

  std::cout << r.status_code() << '\n';           // 200
  std::cout << r.header("content-type") << '\n';  // text/html; charset=UTF-8
  std::cout << r.body().substr(0, 15) << '\n';    // <!doctype html>

  return 0;
}
```

### Advanced Usage

```cpp
const auto r = hypr::request("POST", "https://httpbin.org/post",
    // Optional parameters can be given in any order
    hypr::Query{{"a", "1"}, {"b", "2"}},
    hypr::Headers{
      {"Content-Type", "text/plain"},
      {"Referer", "https://github.com/erengy/hypr/"},
    },
    hypr::Body{"My body is ready."});

std::cout << r.url() << '\n';  // https://httpbin.org/post?a=1&b=2
```

```cpp
// Requests can be built explicitly
hypr::Request request;
request.set_method("POST");
request.set_url("https://httpbin.org/post");
request.set_query({{"a", "1"}, {"b", "2"}});
request.set_headers({
    {"Content-Type", "text/plain"},
    {"Referer", "https://github.com/erengy/hypr/"}});
request.set_body("My body is ready.");

// Sessions can be reused
hypr::Session session;
const auto r = session.send(request);
```

### Error Handling

```cpp
// FTP is not supported, so we will get an error
const auto r = hypr::get("ftp://example.com");

if (r.error()) {
  std::cout << "Error: " << r.error().str() << '\n';  // Unsupported protocol
  return r.error().code;  // CURLE_UNSUPPORTED_PROTOCOL
}
```

## License

Licensed under the [MIT License](https://opensource.org/licenses/MIT).
