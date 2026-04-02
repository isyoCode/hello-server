
#include "../database/UserService.h"
#include "../utils/logger.hpp"
#include "Handler.h"

// 根据 RFC 3986，这些字符不需要编码
// A-Z a-z 0-9 - _ . ~
inline static bool isUnreserved(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' ||
         c == '.' || c == '~';
}

inline static std::string urlEncode(const std::string& str) {
  std::string encoded_str;
  encoded_str.reserve(str.length() * 1.5);  // 预分配一些空间

  for (char c : str) {
    if (isUnreserved(c)) {
      encoded_str += c;
    } else if (c == ' ') {
      encoded_str += '+';  // 空格编码为 '+'
    } else {
      // 所有其他字符编码为 %HH
      encoded_str += '%';
      encoded_str +=
          "0123456789ABCDEF"[(static_cast<unsigned char>(c) >> 4) & 0xF];
      encoded_str += "0123456789ABCDEF"[(static_cast<unsigned char>(c)) & 0xF];
    }
  }
  return encoded_str;
}

inline static std::string urlDecode(const std::string& str) {
  std::string decoded_str;
  decoded_str.reserve(str.length());  // 预分配空间

  for (size_t i = 0; i < str.length(); ++i) {
    char c = str[i];
    if (c == '+') {
      decoded_str += ' ';  // '+' 解码为空格
    } else if (c == '%') {
      if (i + 2 < str.length() &&
          std::isxdigit(static_cast<unsigned char>(str[i + 1])) &&
          std::isxdigit(static_cast<unsigned char>(str[i + 2]))) {
        // 解析 %HH 编码
        std::string hex_byte = str.substr(i + 1, 2);
        decoded_str += static_cast<char>(std::stoi(hex_byte, nullptr, 16));
        i += 2;  // 跳过两个十六进制字符
      } else {
        // 无效的 % 编码，可以根据需要处理，这里选择保留 %
        decoded_str += c;
      }
    } else {
      decoded_str += c;
    }
  }
  return decoded_str;
}

inline static std::map<std::string, std::string> parseUrlEncodedString(
    const std::string& encodedString) {
  std::map<std::string, std::string> params;
  size_t start = 0;
  size_t end = encodedString.find('&');

  while (start < encodedString.length()) {
    std::string pair_str = encodedString.substr(start, end - start);
    size_t eq_pos = pair_str.find('=');

    if (eq_pos != std::string::npos) {
      std::string key = pair_str.substr(0, eq_pos);
      std::string value = pair_str.substr(eq_pos + 1);
      params[urlDecode(key)] = urlDecode(value);  // 解码键和值
    } else {
      // 如果没有等号，整个字符串作为键，值为空
      params[urlDecode(pair_str)] = "";
    }

    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
    end = encodedString.find('&', start);
  }
  return params;
}

namespace yoyo {
namespace handler {

// 辅助函数：根据文件扩展名获取 Content-Type
std::string Handler::getContentType(const std::string& filePath) {
  std::filesystem::path p(filePath);
  std::string ext = p.extension().string();

  if (ext == ".html" || ext == ".htm") return "text/html";
  if (ext == ".css") return "text/css";
  if (ext == ".js") return "application/javascript";
  if (ext == ".json") return "application/json";
  if (ext == ".png") return "image/png";
  if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
  if (ext == ".gif") return "image/gif";
  if (ext == ".ico") return "image/x-icon";
  if (ext == ".svg") return "image/svg+xml";
  if (ext == ".pdf") return "application/pdf";
  if (ext == ".xml") return "application/xml";
  // 默认
  return "application/octet-stream";
}

// 辅助函数：读取文件内容
std::vector<unsigned char> Handler::readFileContent(
    const std::string& filePath) {
  std::ifstream file(filePath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    return {};  // 返回空向量表示读取失败
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<unsigned char> buffer(size);
  if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    return buffer;
  }
  return {};  // 读取失败
}

// 辅助函数：设置重定向响应
void Handler::setRedirectResponse(http::HttpResponse& res,
                                  const std::string& location) {
  res.setVersion("HTTP/1.1");
  res.setStatusCode(302);  // Found
  res.setReason("Found");
  res.addHeader("Location", location);
  std::string body =
      "<html><head><meta http-equiv=\"refresh\" content=\"0;url=" + location +
      "\"></head><body>";
  res.setBody(std::vector<unsigned char>(body.begin(), body.end()));
}

// 加载静态文件资源
void Handler::loadStaticFiles(const std::string& baseDir,
                              const http::HttpRequest& req,
                              http::HttpResponse& res) {
  std::string requestPath = req.getPath();
  if (requestPath == "/") {
    requestPath = "/index.html";  // 默认首页
  }
  std::string filePath = baseDir + requestPath;

  LOGD("Loading static file: " + requestPath);

  // 安全检查：防止路径遍历攻击
  if (filePath.find("..") != std::string::npos) {
    LOGW("Path traversal attack detected: " + requestPath);
    res.setStatusCode(400);
    res.setReason("Bad Request");
    return;
  }

  if (!std::filesystem::exists(filePath) ||
      !std::filesystem::is_regular_file(filePath)) {
    LOGW("File not found: " + filePath);
    res.setVersion("HTTP/1.1");
    res.setStatusCode(404);
    res.setReason("Not Found");
    res.addHeader("Content-Type", "text/html");
    res.setBody("<h1>404 Not Found</h1><p>The requested file " + requestPath +
                " was not found.</p>");
    return;
  }

  // 获取文件大小
  size_t fileSize = std::filesystem::file_size(filePath);
  // 使用 sendfile 优化大文件传输
  constexpr size_t SENDFILE_THRESHOLD = 512 * 1024;  // 512KB 阈值

  res.setVersion("HTTP/1.1");
  res.setStatusCode(200);
  res.setReason("OK");
  res.addHeader("Content-Type", getContentType(filePath));

  // 大文件使用 sendfile 零拷贝传输
  if (fileSize > SENDFILE_THRESHOLD) {
    LOGI("Using sendfile for large file: " + requestPath + " (" +
         std::to_string(fileSize) + " bytes)");
    // 打开文件获取文件描述符
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
      LOGE("Failed to open file: " + filePath +
           " errno=" + std::to_string(errno));
      res.setStatusCode(500);
      res.setReason("Internal Server Error");
      res.addHeader("Content-Type", "text/html");
      res.setBody("<h1>500 Internal Server Error</h1><p>Could not open file " +
                  requestPath + ".</p>");
      return;
    }
    // 设置文件信息，供后续 sendfile 使用
    res.setFileInfo(fd, fileSize);
  } else {
    LOGD("Reading small file to memory: " + requestPath + " (" +
         std::to_string(fileSize) + " bytes)");
    // 小文件直接读取到内存
    std::vector<unsigned char> content = readFileContent(filePath);
    if (content.empty()) {
      LOGE("Failed to read file: " + filePath);
      res.setStatusCode(500);
      res.setReason("Internal Server Error");
      res.addHeader("Content-Type", "text/html");
      res.setBody("<h1>500 Internal Server Error</h1><p>Could not read file " +
                  requestPath + ".</p>");
      return;
    }
    res.setBody(content);
  }
  return;
}
// 登录认证相关
void Handler::loadLoginPage(const std::string& sBaseDir,
                            const http::HttpRequest& req,
                            http::HttpResponse& res) {
  const http::HttpRequest tmpReq("GET", "", "/index.html", {});
  loadStaticFiles(sBaseDir, tmpReq, res);
}

void Handler::handleLoginPost(const http::HttpRequest& req,
                              http::HttpResponse& res) {
  std::string rawBody(req.getBody().begin(), req.getBody().end());
  std::map<std::string, std::string> params = ::parseUrlEncodedString(rawBody);

  std::string username = params["username"];
  std::string password = params["password"];

  LOGI("Login attempt: username=" + username);

  // 使用 UserService 进行真实的数据库认证
  auto userService = yoyo::database::UserService::getInstance();
  auto result = userService->authenticateUser(username, password);

  if (result.success) {
    LOGI("Login successful: username=" + username);
#ifdef YOYODHANDLEEBUG
    std::cout << "Login successful for user: " << username << std::endl;
#endif
    setRedirectResponse(res, "/game_selection.html");
  } else {
    LOGW("Login failed: username=" + username + " reason=" + result.message);
#ifdef YOYODHANDLEEBUG
    std::cout << "Login failed for user: " << username << " (" << result.message
              << ")" << std::endl;
#endif
    res.setVersion("HTTP/1.1");
    res.setStatusCode(401);
    res.setReason("Unauthorized");
    res.addHeader("Content-Type", "text/html");
    res.setBody("<h1>Login Failed</h1><p>" + result.message +
                " <a href=\"/login\">Try again</a></p>");
  }
}

void Handler::loadRegisterPage(const std::string& sBaseDir,
                               const http::HttpRequest& req,
                               http::HttpResponse& res) {
  const http::HttpRequest tmpReq("GET", "", "/register.html", {});
  loadStaticFiles(sBaseDir, tmpReq, res);
}
void Handler::handleRegisterPost(const http::HttpRequest& req,
                                 http::HttpResponse& res) {
  std::string rawBody(req.getBody().begin(), req.getBody().end());
  std::map<std::string, std::string> params = ::parseUrlEncodedString(rawBody);

  std::string username = params["username"];
  std::string email = params["email"];
  std::string password = params["password"];
  std::string confirmPassword = params["confirm_password"];

  LOGI("Registration attempt: username=" + username + " email=" + email);

  // 基本验证
  if (username.empty() || email.empty() || password.empty()) {
    LOGW("Registration failed: missing fields");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(400);
    res.setReason("Bad Request");
    res.addHeader("Content-Type", "text/html");
    res.setBody(
        "<h1>Registration Failed</h1><p>Username, email and password are "
        "required. <a href=\"/register\">Try again</a></p>");
    return;
  }

  if (password != confirmPassword) {
    LOGW("Registration failed: passwords don't match");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(400);
    res.setReason("Bad Request");
    res.addHeader("Content-Type", "text/html");
    res.setBody(
        "<h1>Registration Failed</h1><p>Passwords don't match. <a "
        "href=\"/register\">Try again</a></p>");
    return;
  }

  // 使用 UserService 进行数据库注册
  auto userService = yoyo::database::UserService::getInstance();
  auto result = userService->registerUser(username, email, password);

  if (result.success) {
    LOGI("Registration successful: username=" + username);
#ifdef YOYODHANDLEEBUG
    std::cout << "Registration successful for user: " << username << std::endl;
#endif
    setRedirectResponse(res, "/game_selection.html");
  } else {
    LOGW("Registration failed: username=" + username +
         " reason=" + result.message);
#ifdef YOYODHANDLEEBUG
    std::cout << "Registration failed for user: " << username << " ("
              << result.message << ")" << std::endl;
#endif
    res.setVersion("HTTP/1.1");
    res.setStatusCode(409);
    res.setReason("Conflict");
    res.addHeader("Content-Type", "text/html");
    res.setBody("<h1>Registration Failed</h1><p>" + result.message +
                " <a href=\"/register\">Try again</a></p>");
  }
}

// 应用页面
void Handler::loadDashboardPage(const std::string& sBaseDir,
                                const http::HttpRequest& req,
                                http::HttpResponse& res) {
  const http::HttpRequest tmpReq("GET", "", "/dashboard.html", {});
  loadStaticFiles(sBaseDir, tmpReq, res);
}
void Handler::loadGamePage(const std::string& sBaseDir,
                           const http::HttpRequest& req,
                           http::HttpResponse& res) {
  const http::HttpRequest tmpReq("GET", "", req.getPath(), {});
  loadStaticFiles(sBaseDir, tmpReq, res);
}

// ============================================================================
// Game Statistics API Handlers
// ============================================================================

void Handler::handleSaveGameStats(const http::HttpRequest& req,
                                  http::HttpResponse& res) {
  std::string rawBody(req.getBody().begin(), req.getBody().end());
  std::map<std::string, std::string> params = ::parseUrlEncodedString(rawBody);

  std::string userIdStr = params["user_id"];
  std::string gameType = params["game_type"];
  std::string scoreStr = params["score"];

  LOGI("Save game stats: user_id=" + userIdStr + " game_type=" + gameType +
       " score=" + scoreStr);

  if (userIdStr.empty() || gameType.empty() || scoreStr.empty()) {
    res.setVersion("HTTP/1.1");
    res.setStatusCode(400);
    res.setReason("Bad Request");
    res.addHeader("Content-Type", "application/json");
    res.setBody("{\"success\": false, \"message\": \"缺少必要参数\"}");
    return;
  }

  try {
    int score = std::stoi(scoreStr);
    unsigned int userId = std::stoi(userIdStr);

    auto userService = yoyo::database::UserService::getInstance();
    auto result = userService->updateGameStats(userId, gameType, score);

    res.setVersion("HTTP/1.1");
    res.setStatusCode(result.success ? 200 : 400);
    res.setReason(result.success ? "OK" : "Bad Request");
    res.addHeader("Content-Type", "application/json");

    std::string response =
        "{\"success\": " + std::string(result.success ? "true" : "false") +
        ", \"message\": \"" + result.message + "\"}";
    res.setBody(response);

    if (result.success) {
      LOGI("Game stats saved successfully");
    } else {
      LOGW("Game stats save failed: " + result.message);
    }

  } catch (const std::exception& e) {
    LOGE("Exception in handleSaveGameStats: " + std::string(e.what()));
    res.setVersion("HTTP/1.1");
    res.setStatusCode(500);
    res.setReason("Internal Server Error");
    res.addHeader("Content-Type", "application/json");
    res.setBody("{\"success\": false, \"message\": \"服务器错误\"}");
  }
}

void Handler::handleGetGameStats(const http::HttpRequest& req,
                                 http::HttpResponse& res) {
  // 从 URL 查询参数获取 user_id
  std::string path = req.getPath();
  std::string userIdStr = "1";  // 默认值

  // 解析查询参数 ?user_id=123
  size_t qPos = path.find('?');
  if (qPos != std::string::npos) {
    std::string query = path.substr(qPos + 1);
    std::map<std::string, std::string> params = ::parseUrlEncodedString(query);
    if (params.find("user_id") != params.end()) {
      userIdStr = params["user_id"];
    }
  }

  LOGI("Get game stats: user_id=" + userIdStr);

  try {
    unsigned int userId = std::stoi(userIdStr);

    auto userService = yoyo::database::UserService::getInstance();
    auto result = userService->getGameStats(userId);

    res.setVersion("HTTP/1.1");
    res.setStatusCode(result.success ? 200 : 404);
    res.setReason(result.success ? "OK" : "Not Found");
    res.addHeader("Content-Type", "application/json");

    if (result.success) {
      res.setBody(result.message);  // message 包含 JSON 数据
    } else {
      std::string errorJson =
          "{\"success\": false, \"message\": \"" + result.message + "\"}";
      res.setBody(errorJson);
    }

  } catch (const std::exception& e) {
    LOGE("Exception in handleGetGameStats: " + std::string(e.what()));
    res.setVersion("HTTP/1.1");
    res.setStatusCode(500);
    res.setReason("Internal Server Error");
    res.addHeader("Content-Type", "application/json");
    res.setBody("{\"success\": false, \"message\": \"服务器错误\"}");
  }
}

}  // end of namespace handler
}  // end of namespace yoyo
