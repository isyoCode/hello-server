# 📝 Logger 日志系统使用指南

YoyoCppServer 提供了完整的日志系统，用于记录应用程序运行过程中的各种信息，便于调试和监控。

## 🚀 快速开始

### 基本使用

```cpp
#include "logger.hpp"

int main() {
    // 1. 初始化 Logger（必须在最开始）
    auto logger = yoyo::Logger::getInstance();
    logger->setPrefixPath(".")
          .setLogDirName("server_logs")
          .setLogFileName("app")
          .setConsle(true);  // 同时输出到控制台

    // 2. 记录各种级别的日志
    LOGI("应用启动成功");           // INFO 级别
    LOGW("注意: 某个资源缺失");      // WARNING 级别
    LOGE("错误: 数据库连接失败");    // ERROR 级别
    LOGD("调试信息: 变量值 = 123");  // DEBUG 级别

    return 0;
}
```

## 📊 日志级别

| 级别 | 宏 | 用途 | 颜色 |
|------|-----|------|------|
| DEBUG | LOGD | 调试信息 | 灰色 |
| INFO | LOGI | 一般信息 | 蓝色 |
| WARN | LOGW | 警告信息 | 黄色 |
| ERROR | LOGE | 错误信息 | 红色 |

## 🔧 Logger 配置

### 设置日志目录

```cpp
auto logger = yoyo::Logger::getInstance();

// 设置日志文件所在目录
logger->setPrefixPath("/home/user/logs")  // 绝对路径或相对路径
       .setLogDirName("myapp_logs")      // 日志目录名
       .setLogFileName("debug");          // 日志文件名
```

### 启用/禁用控制台输出

```cpp
// 同时输出到文件和控制台
logger->setConsle(true);

// 只输出到文件
logger->setConsle(false);
```

### 启用/禁用颜色输出

```cpp
// 在控制台输出中启用颜色
logger->setIsColor(true);

// 禁用颜色（便于日志分析）
logger->setIsColor(false);
```

## 💡 实际使用示例

### 示例 1: 服务器启动日志

```cpp
#include "logger.hpp"
#include "core/TcpServer.h"

int main() {
    // 初始化 Logger
    auto logger = yoyo::Logger::getInstance();
    logger->setPrefixPath(".")
          .setLogDirName("server_logs")
          .setLogFileName("tcp_server")
          .setConsle(true);

    LOGI("========== 服务器启动 ==========");
    
    try {
        // 创建 TCP 服务器
        yoyo::Eventloop loop;
        yoyo::TcpServer server(&loop, yoyo::InetAddress(8888));
        
        LOGI("服务器创建成功");
        LOGI("监听地址: 0.0.0.0:8888");
        
        // 启动服务器
        server.start();
        LOGI("服务器已启动，等待连接...");
        
        // 进入事件循环
        loop.loop();
        
    } catch (const std::exception& e) {
        LOGE(std::string("服务器启动失败: ") + e.what());
        return 1;
    }
    
    return 0;
}
```

### 示例 2: HTTP 请求处理日志

```cpp
void handleHttpRequest(const HttpRequest& req, HttpResponse& res) {
    auto logger = yoyo::Logger::getInstance();
    
    // 记录请求信息
    LOGI(std::string("收到请求: ") + req.getMethod() + " " + req.getPath());
    LOGD(std::string("请求 URL: ") + req.getUrl());
    
    // 处理请求
    try {
        if (req.getMethod() == "GET") {
            LOGD("处理 GET 请求");
            res.setStatusCode(200);
        } else if (req.getMethod() == "POST") {
            LOGD("处理 POST 请求");
            LOGD(std::string("请求体大小: ") + 
                 std::to_string(req.getBody().size()) + " 字节");
            res.setStatusCode(201);
        } else {
            LOGW(std::string("不支持的方法: ") + req.getMethod());
            res.setStatusCode(405);
        }
        
        LOGI(std::string("响应状态码: ") + std::to_string(res.getStatusCode()));
        
    } catch (const std::exception& e) {
        LOGE(std::string("处理请求时出错: ") + e.what());
        res.setStatusCode(500);
    }
}
```

### 示例 3: 数据库操作日志

```cpp
bool authenticateUser(const std::string& username, const std::string& password) {
    auto logger = yoyo::Logger::getInstance();
    
    LOGD(std::string("认证用户: ") + username);
    
    try {
        auto userService = yoyo::UserService::getInstance();
        
        LOGD("查询数据库...");
        auto result = userService->authenticateUser(username, password);
        
        if (result.success) {
            LOGI(std::string("用户 ") + username + " 认证成功");
            return true;
        } else {
            LOGW(std::string("用户 ") + username + " 认证失败: " + result.errorMsg);
            return false;
        }
        
    } catch (const std::exception& e) {
        LOGE(std::string("认证过程出错: ") + e.what());
        return false;
    }
}
```

### 示例 4: 性能监测日志

```cpp
void processLargeData(const std::vector<int>& data) {
    auto logger = yoyo::Logger::getInstance();
    
    auto start = std::chrono::high_resolution_clock::now();
    LOGI(std::string("开始处理 ") + std::to_string(data.size()) + " 条数据");
    
    try {
        // 处理数据
        int sum = 0;
        for (int val : data) {
            sum += val;
            // 模拟处理
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        LOGI(std::string("处理完成，耗时: ") + std::to_string(duration.count()) + " ms");
        LOGI(std::string("求和结果: ") + std::to_string(sum));
        
    } catch (const std::exception& e) {
        LOGE(std::string("处理数据时出错: ") + e.what());
    }
}
```

## 📂 日志文件位置

默认配置下，日志文件会被保存在：

```
当前目录/
├── server_logs/
│   ├── app.log              # 日志文件
│   ├── app.log.1            # 备份 1
│   ├── app.log.2            # 备份 2
│   └── ...
```

### 自定义日志位置

```cpp
logger->setPrefixPath("/var/log")        // 使用 /var/log 作为基目录
       .setLogDirName("myapp");          // 日志目录为 /var/log/myapp
```

## 🎯 日志级别选择建议

### 何时使用 DEBUG

```cpp
// 适合用于开发和调试
LOGD(std::string("变量 x 的值: ") + std::to_string(x));
LOGD("进入函数 processData");
LOGD("数据库查询完成");
```

### 何时使用 INFO

```cpp
// 适合用于正常的信息记录
LOGI("服务器启动成功");
LOGI("新用户连接");
LOGI("请求处理完成");
```

### 何时使用 WARNING

```cpp
// 适合用于警告信息
LOGW("连接超时，即将重试");
LOGW("内存使用率过高");
LOGW("数据库连接池中可用连接不足");
```

### 何时使用 ERROR

```cpp
// 适合用于错误信息
LOGE("数据库连接失败");
LOGE("文件读取错误");
LOGE("用户认证失败");
```

## 🔍 查看日志文件

### 实时查看日志

```bash
# 持续显示最新的日志
tail -f server_logs/app.log

# 显示最后 100 行日志
tail -n 100 server_logs/app.log
```

### 搜索特定日志

```bash
# 搜索错误日志
grep "ERROR" server_logs/app.log

# 搜索特定用户的日志
grep "alice" server_logs/app.log

# 搜索时间范围内的日志
grep "2026-04-02" server_logs/app.log
```

### 统计日志

```bash
# 统计各级别日志数量
echo "DEBUG: $(grep -c "DEBUG" server_logs/app.log)"
echo "INFO: $(grep -c "INFO" server_logs/app.log)"
echo "WARN: $(grep -c "WARN" server_logs/app.log)"
echo "ERROR: $(grep -c "ERROR" server_logs/app.log)"
```

## 📝 日志格式

每条日志记录包含以下信息：

```
[时间戳] [级别] [文件名:行号] [函数名] 消息内容
```

示例：
```
[2026-04-02 14:30:45.123] [INFO] [main.cc:25] main 服务器启动成功
[2026-04-02 14:30:46.456] [DEBUG] [handler.cc:102] handleRequest 处理 GET 请求
[2026-04-02 14:30:47.789] [ERROR] [service.cc:58] authenticate 认证失败
```

## ⚙️ 高级配置

### 日志缓冲大小

Logger 使用内部缓冲区来提高性能。默认缓冲大小通常为 4KB。

### 日志轮转

Logger 会自动管理日志文件大小，当日志文件过大时会自动创建备份。

## 💡 最佳实践

1. **在应用启动时初始化 Logger**
   ```cpp
   // 在 main 函数开始处初始化
   auto logger = yoyo::Logger::getInstance();
   logger->setPrefixPath(".").setLogDirName("logs").setConsle(true);
   ```

2. **为不同模块使用适当的日志级别**
   ```cpp
   // 网络模块：INFO 级别为主
   LOGI("新连接建立");
   
   // 数据库模块：DEBUG 级别
   LOGD("执行 SQL: SELECT * FROM users");
   ```

3. **避免过度日志记录**
   ```cpp
   // ❌ 不要在循环中频繁记录
   for (int i = 0; i < 1000000; ++i) {
       LOGI(std::to_string(i));  // 太频繁！
   }
   
   // ✅ 记录摘要信息
   LOGI(std::string("处理了 ") + std::to_string(count) + " 条记录");
   ```

4. **在错误处理中使用 ERROR 级别**
   ```cpp
   try {
       // 处理代码
   } catch (const std::exception& e) {
       LOGE(std::string("操作失败: ") + e.what());
   }
   ```

5. **记录重要的状态变化**
   ```cpp
   LOGI("用户 alice 已登录");
   LOGI("游戏开始");
   LOGI("连接已关闭");
   ```

## 🔗 相关文件

- **头文件**: `src/utils/logger.hpp` (21KB)
- **实现文件**: `src/utils/logger.cc`
- **日志输出**: `server_logs/` 目录

## 📚 更多信息

详细的 Logger API 文档请查看 `src/utils/logger.hpp` 的源代码注释。

---

**Logger 系统已完全集成！使用日志记录来提高代码的可维护性和可调试性。** ✅
