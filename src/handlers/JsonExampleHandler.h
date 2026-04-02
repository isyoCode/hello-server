// ============================================================================
// JsonExampleHandler.h - JSON 处理示例
// ============================================================================
// 演示如何处理 JSON 格式的 HTTP 请求
// 包括 JSON 请求解析、验证和 JSON 响应生成
// ============================================================================

#ifndef YOYO_JSON_EXAMPLE_HANDLER_H_
#define YOYO_JSON_EXAMPLE_HANDLER_H_

#include <string>
#include "../http/request.h"
#include "../http/response.h"
#include "../http/json_utils.h"

namespace yoyo {
namespace handler {

class JsonExampleHandler {
public:
    /**
     * @brief 处理 JSON 格式的用户数据
     *
     * 请求格式:
     * POST /api/user/json
     * Content-Type: application/json
     *
     * {
     *   "username": "alice",
     *   "email": "alice@example.com",
     *   "age": 25
     * }
     *
     * 响应格式:
     * {
     *   "status": "ok",
     *   "message": "User data processed successfully",
     *   "data": {
     *     "user_id": 123,
     *     "username": "alice",
     *     "email": "alice@example.com"
     *   }
     * }
     */
    static void handleJsonUserData(
        const http::HttpRequest& req,
        http::HttpResponse& res);

    /**
     * @brief 处理 JSON 格式的游戏统计数据
     *
     * 请求格式:
     * POST /api/game/stats
     * Content-Type: application/json
     *
     * {
     *   "user_id": 123,
     *   "game_type": "tetris",
     *   "score": 1500,
     *   "duration": 300
     * }
     */
    static void handleJsonGameStats(
        const http::HttpRequest& req,
        http::HttpResponse& res);

    /**
     * @brief 处理 JSON 格式的登录请求
     *
     * 请求格式:
     * POST /api/login
     * Content-Type: application/json
     *
     * {
     *   "username": "alice",
     *   "password": "alice123"
     * }
     */
    static void handleJsonLogin(
        const http::HttpRequest& req,
        http::HttpResponse& res);

    /**
     * @brief 处理 JSON 格式的注册请求
     *
     * 请求格式:
     * POST /api/register
     * Content-Type: application/json
     *
     * {
     *   "username": "bob",
     *   "email": "bob@example.com",
     *   "password": "bob123"
     * }
     */
    static void handleJsonRegister(
        const http::HttpRequest& req,
        http::HttpResponse& res);

    /**
     * @brief 处理复杂的嵌套 JSON 数据
     *
     * 请求格式:
     * POST /api/game/batch
     * Content-Type: application/json
     *
     * {
     *   "operations": [
     *     {
     *       "action": "update",
     *       "game_id": "tetris",
     *       "score": 1500
     *     },
     *     {
     *       "action": "submit",
     *       "game_id": "fishing",
     *       "score": 2000
     *     }
     *   ]
     * }
     */
    static void handleJsonBatchOperations(
        const http::HttpRequest& req,
        http::HttpResponse& res);

private:
    /**
     * @brief 验证用户数据的有效性
     */
    static bool validateUserData(
        const JsonFiled& json,
        std::string& errorMsg);

    /**
     * @brief 验证游戏统计数据的有效性
     */
    static bool validateGameStats(
        const JsonFiled& json,
        std::string& errorMsg);
};

}  // namespace handler
}  // namespace yoyo

#endif  // YOYO_JSON_EXAMPLE_HANDLER_H_
