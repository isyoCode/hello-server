#ifndef yoyo_Handlers_h_
#define yoyo_Handlers_h_

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

#include "../http/request.h"
#include "../http/response.h"

namespace yoyo {
namespace handler {

class Handler {
public:
    // 加载静态文件资源
    static void loadStaticFiles(const std::string& baseDir, const http::HttpRequest& req, http::HttpResponse& res);

    // 登录认证相关
    static void loadLoginPage(const std::string& sBaseDir, const http::HttpRequest& req, http::HttpResponse& res);
    static void handleLoginPost(const http::HttpRequest& req, http::HttpResponse& res);
    static void loadRegisterPage(const std::string& sBaseDir, const http::HttpRequest& req, http::HttpResponse& res);
    static void handleRegisterPost(const http::HttpRequest& req, http::HttpResponse& res);

    // 应用页面
    static void loadDashboardPage(const std::string& sBaseDir, const http::HttpRequest& req, http::HttpResponse& res);
    static void loadGamePage(const std::string& sBaseDir, const http::HttpRequest& req, http::HttpResponse& res); // 通用游戏页面处理

    // 游戏统计 API
    static void handleSaveGameStats(const http::HttpRequest& req, http::HttpResponse& res);
    static void handleGetGameStats(const http::HttpRequest& req, http::HttpResponse& res);

private:
    // 辅助函数：根据文件扩展名获取 Content-Type
    static std::string getContentType(const std::string& filePath);
    // 辅助函数：读取文件内容
    static std::vector<unsigned char> readFileContent(const std::string& filePath);
    // 辅助函数：设置重定向响应
    static void setRedirectResponse(http::HttpResponse& res, const std::string& location);


};

} // end of namespace handler
} // end of namespace yoyo




#endif