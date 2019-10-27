#include <cassert>
#include <iostream>

#include <hypr.hpp>

namespace {

////////////////////////////////////////////////////////////////////////////////
// Request

void test_request_method() {
  hypr::Request r;

  assert(r.method() == hypp::method::kGet);

  r.set_method("");
  assert(r.method() == hypp::method::kGet);

  r.set_method("!!!");
  assert(r.method() == "!!!");

  r.set_method(hypp::method::kOptions);
  assert(r.method() == hypp::method::kOptions);

  r.set_method("post");
  assert(r.method() == "POST");
}

// @TODO: Move to hypp
void test_request_target() {
  hypr::Request r;

  r.set_target("/where?q=now");
  assert(r.target().form == hypp::RequestTarget::Form::Origin);
  assert(r.target().uri.scheme.has_value() == false);
  assert(r.target().uri.authority.has_value() == false);
  assert(r.target().uri.path == "/where");
  assert(r.target().uri.query.has_value());
  assert(r.target().uri.query.value() == "q=now");
  assert(r.target().uri.fragment.has_value() == false);

  r.set_target("http://www.example.org/pub/WWW/TheProject.html");
  assert(r.target().form == hypp::RequestTarget::Form::Absolute);
  assert(r.target().uri.scheme.has_value());
  assert(r.target().uri.scheme.value() == "http");
  assert(r.target().uri.authority.has_value());
  assert(r.target().uri.authority.value().host == "www.example.org");
  assert(r.target().uri.authority.value().port.has_value() == false);
  assert(r.target().uri.authority.value().user_info.has_value() == false);
  assert(r.target().uri.path == "/pub/WWW/TheProject.html");
  assert(r.target().uri.query.has_value() == false);
  assert(r.target().uri.fragment.has_value() == false);

  // @TODO: Fix hypp::ParseRequestTarget()
  /*
  r.set_target("www.example.com:80");
  assert(r.target().form == hypp::RequestTarget::Form::Authority);
  assert(r.target().uri.scheme.has_value() == false);
  assert(r.target().uri.authority.has_value());
  assert(r.target().uri.authority.value().host == "www.example.com");
  assert(r.target().uri.authority.value().port.has_value());
  assert(r.target().uri.authority.value().port.value() == "80");
  assert(r.target().uri.authority.value().user_info.has_value() == false);
  assert(r.target().uri.path.empty());
  assert(r.target().uri.query.has_value() == false);
  assert(r.target().uri.fragment.has_value() == false);
  */

  r.set_target("*");
  assert(r.target().form == hypp::RequestTarget::Form::Asterisk);
  assert(r.target().uri.scheme.has_value() == false);
  assert(r.target().uri.authority.has_value() == false);
  assert(r.target().uri.path.empty());
  assert(r.target().uri.query.has_value() == false);
  assert(r.target().uri.fragment.has_value() == false);
}

void test_request_query() {
  hypr::Request r;

  r.set_query(hypr::Query{"plain!text?hello&world"});
  assert(r.target().uri.query.has_value());
  assert(r.target().uri.query.value() == "plain%21text%3Fhello%26world");

  r.set_query(hypr::Query{{"a", "1"}, {"b", "2"}});
  assert(r.target().uri.query.has_value());
  assert(r.target().uri.query.value() == "a=1&b=2");

  r.set_query(hypr::Query{});
  assert(r.target().uri.query.has_value() == false);
}

void test_request_headers() {
  constexpr auto connection = "keep-alive";
  constexpr auto content_type_html = "text/html";
  constexpr auto content_type_json = "application/json";
  constexpr auto referer = "https://www.example.com";

  hypr::Request r;

  r.set_headers({
        {"Content-Type", content_type_html},
        {"Referer", referer},
      });
  assert(r.header("content-type") == content_type_html);
  assert(r.header("referer") == referer);

  r.set_header("Content-Type", content_type_json);
  assert(r.header("content-type") == content_type_json);

  r.add_header("Connection", connection);
  assert(r.header("connection") == connection);

  r.add_header("Cookie", "foo");
  assert(r.header("cookie") == "foo");
  r.add_header("Cookie", "bar");
  assert(r.header("cookie") == "foo, bar");
}

void test_request_body() {
  constexpr auto str = "plain!text?hello&world";

  {
    hypr::Request r;
    r.set_body(hypr::Body{str});
    assert(r.body() == str);
    assert(r.header("content-type") == "text/plain");
  }

  {
    hypr::Request r;
    r.set_body(hypr::Body{{"a", "1"}, {"b", "2"}});
    assert(r.body() == "a=1&b=2");
    assert(r.header("content-type") == "application/x-www-form-urlencoded");
  }

  {
    hypr::Request r;
    r.set_header("Content-Type", "application/json");
    r.set_body(hypr::Body{str});
    assert(r.body() == str);
    assert(r.header("content-type") == "application/json");
  }
}

////////////////////////////////////////////////////////////////////////////////
// Response

bool is_response_ok(const hypr::Response& r) {
  if (r.error()) {
    std::cout << r.error().str() << '\n';
    return false;
  }
  return r.status_code() < hypp::status::k400_Bad_Request;
}

void test_response_simple() {
  const auto r = hypr::get("https://example.com");
  assert(is_response_ok(r));
  assert(r.header("content-type") == "text/html; charset=UTF-8");
  assert(r.body().substr(0, 15) == "<!doctype html>");
}

void test_response_advanced() {
  const auto r = hypr::request("POST", "https://httpbin.org/post",
      hypr::Query{{"a", "1"}, {"b", "2"}},
      hypr::Headers{
        {"Referer", "https://github.com/erengy/hypr"},
      },
      hypr::Body{"My body is ready."}
    );
  assert(is_response_ok(r));
  assert(r.url() == "https://httpbin.org/post?a=1&b=2");
}

void test_session() {
  hypr::Request request;
  request.set_method("POST");
  request.set_target("https://httpbin.org/post");
  request.set_query({{"a", "1"}, {"b", "2"}});
  request.set_headers({{"Referer", "https://github.com/erengy/hypr"}});
  request.set_body({"My body is ready."});

  hypr::Session session;
  const auto r1 = session.send(request);
  assert(is_response_ok(r1));
  assert(r1.url() == "https://httpbin.org/post?a=1&b=2");

  request.set_query({{"c", "3"}, {"d", "4"}});
  const auto r2 = session.send(request);
  assert(is_response_ok(r2));
  assert(r2.url() == "https://httpbin.org/post?c=3&d=4");

  // @TODO: Test session options
}

////////////////////////////////////////////////////////////////////////////////
// Error handling

void test_error_handling() {
  const auto r = hypr::get("ftp://localhost");
  assert(r.error().code == CURLE_UNSUPPORTED_PROTOCOL);
  assert(r.error().str() == "Unsupported protocol");
}

////////////////////////////////////////////////////////////////////////////////

void test_all() {
  test_request_method();
  test_request_target();
  test_request_query();
  test_request_headers();
  test_request_body();
#ifdef HYPR_HAS_INTERNET_CONNECTION
  test_response_simple();
  test_response_advanced();
  test_session();
#endif
  test_error_handling();

  std::cout << "hypr passed all tests!\n";
}

}  // namespace

int main() {
  test_all();
  return 0;
}
