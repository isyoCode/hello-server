// ============================================================================
// json_utils.h - JSON 工具模块
// ============================================================================
// 提供 JSON 请求解析和响应生成的便捷函数
// 支持 Content-Type: application/json 的 HTTP 请求处理
// ============================================================================

#ifndef YOYO_JSON_UTILS_H_
#define YOYO_JSON_UTILS_H_

#include <string>
#include <memory>
#include "json_parser.hpp"
#include "request.h"
#include "response.h"

namespace yoyo {
namespace http {

class JsonUtils {
public:
    /**
     * @brief 从 HTTP 请求体中解析 JSON
     * @param request HTTP 请求对象
     * @return 解析后的 JSON 对象，如果解析失败返回 null JSON
     */
    static JsonFiled parseJsonFromRequest(const HttpRequest& request) {
        try {
            const std::string& body = request.getBody();
            if (body.empty()) {
                return JsonFiled();  // 返回 null JSON
            }

            JsonParser parser(body);
            return parser.parser();
        } catch (const std::exception& e) {
            // 日志记录解析错误
            return JsonFiled();  // 返回 null JSON
        }
    }

    /**
     * @brief 检查请求是否为 JSON 格式
     * @param request HTTP 请求对象
     * @return 如果 Content-Type 是 application/json 返回 true
     */
    static bool isJsonRequest(const HttpRequest& request) {
        const auto& contentType = request.getHeader("Content-Type");
        return !contentType.empty() &&
               contentType.find("application/json") != std::string::npos;
    }

    /**
     * @brief 创建 JSON 响应
     * @param statusCode HTTP 状态码
     * @param json JSON 对象
     * @return HTTP 响应对象
     */
    static HttpResponse createJsonResponse(
        int statusCode,
        const JsonFiled& json) {
        HttpResponse response;
        response.setStatusCode(statusCode);
        response.addHeader("Content-Type", "application/json; charset=utf-8");
        response.setBody(json.writeToString());
        return response;
    }

    /**
     * @brief 创建 JSON 错误响应
     * @param statusCode HTTP 状态码
     * @param errorMessage 错误信息
     * @param errorCode 错误代码（可选）
     * @return HTTP 响应对象
     */
    static HttpResponse createJsonErrorResponse(
        int statusCode,
        const std::string& errorMessage,
        int errorCode = -1) {
        JsonFiled::json_object errorObj;
        errorObj["error"] = JsonFiled(errorMessage);
        errorObj["code"] = JsonFiled(errorCode);

        JsonFiled jsonResponse(errorObj);
        return createJsonResponse(statusCode, jsonResponse);
    }

    /**
     * @brief 创建 JSON 成功响应
     * @param data 响应数据
     * @param message 成功信息（可选）
     * @return HTTP 响应对象
     */
    static HttpResponse createJsonSuccessResponse(
        const JsonFiled& data,
        const std::string& message = "success") {
        JsonFiled::json_object successObj;
        successObj["status"] = JsonFiled(std::string("ok"));
        successObj["message"] = JsonFiled(message);
        successObj["data"] = data;

        JsonFiled jsonResponse(successObj);
        return createJsonResponse(200, jsonResponse);
    }

    /**
     * @brief 安全地从 JSON 对象获取字符串值
     * @param json JSON 对象
     * @param key 键名
     * @param defaultValue 默认值
     * @return 字符串值或默认值
     */
    static std::string getJsonString(
        const JsonFiled& json,
        const std::string& key,
        const std::string& defaultValue = "") {
        if (!json.isObject() || !json.isMember(key)) {
            return defaultValue;
        }
        try {
            const JsonFiled& field = json[key];
            if (field.isString()) {
                return field.asString();
            }
        } catch (...) {
            // 类型转换失败，返回默认值
        }
        return defaultValue;
    }

    /**
     * @brief 安全地从 JSON 对象获取整数值
     * @param json JSON 对象
     * @param key 键名
     * @param defaultValue 默认值
     * @return 整数值或默认值
     */
    static int getJsonInt(
        const JsonFiled& json,
        const std::string& key,
        int defaultValue = 0) {
        if (!json.isObject() || !json.isMember(key)) {
            return defaultValue;
        }
        try {
            const JsonFiled& field = json[key];
            if (field.isInt()) {
                return field.asInt();
            }
        } catch (...) {
            // 类型转换失败，返回默认值
        }
        return defaultValue;
    }

    /**
     * @brief 安全地从 JSON 对象获取布尔值
     * @param json JSON 对象
     * @param key 键名
     * @param defaultValue 默认值
     * @return 布尔值或默认值
     */
    static bool getJsonBool(
        const JsonFiled& json,
        const std::string& key,
        bool defaultValue = false) {
        if (!json.isObject() || !json.isMember(key)) {
            return defaultValue;
        }
        try {
            const JsonFiled& field = json[key];
            if (field.isBool()) {
                return field.asBool();
            }
        } catch (...) {
            // 类型转换失败，返回默认值
        }
        return defaultValue;
    }

    /**
     * @brief 安全地从 JSON 对象获取浮点数值
     * @param json JSON 对象
     * @param key 键名
     * @param defaultValue 默认值
     * @return 浮点数值或默认值
     */
    static double getJsonDouble(
        const JsonFiled& json,
        const std::string& key,
        double defaultValue = 0.0) {
        if (!json.isObject() || !json.isMember(key)) {
            return defaultValue;
        }
        try {
            const JsonFiled& field = json[key];
            if (field.isDouble()) {
                return field.asDouble();
            }
        } catch (...) {
            // 类型转换失败，返回默认值
        }
        return defaultValue;
    }
};

}  // namespace http
}  // namespace yoyo

#endif  // YOYO_JSON_UTILS_H_
