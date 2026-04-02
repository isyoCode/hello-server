// ============================================================================
// http_parser_test.cc - HTTP 解析器单元测试
// ============================================================================
// 使用 doctest 框架测试 HTTP 请求和响应解析器
//
// 测试内容:
//   - HTTP 请求解析 (method, URL, version, headers, body)
//   - HTTP 响应解析 (status code, headers, body)
//   - 不同的 HTTP 方法 (GET, POST, PUT, DELETE)
//   - Header 处理
//   - Body 处理
// ============================================================================

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <iostream>
#include <string>
#include "../../src/http/request_parser.h"
#include "../../src/http/response_parser.h"

using namespace yoyo::http;

// ============================================================================
// 测试数据
// ============================================================================

const std::string GET_REQUEST =
    "GET /api/user?id=123 HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "User-Agent: curl/7.68.0\r\n"
    "Accept: */*\r\n"
    "\r\n";

const std::string POST_REQUEST =
    "POST /api/user HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 27\r\n"
    "\r\n"
    "{\"name\":\"Alice\",\"age\":25}";

const std::string RESPONSE =
    "HTTP/1.1 200 OK\r\n"
    "Server: nginx\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 11\r\n"
    "\r\n"
    "Hello World";

const std::string ERROR_RESPONSE =
    "HTTP/1.1 404 Not Found\r\n"
    "Server: nginx\r\n"
    "Content-Length: 9\r\n"
    "\r\n"
    "Not Found";

// ============================================================================
// 测试 1: GET 请求解析
// ============================================================================

TEST_CASE("HTTP Parser - GET Request Parsing") {
    HttpRequestParser parser;
    bool success = parser.parse(GET_REQUEST);

    CHECK(success == true);

    const auto& req = parser.getRequse();
    CHECK(req.getMethod() == "GET");
    CHECK(req.getUrl() == "/api/user?id=123");
    CHECK(req.getVersion() == "HTTP/1.1");
}

TEST_CASE("HTTP Parser - GET Request Headers") {
    HttpRequestParser parser;
    parser.parse(GET_REQUEST);

    const auto& req = parser.getRequse();
    const auto& headers = req.getheader();

    CHECK(headers.find("Host") != headers.end());
    CHECK(headers.find("host") != headers.end());  // 不区分大小写
    CHECK(headers.at("host") == "example.com");
    CHECK(headers.at("user-agent") == "curl/7.68.0");
}

TEST_CASE("HTTP Parser - GET Request Body") {
    HttpRequestParser parser;
    parser.parse(GET_REQUEST);

    const auto& req = parser.getRequse();
    CHECK(req.getBody().empty() == true);
}

// ============================================================================
// 测试 2: POST 请求解析
// ============================================================================

TEST_CASE("HTTP Parser - POST Request Parsing") {
    HttpRequestParser parser;
    bool success = parser.parse(POST_REQUEST);

    CHECK(success == true);

    const auto& req = parser.getRequse();
    CHECK(req.getMethod() == "POST");
    CHECK(req.getUrl() == "/api/user");
    CHECK(req.getVersion() == "HTTP/1.1");
}

TEST_CASE("HTTP Parser - POST Request Headers") {
    HttpRequestParser parser;
    parser.parse(POST_REQUEST);

    const auto& req = parser.getRequse();
    const auto& headers = req.getheader();

    CHECK(headers.at("content-type") == "application/json");
    CHECK(headers.at("content-length") == "27");
}

TEST_CASE("HTTP Parser - POST Request Body") {
    HttpRequestParser parser;
    parser.parse(POST_REQUEST);

    const auto& req = parser.getRequse();
    std::string body(req.getBody().begin(), req.getBody().end());

    CHECK(body.empty() == false);
    CHECK(body.find("Alice") != std::string::npos);
    CHECK(body.find("age") != std::string::npos);
}

// ============================================================================
// 测试 3: HTTP 响应解析
// ============================================================================

TEST_CASE("HTTP Parser - Response Parsing") {
    ResponseParser parser;
    bool success = parser.parse(RESPONSE);

    CHECK(success == true);

    const auto& resp = parser.getResponse();
    CHECK(resp.getStatusCode() == 200);
}

TEST_CASE("HTTP Parser - Response Headers") {
    ResponseParser parser;
    parser.parse(RESPONSE);

    const auto& resp = parser.getResponse();
    const auto& headers = resp.getHeaders();

    CHECK(headers.find("Server") != headers.end());
    CHECK(headers.at("Server") == "nginx");
    CHECK(headers.at("Content-Type") == "application/json");
}

TEST_CASE("HTTP Parser - Response Body") {
    ResponseParser parser;
    parser.parse(RESPONSE);

    const auto& resp = parser.getResponse();
    std::string body(resp.getBody().begin(), resp.getBody().end());

    CHECK(body == "Hello World");
}

// ============================================================================
// 测试 4: 错误响应处理
// ============================================================================

TEST_CASE("HTTP Parser - 404 Response") {
    ResponseParser parser;
    bool success = parser.parse(ERROR_RESPONSE);

    CHECK(success == true);

    const auto& resp = parser.getResponse();
    CHECK(resp.getStatusCode() == 404);

    std::string body(resp.getBody().begin(), resp.getBody().end());
    CHECK(body == "Not Found");
}

// ============================================================================
// 测试 5: 请求路径和参数
// ============================================================================

TEST_CASE("HTTP Parser - Request Path Extraction") {
    HttpRequestParser parser;
    parser.parse(GET_REQUEST);

    const auto& req = parser.getRequse();
    CHECK(req.getPath() == "/api/user");
}

// ============================================================================
// 测试 6: 多个 Header 处理
// ============================================================================

TEST_CASE("HTTP Parser - Multiple Headers") {
    const std::string multi_header_req =
        "POST /api/data HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Authorization: Bearer token123\r\n"
        "X-Custom-Header: custom-value\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    HttpRequestParser parser;
    bool success = parser.parse(multi_header_req);

    CHECK(success == true);

    const auto& req = parser.getRequse();
    const auto& headers = req.getheader();

    CHECK(headers.size() >= 5);
    CHECK(headers.at("authorization") == "Bearer token123");
    CHECK(headers.at("x-custom-header") == "custom-value");
}

// ============================================================================
// 测试 7: 内容长度处理
// ============================================================================

TEST_CASE("HTTP Parser - Content Length") {
    const std::string req =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";

    HttpRequestParser parser;
    bool success = parser.parse(req);

    CHECK(success == true);

    const auto& request = parser.getRequse();
    std::string body(request.getBody().begin(), request.getBody().end());
    CHECK(body == "Hello, World!");
}

// ============================================================================
// 测试 8: 不同的 HTTP 方法
// ============================================================================

TEST_CASE("HTTP Parser - PUT Request") {
    const std::string put_req =
        "PUT /api/user/123 HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    HttpRequestParser parser;
    bool success = parser.parse(put_req);

    CHECK(success == true);
    CHECK(parser.getRequse().getMethod() == "PUT");
    CHECK(parser.getRequse().getUrl() == "/api/user/123");
}

TEST_CASE("HTTP Parser - DELETE Request") {
    const std::string delete_req =
        "DELETE /api/user/123 HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    HttpRequestParser parser;
    bool success = parser.parse(delete_req);

    CHECK(success == true);
    CHECK(parser.getRequse().getMethod() == "DELETE");
}

// ============================================================================
// 测试 9: 响应状态码
// ============================================================================

TEST_CASE("HTTP Parser - Various Status Codes") {
    std::vector<int> status_codes = {200, 201, 204, 301, 302, 400, 401, 403, 404, 500, 502, 503};

    for (int code : status_codes) {
        std::string resp = "HTTP/1.1 " + std::to_string(code) + " Status\r\n\r\n";
        ResponseParser parser;
        bool success = parser.parse(resp);

        CHECK(success == true);
        CHECK(parser.getResponse().getStatusCode() == code);
    }
}

// ============================================================================
// 测试 10: 大型 Body 处理
// ============================================================================

TEST_CASE("HTTP Parser - Large Body") {
    std::string large_body(1000, 'A');  // 1000 字符的 body
    std::string req =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: " + std::to_string(large_body.size()) + "\r\n"
        "\r\n" + large_body;

    HttpRequestParser parser;
    bool success = parser.parse(req);

    CHECK(success == true);

    std::string body(parser.getRequse().getBody().begin(),
                     parser.getRequse().getBody().end());
    CHECK(body == large_body);
    CHECK(body.size() == 1000);
}

// ============================================================================
// 测试总结
// ============================================================================

/*
 * HTTP 解析器单元测试总结:
 *
 * ✓ GET 请求解析 (method, URL, version, headers, body)
 * ✓ POST 请求解析 (含 JSON body)
 * ✓ PUT 请求解析
 * ✓ DELETE 请求解析
 * ✓ HTTP 响应解析 (status code, headers, body)
 * ✓ 错误响应处理 (404, 错误码)
 * ✓ Header 处理 (多个 header，不区分大小写)
 * ✓ Body 处理 (有/无 body，大型 body)
 * ✓ Content-Length 处理
 * ✓ 路径和参数提取
 * ✓ 各种 HTTP 状态码
 *
 * 编译运行:
 *   g++ -std=c++20 http_parser_test.cc -o http_parser_test -I../../src
 *   ./http_parser_test
 */
