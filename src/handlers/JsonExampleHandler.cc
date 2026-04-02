// ============================================================================
// JsonExampleHandler.cc - JSON 处理示例实现
// ============================================================================

#include "JsonExampleHandler.h"
#include "../utils/logger.hpp"

using namespace yoyo::http;

namespace yoyo {
namespace handler {

void JsonExampleHandler::handleJsonUserData(
    const HttpRequest& req,
    HttpResponse& res) {
    try {
        // 检查是否为 JSON 请求
        if (!JsonUtils::isJsonRequest(req)) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Content-Type must be application/json");
            return;
        }

        // 解析 JSON 请求体
        JsonFiled jsonBody = JsonUtils::parseJsonFromRequest(req);
        if (jsonBody.isNull()) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Invalid JSON format in request body");
            return;
        }

        // 验证数据
        std::string errorMsg;
        if (!validateUserData(jsonBody, errorMsg)) {
            res = JsonUtils::createJsonErrorResponse(400, errorMsg);
            return;
        }

        // 提取 JSON 数据
        std::string username = JsonUtils::getJsonString(jsonBody, "username");
        std::string email = JsonUtils::getJsonString(jsonBody, "email");
        int age = JsonUtils::getJsonInt(jsonBody, "age", 0);

        // 构建成功响应
        JsonFiled::json_object dataObj;
        dataObj["user_id"] = JsonFiled(12345);
        dataObj["username"] = JsonFiled(username);
        dataObj["email"] = JsonFiled(email);
        dataObj["age"] = JsonFiled(age);

        res = JsonUtils::createJsonSuccessResponse(
            JsonFiled(dataObj),
            "User data processed successfully");

    } catch (const std::exception& e) {
        res = JsonUtils::createJsonErrorResponse(
            500,
            std::string("Internal server error: ") + e.what());
    }
}

void JsonExampleHandler::handleJsonGameStats(
    const HttpRequest& req,
    HttpResponse& res) {
    try {
        if (!JsonUtils::isJsonRequest(req)) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Content-Type must be application/json");
            return;
        }

        JsonFiled jsonBody = JsonUtils::parseJsonFromRequest(req);
        if (jsonBody.isNull()) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Invalid JSON format in request body");
            return;
        }

        std::string errorMsg;
        if (!validateGameStats(jsonBody, errorMsg)) {
            res = JsonUtils::createJsonErrorResponse(400, errorMsg);
            return;
        }

        // 提取游戏统计数据
        int userId = JsonUtils::getJsonInt(jsonBody, "user_id");
        std::string gameType = JsonUtils::getJsonString(jsonBody, "game_type");
        int score = JsonUtils::getJsonInt(jsonBody, "score");
        int duration = JsonUtils::getJsonInt(jsonBody, "duration", 0);

        // 模拟数据库操作
        JsonFiled::json_object resultObj;
        resultObj["user_id"] = JsonFiled(userId);
        resultObj["game_type"] = JsonFiled(gameType);
        resultObj["score"] = JsonFiled(score);
        resultObj["duration"] = JsonFiled(duration);
        resultObj["recorded"] = JsonFiled(true);
        resultObj["timestamp"] = JsonFiled(12345678);

        res = JsonUtils::createJsonSuccessResponse(
            JsonFiled(resultObj),
            "Game statistics recorded successfully");

    } catch (const std::exception& e) {
        res = JsonUtils::createJsonErrorResponse(
            500,
            std::string("Internal server error: ") + e.what());
    }
}

void JsonExampleHandler::handleJsonLogin(
    const HttpRequest& req,
    HttpResponse& res) {
    try {
        if (!JsonUtils::isJsonRequest(req)) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Content-Type must be application/json");
            return;
        }

        JsonFiled jsonBody = JsonUtils::parseJsonFromRequest(req);
        if (jsonBody.isNull()) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Invalid JSON format");
            return;
        }

        // 提取用户名和密码
        std::string username = JsonUtils::getJsonString(jsonBody, "username");
        std::string password = JsonUtils::getJsonString(jsonBody, "password");

        if (username.empty() || password.empty()) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Username and password are required");
            return;
        }

        // TODO: 调用 UserService 进行认证
        // 这里简单模拟成功登录
        JsonFiled::json_object loginObj;
        loginObj["user_id"] = JsonFiled(123);
        loginObj["username"] = JsonFiled(username);
        loginObj["token"] = JsonFiled(std::string("token_" + username));
        loginObj["login_time"] = JsonFiled(1234567890);

        res = JsonUtils::createJsonSuccessResponse(
            JsonFiled(loginObj),
            "Login successful");

    } catch (const std::exception& e) {
        res = JsonUtils::createJsonErrorResponse(
            500,
            std::string("Login error: ") + e.what());
    }
}

void JsonExampleHandler::handleJsonRegister(
    const HttpRequest& req,
    HttpResponse& res) {
    try {
        if (!JsonUtils::isJsonRequest(req)) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Content-Type must be application/json");
            return;
        }

        JsonFiled jsonBody = JsonUtils::parseJsonFromRequest(req);
        if (jsonBody.isNull()) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Invalid JSON format");
            return;
        }

        std::string username = JsonUtils::getJsonString(jsonBody, "username");
        std::string email = JsonUtils::getJsonString(jsonBody, "email");
        std::string password = JsonUtils::getJsonString(jsonBody, "password");

        if (username.empty() || email.empty() || password.empty()) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Username, email and password are required");
            return;
        }

        // TODO: 调用 UserService 进行注册
        JsonFiled::json_object registerObj;
        registerObj["user_id"] = JsonFiled(124);
        registerObj["username"] = JsonFiled(username);
        registerObj["email"] = JsonFiled(email);
        registerObj["created_at"] = JsonFiled(1234567890);

        res = JsonUtils::createJsonSuccessResponse(
            JsonFiled(registerObj),
            "Registration successful");

    } catch (const std::exception& e) {
        res = JsonUtils::createJsonErrorResponse(
            500,
            std::string("Registration error: ") + e.what());
    }
}

void JsonExampleHandler::handleJsonBatchOperations(
    const HttpRequest& req,
    HttpResponse& res) {
    try {
        if (!JsonUtils::isJsonRequest(req)) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Content-Type must be application/json");
            return;
        }

        JsonFiled jsonBody = JsonUtils::parseJsonFromRequest(req);
        if (jsonBody.isNull() || !jsonBody.isMember("operations")) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "Invalid JSON format: 'operations' field is required");
            return;
        }

        // 获取操作数组
        JsonFiled operations = jsonBody["operations"];
        if (!operations.isArray()) {
            res = JsonUtils::createJsonErrorResponse(
                400,
                "operations must be an array");
            return;
        }

        // 处理每个操作
        std::vector<JsonFiled> results;
        auto opArray = operations.asArray();

        for (const auto& op : opArray) {
            if (!op.isObject()) continue;

            std::string action = JsonUtils::getJsonString(op, "action");
            std::string gameId = JsonUtils::getJsonString(op, "game_id");
            int score = JsonUtils::getJsonInt(op, "score");

            JsonFiled::json_object resultObj;
            resultObj["action"] = JsonFiled(action);
            resultObj["game_id"] = JsonFiled(gameId);
            resultObj["score"] = JsonFiled(score);
            resultObj["status"] = JsonFiled(std::string("processed"));

            results.push_back(JsonFiled(resultObj));
        }

        // 返回结果数组
        JsonFiled::json_object response;
        response["total"] = JsonFiled(static_cast<int>(results.size()));
        response["processed"] = JsonFiled(static_cast<int>(results.size()));
        response["results"] = JsonFiled(results);

        res = JsonUtils::createJsonSuccessResponse(
            JsonFiled(response),
            "Batch operations completed");

    } catch (const std::exception& e) {
        res = JsonUtils::createJsonErrorResponse(
            500,
            std::string("Batch operation error: ") + e.what());
    }
}

bool JsonExampleHandler::validateUserData(
    const JsonFiled& json,
    std::string& errorMsg) {
    if (!json.isObject()) {
        errorMsg = "Request body must be a JSON object";
        return false;
    }

    // 检查必需字段
    if (!json.isMember("username") || !json["username"].isString()) {
        errorMsg = "Username is required and must be a string";
        return false;
    }

    if (!json.isMember("email") || !json["email"].isString()) {
        errorMsg = "Email is required and must be a string";
        return false;
    }

    // 验证用户名长度
    std::string username = json["username"].asString();
    if (username.length() < 3 || username.length() > 20) {
        errorMsg = "Username must be between 3 and 20 characters";
        return false;
    }

    // 验证邮箱格式（简单检查）
    std::string email = json["email"].asString();
    if (email.find('@') == std::string::npos) {
        errorMsg = "Invalid email format";
        return false;
    }

    return true;
}

bool JsonExampleHandler::validateGameStats(
    const JsonFiled& json,
    std::string& errorMsg) {
    if (!json.isObject()) {
        errorMsg = "Request body must be a JSON object";
        return false;
    }

    if (!json.isMember("user_id") || !json["user_id"].isInt()) {
        errorMsg = "user_id is required and must be an integer";
        return false;
    }

    if (!json.isMember("game_type") || !json["game_type"].isString()) {
        errorMsg = "game_type is required and must be a string";
        return false;
    }

    if (!json.isMember("score") || !json["score"].isInt()) {
        errorMsg = "score is required and must be an integer";
        return false;
    }

    // 验证游戏类型
    std::string gameType = json["game_type"].asString();
    static const std::vector<std::string> validGames = {
        "tetris", "fighter", "fishing"
    };
    if (std::find(validGames.begin(), validGames.end(), gameType) ==
        validGames.end()) {
        errorMsg = "Invalid game_type: " + gameType;
        return false;
    }

    // 验证分数范围
    int score = json["score"].asInt();
    if (score < 0 || score > 999999) {
        errorMsg = "Score must be between 0 and 999999";
        return false;
    }

    return true;
}

}  // namespace handler
}  // namespace yoyo
