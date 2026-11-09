#include "test_framework.h"

#include <stdexcept>

#include "core/HttpServer.h"

using namespace calc::http;

TEST(Http, QueryStringParsing) {
    auto params = parseQueryString("a=1&b=hello&c=");
    CHECK_EQ(params.size(), size_t(3));
    CHECK_STREQ(params["a"], "1");
    CHECK_STREQ(params["b"], "hello");
    CHECK_STREQ(params["c"], "");

    // 键值都做 URL 解码。
    auto decoded = parseQueryString("name=%E4%B8%AD%E6%96%87&x=1%2B2");
    CHECK_STREQ(decoded["name"], "中文");
    CHECK_STREQ(decoded["x"], "1+2");
}

TEST(Http, UrlDecode) {
    CHECK_STREQ(urlDecode("hello"), "hello");
    CHECK_STREQ(urlDecode("a%20b"), "a b");
    CHECK_STREQ(urlDecode("a+b"), "a b");
    CHECK_STREQ(urlDecode("%E4%B8%AD"), "中");
}

TEST(Http, RequestParameters) {
    Request request;
    request.queryParams = parseQueryString("n=42&ratio=0.5&text=hello&list=1&list=2&list=3");
    request.queryParamsAll = parseQueryStringAll("n=42&ratio=0.5&text=hello&list=1&list=2&list=3");

    CHECK_EQ(request.integerParam("n"), 42LL);
    CHECK_NEAR(request.numberParam("ratio"), 0.5, 1e-12);
    CHECK_STREQ(request.param("text"), "hello");
    CHECK_STREQ(request.param("missing", "fallback"), "fallback");
    // 非法的数值返回 fallback。
    CHECK_NEAR(request.numberParam("text", -1.0), -1.0, 1e-12);

    const auto list = request.numberListParam("list");
    CHECK_EQ(list.size(), size_t(3));
    CHECK_NEAR(list[0], 1.0, 1e-12);
    CHECK_NEAR(list[2], 3.0, 1e-12);
}

TEST(Http, QueryStringKeepsDuplicates) {
    // 同名参数应全部保留且顺序不变。
    const auto all = parseQueryStringAll("a=1&a=2&b=3&a=4");
    CHECK_EQ(all.size(), size_t(2));
    CHECK_EQ(all.at("a").size(), size_t(3));
    CHECK_STREQ(all.at("a")[0], "1");
    CHECK_STREQ(all.at("a")[1], "2");
    CHECK_STREQ(all.at("a")[2], "4");
    CHECK_STREQ(all.at("b")[0], "3");

    // 单值视图保留最后一个。
    const auto single = parseQueryString("a=1&a=2");
    CHECK_STREQ(single.at("a"), "2");
}

TEST(Http, ResponseFactories) {
    auto json = Response::json("{\"a\":1}");
    CHECK_EQ(json.status, 200);
    CHECK(json.contentType.find("application/json") != std::string::npos);
    CHECK_STREQ(json.body, "{\"a\":1}");

    auto error = Response::error(404, "未找到");
    CHECK_EQ(error.status, 404);
    CHECK(error.body.find("未找到") != std::string::npos);

    auto html = Response::html("<html></html>");
    CHECK(html.contentType.find("text/html") != std::string::npos);
}

TEST(Http, RouteDispatch) {
    Server server;
    server.route("/api/ping", [](const Request&) {
        return Response::json("{\"pong\":true}");
    });

    Request request;
    request.path = "/api/ping";
    const Response response = server.handle(request);
    CHECK_EQ(response.status, 200);
    CHECK(response.body.find("pong") != std::string::npos);

    // 未注册的路径返回 404。
    Request missing;
    missing.path = "/api/missing";
    CHECK_EQ(server.handle(missing).status, 404);
}

TEST(Http, HandlerExceptionIsCaught) {
    Server server;
    server.route("/api/boom", [](const Request&) -> Response {
        throw std::runtime_error("出错了");
    });

    Request request;
    request.path = "/api/boom";
    const Response response = server.handle(request);
    // 处理器抛异常时应返回 500 而不是让服务器崩溃。
    CHECK_EQ(response.status, 500);
    CHECK(response.body.find("出错了") != std::string::npos);
}

TEST(Http, QueryParamsReachHandler) {
    Server server;
    server.route("/api/echo", [](const Request& request) {
        return Response::json("{\"n\":" + request.param("n", "0") + "}");
    });

    Request request;
    request.path = "/api/echo";
    request.queryParams = parseQueryString("n=7");
    const Response response = server.handle(request);
    CHECK(response.body.find("7") != std::string::npos);
}

TEST(Http, ListenAndBind) {
    Server server;
    // 端口 0 让系统分配空闲端口，验证绑定逻辑可用。
    const bool bound = server.listen("127.0.0.1", 0);
    CHECK(bound);
    CHECK(server.boundPort() > 0);
    server.stop();
}
