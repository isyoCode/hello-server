#include "../src/core/TcpServer.h"

#include "../src/http/request_parser.h"
#include "../src/http/response.h"

#include "../src/router/Router.h"
#include "../src/handlers/Handler.h"

#include "../src/utils/logger.hpp"


int main() {
    using namespace yoyo;

    // ========== 在创建任何对象之前先初始化日志系统 ==========
    Logger::getInstance()
        ->setPrefixPath(".")
        .setLogDirName("server_logs")
        .setLogFileName("tcp_server")
        .setConsle(true)       // 启用控制台输出
        .setColor(true)        // 启用彩色输出
        .setWritefile(true)    // 启用文件输出
        .setRotate(true)       // 启用日志轮转
        .setFileMaxSize(10 * 1024 * 1024);  // 10MB轮转

    Eventloop loop(false);
    InetAddress listenAddr(8888);
    TcpServer server(&loop, listenAddr);
    server.start();

    // 服务器资源配置
    const static std::string STATIC_FILE_ROOT_ABS = "/home/isyo/hello-git/net/resource";

    // 初始化router 
    yoyo::router::Router mainRouter;
    mainRouter.addRouter("GET", "/", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/index.html", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/test_images.html", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));

    // 登录/注册页面
    mainRouter.addRouter("GET", "/login", std::bind(&yoyo::handler::Handler::loadLoginPage, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("POST", "/login", yoyo::handler::Handler::handleLoginPost); // handleLoginPost 不直接加载文件，所以不需要 STATIC_FILE_ROOT_ABS

    mainRouter.addRouter("GET", "/register", std::bind(&yoyo::handler::Handler::loadRegisterPage, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("POST", "/register", yoyo::handler::Handler::handleRegisterPost); // handleRegisterPost 也不需要

    // 仪表盘和游戏页面
    mainRouter.addRouter("GET", "/dashboard", std::bind(&yoyo::handler::Handler::loadDashboardPage, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/game1.html", std::bind(&yoyo::handler::Handler::loadGamePage, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/game2.html", std::bind(&yoyo::handler::Handler::loadGamePage, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    
    // 其他静态资源（CSS, JS, 图片等）
    // 这里的路由需要你根据实际的静态文件路径来添加。
    // 如果你的 Router 支持通配符，例如 "/css/*" 或 "/images/*"，那会更方便。
    // 如果不支持，你需要为每一个文件添加精确路由。
    mainRouter.addRouter("GET", "/css/style.css", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));

    // 图片资源路由
    mainRouter.addRouter("GET", "/images/1.png", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/images/3.jpg", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/images/scenery1.jpg", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/images/scenery2.jpg", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/images/scenery3.jpg", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/images/scenery4.jpg", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/images/background.jpg", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));
    mainRouter.addRouter("GET", "/images/someone.jpg", std::bind(&yoyo::handler::Handler::loadStaticFiles, STATIC_FILE_ROOT_ABS, std::placeholders::_1, std::placeholders::_2));

    // 游戏统计 API 路由
    mainRouter.addRouter("POST", "/api/save-game-stats", yoyo::handler::Handler::handleSaveGameStats);
    mainRouter.addRouter("GET", "/api/user/game-stats", yoyo::handler::Handler::handleGetGameStats);


    server.setMessageCallback([&mainRouter](const TcpConnection::TcpConnectionPtr conn, const std::string& smsg){
        // 解析HTTP请求
        yoyo::http::HttpRequestParser parser;
        bool bSuccess = parser.parse(smsg);

        yoyo::http::HttpResponse response;
        response.setVersion("HTTP/1.1");

        if(bSuccess) {
            const auto& request = parser.getRequse();

            // 记录请求日志
            LOGI("HTTP " + request.getMethod() + " " + request.getPath() + " from fd=" + std::to_string(conn->fd()));

            // 使用路由器处理请求
            bool routed = mainRouter.route(request, response);

            if(!routed) {
                // 路由未找到，尝试作为静态文件处理
                LOGW("Route not found, trying static file: " + request.getPath());

                // 尝试加载静态文件
                yoyo::handler::Handler::loadStaticFiles(STATIC_FILE_ROOT_ABS, request, response);

                // 如果静态文件加载失败（返回404），保持404状态
                if(response.getStatusCode() != 200) {
                    LOGW("Static file not found: " + request.getPath());
                }
            }

            // 处理 Connection 头
            auto header = request.getheader();
            auto connIter = header.find("connection");
            if(connIter != header.end() && connIter->second == "close") {
                response.addHeader("Connection", "close");
            } else {
                response.addHeader("Connection", "keep-alive");
            }

            // 添加 CORS 支持
            std::string requestOrigin = "*";
            auto originIter = header.find("origin");
            if (originIter != header.end()) {
                requestOrigin = originIter->second;
            }
            response.addHeader("Access-Control-Allow-Origin", requestOrigin == "null" ? "*" : requestOrigin);

            // 处理 OPTIONS 预检请求
            if (request.getMethod() == "OPTIONS") {
                response.setStatusCode(200);
                response.setReason("OK");
                response.addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
                response.addHeader("Access-Control-Allow-Headers", "Content-Type, Accept");
                response.addHeader("Access-Control-Max-Age", "86400");
                std::vector<yoyo::http::HttpResponse::byte> emptyBody;
                response.setBody(emptyBody);
            }
        } else {
            LOGE("HTTP request parse failed from fd=" + std::to_string(conn->fd()));
            response.setStatusCode(400);
            response.setReason("Bad Request");
            std::string errorMsg = "Bad Request";
            std::vector<yoyo::http::HttpResponse::byte> body(errorMsg.begin(), errorMsg.end());
            response.setBody(body);
            response.addHeader("Content-Type", "text/plain");
            response.addHeader("Connection", "close");
        }

        // 发送响应
        if (response.hasFile()) {
            // 使用 sendfile 发送大文件
            const auto& fileInfo = response.getFileInfo();
            std::string headerStr = response.serializeHeaders(fileInfo->size);
            conn->sendFile(headerStr, fileInfo->fd, fileInfo->size);
        } else {
            // 普通响应，直接序列化发送
            std::string responseStr = response.serialize();
            conn->send(responseStr);
        }
    });

    LOGI("Starting event loop...");
    loop.loop();

    LOGI("Server shutting down");
    return 0;
}