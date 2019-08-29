# hypr

hypr is an HTTP client library for C++. Provides an interface similar to [Requests](https://2.python-requests.org). Uses [hypp](https://github.com/erengy/hypp/) and [libcurl](https://curl.haxx.se) under the hood.

## Usage

***This is a work in progress. Usage in public applications is not yet recommended.***

```cpp
#include <iostream>
#include <hypr.hpp>

int main() {
  const auto r = hypr::get("http://example.com");

  std::cout << r.status_code() << '\n';            // 200
  std::cout << r.headers["Content-Type"] << '\n';  // text/html
  std::cout << r.content().substr(0, 15) << '\n';  // <!doctype html>

  return 0;
}
```

```cpp
#include <iostream>
#include <hypr.hpp>

int main() {
  const auto r = hypr::request("GET", "http://example.com",
      hypr::Headers{
        {"Referer", "https://github.com/erengy/hypr/"},
        {"User-Agent", "hypr/0.1"},
      },
      hypr::Params{
        {"key1", "value1"},
        {"key2", "value2"},
      }
    );

  std::cout << r.url() << '\n';  // http://example.com/?key1=value1&key2=value2

  return 0;
}
```

## License

Licensed under the [MIT License](https://opensource.org/licenses/MIT).
