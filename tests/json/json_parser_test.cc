// ============================================================================
// json_parser_test.cc - JSON 解析器单元测试
// ============================================================================
// 使用 doctest 框架测试 JSON 解析器的各项功能
//
// 测试内容:
//   - 基本类型解析 (null, bool, number, string)
//   - 复杂类型解析 (array, object)
//   - 嵌套结构处理
//   - 特殊字符转义
//   - 错误处理
//   - JSON 序列化
// ============================================================================

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <iostream>
#include <string>
#include "../../src/http/json_parser.hpp"

using namespace yoyo;

// ============================================================================
// 测试 1: 基本类型
// ============================================================================

TEST_CASE("JSON Parser - Null Type") {
    std::string json_str = "null";
    JsonParser parser(json_str);
    JsonFiled result = parser.parser();

    CHECK(result.isNull() == true);
    CHECK(result.getType() == JSONTYPE::JSON_NULL);
}

TEST_CASE("JSON Parser - Boolean Type") {
    std::string json_true = "true";
    std::string json_false = "false";

    JsonParser parser1(json_true);
    JsonFiled result1 = parser1.parser();
    CHECK(result1.isBool() == true);
    CHECK(result1.asBool() == true);

    JsonParser parser2(json_false);
    JsonFiled result2 = parser2.parser();
    CHECK(result2.isBool() == true);
    CHECK(result2.asBool() == false);
}

TEST_CASE("JSON Parser - Number Type") {
    std::string json_int = "42";
    std::string json_negative = "-100";
    std::string json_large = "999999";

    JsonParser parser1(json_int);
    JsonFiled result1 = parser1.parser();
    CHECK(result1.isInt() == true);
    CHECK(result1.asInt() == 42);

    JsonParser parser2(json_negative);
    JsonFiled result2 = parser2.parser();
    CHECK(result2.isInt() == true);
    CHECK(result2.asInt() == -100);

    JsonParser parser3(json_large);
    JsonFiled result3 = parser3.parser();
    CHECK(result3.isInt() == true);
    CHECK(result3.asInt() == 999999);
}

TEST_CASE("JSON Parser - Double Type") {
    std::string json_double1 = "3.14";
    std::string json_double2 = "-2.5";

    JsonParser parser1(json_double1);
    JsonFiled result1 = parser1.parser();
    CHECK(result1.isDouble() == true);
    double val1 = result1.asDouble();
    CHECK(val1 > 3.13);
    CHECK(val1 < 3.15);

    JsonParser parser2(json_double2);
    JsonFiled result2 = parser2.parser();
    CHECK(result2.isDouble() == true);
    double val2 = result2.asDouble();
    CHECK(val2 < -2.4);
    CHECK(val2 > -2.6);
}

TEST_CASE("JSON Parser - String Type") {
    std::string json_str = "\"hello world\"";
    JsonParser parser(json_str);
    JsonFiled result = parser.parser();

    CHECK(result.isString() == true);
    CHECK(result.asString() == "hello world");
}

// ============================================================================
// 测试 2: 数组类型
// ============================================================================

TEST_CASE("JSON Parser - Empty Array") {
    std::string json_array = "[]";
    JsonParser parser(json_array);
    JsonFiled result = parser.parser();

    CHECK(result.isArray() == true);
    CHECK(result.isEmpty() == true);
    CHECK(result.size() == 0);
}

TEST_CASE("JSON Parser - Array with Integers") {
    std::string json_array = "[1, 2, 3, 4, 5]";
    JsonParser parser(json_array);
    JsonFiled result = parser.parser();

    CHECK(result.isArray() == true);
    CHECK(result.size() == 5);
    CHECK(result[0].asInt() == 1);
    CHECK(result[4].asInt() == 5);
}

TEST_CASE("JSON Parser - Mixed Array") {
    std::string json_array = "[1, \"hello\", true, 3.14, null]";
    JsonParser parser(json_array);
    JsonFiled result = parser.parser();

    CHECK(result.isArray() == true);
    CHECK(result.size() == 5);
    CHECK(result[0].isInt() == true);
    CHECK(result[1].isString() == true);
    CHECK(result[2].isBool() == true);
    CHECK(result[3].isDouble() == true);
    CHECK(result[4].isNull() == true);
}

// ============================================================================
// 测试 3: 对象类型
// ============================================================================

TEST_CASE("JSON Parser - Empty Object") {
    std::string json_obj = "{}";
    JsonParser parser(json_obj);
    JsonFiled result = parser.parser();

    CHECK(result.isObject() == true);
    CHECK(result.isEmpty() == true);
    CHECK(result.size() == 0);
}

TEST_CASE("JSON Parser - Simple Object") {
    std::string json_obj = R"({"name": "Alice", "age": 25})";
    JsonParser parser(json_obj);
    JsonFiled result = parser.parser();

    CHECK(result.isObject() == true);
    CHECK(result.size() == 2);
    CHECK(result.isMember("name") == true);
    CHECK(result["name"].asString() == "Alice");
    CHECK(result["age"].asInt() == 25);
}

TEST_CASE("JSON Parser - Object with Different Types") {
    std::string json_obj = R"({"id": 123, "active": true, "score": 9.8, "note": null})";
    JsonParser parser(json_obj);
    JsonFiled result = parser.parser();

    CHECK(result.isObject() == true);
    CHECK(result["id"].isInt() == true);
    CHECK(result["active"].isBool() == true);
    CHECK(result["score"].isDouble() == true);
    CHECK(result["note"].isNull() == true);
}

// ============================================================================
// 测试 4: 嵌套结构
// ============================================================================

TEST_CASE("JSON Parser - Nested Object") {
    std::string json_nested = R"({
        "user": {
            "name": "Bob",
            "address": {
                "city": "Shanghai",
                "country": "China"
            }
        }
    })";

    JsonParser parser(json_nested);
    JsonFiled result = parser.parser();

    CHECK(result.isObject() == true);
    CHECK(result["user"]["name"].asString() == "Bob");
    CHECK(result["user"]["address"]["city"].asString() == "Shanghai");
    CHECK(result["user"]["address"]["country"].asString() == "China");
}

TEST_CASE("JSON Parser - Nested Array") {
    std::string json_nested = "[[1, 2], [3, 4], [5, 6]]";
    JsonParser parser(json_nested);
    JsonFiled result = parser.parser();

    CHECK(result.isArray() == true);
    CHECK(result.size() == 3);
    CHECK(result[0][0].asInt() == 1);
    CHECK(result[1][1].asInt() == 4);
    CHECK(result[2][0].asInt() == 5);
}

TEST_CASE("JSON Parser - Complex Structure") {
    std::string json_complex = R"({
        "company": "TechCorp",
        "employees": [
            {"id": 1, "name": "Alice", "skills": ["C++", "Python"]},
            {"id": 2, "name": "Bob", "skills": ["Java", "Go"]}
        ],
        "founded": 2015
    })";

    JsonParser parser(json_complex);
    JsonFiled result = parser.parser();

    CHECK(result["company"].asString() == "TechCorp");
    CHECK(result["employees"].size() == 2);
    CHECK(result["employees"][0]["name"].asString() == "Alice");
    CHECK(result["employees"][0]["skills"][0].asString() == "C++");
    CHECK(result["employees"][1]["skills"][1].asString() == "Go");
    CHECK(result["founded"].asInt() == 2015);
}

// ============================================================================
// 测试 5: 特殊字符处理
// ============================================================================

TEST_CASE("JSON Parser - Escaped Characters") {
    std::string json_escaped = R"({"text": "Hello \"World\""})";
    JsonParser parser(json_escaped);
    JsonFiled result = parser.parser();

    CHECK(result["text"].asString() == "Hello \"World\"");
}

TEST_CASE("JSON Parser - String with Newlines") {
    std::string json_newline = R"({"text": "Line1\nLine2"})";
    JsonParser parser(json_newline);
    JsonFiled result = parser.parser();

    std::string text = result["text"].asString();
    CHECK(text.find("Line1") != std::string::npos);
    CHECK(text.find("Line2") != std::string::npos);
}

// ============================================================================
// 测试 6: JSON 序列化
// ============================================================================

TEST_CASE("JSON Parser - Serialize to String") {
    JsonFiled::json_object obj;
    obj["name"] = JsonFiled(std::string("Alice"));
    obj["age"] = JsonFiled(25);
    obj["active"] = JsonFiled(true);

    JsonFiled json(obj);
    std::string serialized = json.writeToString();

    CHECK(serialized.find("\"name\"") != std::string::npos);
    CHECK(serialized.find("Alice") != std::string::npos);
    CHECK(serialized.find("25") != std::string::npos);
}

TEST_CASE("JSON Parser - Serialize Array") {
    std::vector<JsonFiled> arr;
    arr.push_back(JsonFiled(1));
    arr.push_back(JsonFiled(std::string("two")));
    arr.push_back(JsonFiled(3.0));

    JsonFiled json(arr);
    std::string serialized = json.writeToString();

    CHECK(serialized.find("[") != std::string::npos);
    CHECK(serialized.find("]") != std::string::npos);
    CHECK(serialized.find("two") != std::string::npos);
}

// ============================================================================
// 测试 7: 错误处理
// ============================================================================

TEST_CASE("JSON Parser - Invalid JSON") {
    std::string invalid_json = "{invalid}";

    try {
        JsonParser parser(invalid_json);
        JsonFiled result = parser.parser();
        CHECK(false);  // 应该抛出异常
    } catch (const JsonParseError& e) {
        CHECK(true);  // 成功捕获异常
    } catch (...) {
        CHECK(false);  // 捕获其他异常
    }
}

TEST_CASE("JSON Parser - Unclosed Brace") {
    std::string invalid_json = "{\"name\": \"Alice\"";

    try {
        JsonParser parser(invalid_json);
        JsonFiled result = parser.parser();
        CHECK(false);  // 应该抛出异常
    } catch (const std::exception& e) {
        CHECK(true);  // 成功捕获异常
    } catch (...) {
        CHECK(true);  // 成功捕获任何异常
    }
}

TEST_CASE("JSON Parser - Empty String") {
    std::string empty_json = "";

    try {
        JsonParser parser(empty_json);
        JsonFiled result = parser.parser();
        CHECK(false);  // 应该抛出异常
    } catch (const std::logic_error& e) {
        CHECK(true);  // 成功捕获异常
    }
}

// ============================================================================
// 测试 8: 数据访问
// ============================================================================

TEST_CASE("JSON Parser - Member Access") {
    std::string json_obj = R"({"user": {"id": 1, "name": "Alice"}})";
    JsonParser parser(json_obj);
    JsonFiled result = parser.parser();

    // 检查成员是否存在
    CHECK(result.isMember("user") == true);
    CHECK(result.isMember("nonexistent") == false);

    // 访问成员
    CHECK(result["user"]["id"].asInt() == 1);
}

TEST_CASE("JSON Parser - Type Operators") {
    std::string json_obj = R"({"count": 42})";
    JsonParser parser(json_obj);
    JsonFiled result = parser.parser();

    // 使用类型转换操作符
    int value = static_cast<int>(result["count"]);
    CHECK(value == 42);
}

// ============================================================================
// 测试总结
// ============================================================================

/*
 * JSON 解析器单元测试总结:
 *
 * ✓ 基本类型测试 (null, bool, number, double, string)
 * ✓ 数组测试 (空数组、整数数组、混合数组)
 * ✓ 对象测试 (空对象、简单对象、复杂对象)
 * ✓ 嵌套结构测试 (嵌套对象、嵌套数组、复杂混合)
 * ✓ 特殊字符处理 (转义字符、换行符)
 * ✓ 序列化测试 (对象序列化、数组序列化)
 * ✓ 错误处理测试 (无效 JSON、缺少符号、空字符串)
 * ✓ 数据访问测试 (成员访问、类型检查、类型转换)
 *
 * 编译运行:
 *   g++ -std=c++20 json_parser_test.cc -o json_parser_test
 *   ./json_parser_test
 */
