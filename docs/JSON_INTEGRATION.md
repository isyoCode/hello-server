# JSON 集成指南

本文档介绍 YoyoCppServer 的 JSON 模块使用方法，包括解析、生成和 HTTP 集成。

## 📋 目录

- [快速开始](#快速开始)
- [核心模块](#核心模块)
- [HTTP 集成](#http-集成)
- [实战示例](#实战示例)
- [最佳实践](#最佳实践)
- [常见问题](#常见问题)

---

## 🚀 快速开始

### 基本使用

```cpp
#include "http/json_parser.hpp"

using namespace yoyo;

// 解析 JSON 字符串
std::string jsonStr = R"({"name": "Alice", "age": 25})";
JsonParser parser(jsonStr);
JsonFiled json = parser.parser();

// 访问字段
std::string name = json["name"].asString();  // "Alice"
int age = json["age"].asInt();               // 25

// 生成 JSON 字符串
std::string output = json.writeToString();
```

### HTTP 请求处理

```cpp
#include "http/json_utils.h"

void handleRequest(const HttpRequest& req, HttpResponse& res) {
    // 解析请求
    JsonFiled json = JsonUtils::parseJsonFromRequest(req);
    
    // 构建响应
    JsonFiled::json_object data;
    data["status"] = JsonFiled(std::string("success"));
    
    res = JsonUtils::createJsonSuccessResponse(JsonFiled(data));
}
```

---

## 🔧 核心模块

### 1. JsonFiled - JSON 值类型

`JsonFiled` 是核心 JSON 数据结构，支持所有 JSON 类型。

#### 支持的类型

| JSON 类型 | C++ 类型 | 枚举值 |
|-----------|---------|--------|
| null | `std::monostate` | `JSON_NULL` |
| boolean | `bool` | `JSON_BOOLEAN` |
| integer | `int32_t` | `JSON_NUMBER` |
| double | `double` | `JSON_DOUBLE` |
| string | `std::string` | `JSON_STRING` |
| array | `std::vector<JsonFiled>` | `JSON_ARRAY` |
| object | `std::map<std::string, JsonFiled>` | `JSON_OBJECT` |

#### 构造 JSON 对象

```cpp
// 创建各种类型的 JSON 值
JsonFiled nullValue;                           // null
JsonFiled boolValue(true);                     // boolean
JsonFiled intValue(42);                        // integer
JsonFiled doubleValue(3.14);                   // double
JsonFiled strValue(std::string("hello"));      // string

// 创建 JSON 数组
JsonFiled::json_array arr = {
    JsonFiled(1),
    JsonFiled(2),
    JsonFiled(3)
};
JsonFiled arrayValue(arr);

// 创建 JSON 对象
JsonFiled::json_object obj;
obj["name"] = JsonFiled(std::string("Alice"));
obj["age"] = JsonFiled(25);
obj["active"] = JsonFiled(true);
JsonFiled objectValue(obj);
```

#### 访问 JSON 值

```cpp
JsonFiled json = /* ... */;

// 类型检查
if (json.isObject()) { /* ... */ }
if (json.isArray()) { /* ... */ }
if (json.isString()) { /* ... */ }

// 获取值
std::string name = json["name"].asString();
int age = json["age"].asInt();
bool active = json["active"].asBool();

// 数组访问
JsonFiled arr = json["items"];
for (size_t i = 0; i < arr.size(); ++i) {
    JsonFiled item = arr[i];
    // 处理 item
}

// 对象遍历
if (json.isObject()) {
    auto obj = json.asObject();
    for (const auto& [key, value] : obj) {
        std::cout << key << ": " << value.writeToString() << "\n";
    }
}
```

#### 成员检查

```cpp
// 检查对象是否包含某个键
if (json.isMember("username")) {
    std::string username = json["username"].asString();
}

// 安全访问（不存在时不会抛异常）
std::string email = json.isMember("email") 
    ? json["email"].asString() 
    : "default@example.com";
```

---

### 2. JsonParser - JSON 解析器

解析 JSON 字符串为 `JsonFiled` 对象。

#### 基本解析

```cpp
#include "http/json_parser.hpp"

std::string jsonStr = R"({
    "user": {
        "id": 123,
        "name": "Alice",
        "email": "alice@example.com"
    },
    "tags": ["cpp", "json", "server"]
})";

try {
    JsonParser parser(jsonStr);
    JsonFiled json = parser.parser();
    
    // 访问嵌套数据
    int userId = json["user"]["id"].asInt();
    std::string name = json["user"]["name"].asString();
    
    // 访问数组
    JsonFiled tags = json["tags"];
    for (size_t i = 0; i < tags.size(); ++i) {
        std::cout << tags[i].asString() << "\n";
    }
    
} catch (const JsonParseError& e) {
    std::cerr << "Parse error: " << e.what() << "\n";
}
```

#### 错误处理

```cpp
try {
    JsonParser parser(invalidJsonStr);
    JsonFiled json = parser.parser();
} catch (const JsonParseError& e) {
    std::cerr << "Parse error at position " 
              << e.getErrorIndex() << ": " 
              << e.what() << "\n";
} catch (const std::logic_error& e) {
    std::cerr << "Logic error: " << e.what() << "\n";
}
```

---

### 3. JsonUtils - HTTP 集成工具

提供 HTTP 请求/响应与 JSON 的便捷集成。

#### 解析 HTTP 请求

```cpp
#include "http/json_utils.h"

void handleJsonRequest(const HttpRequest& req, HttpResponse& res) {
    // 检查 Content-Type
    if (!JsonUtils::isJsonRequest(req)) {
        res = JsonUtils::createJsonErrorResponse(
            400, 
            "Content-Type must be application/json"
        );
        return;
    }
    
    // 解析请求体
    JsonFiled json = JsonUtils::parseJsonFromRequest(req);
    
    if (json.isNull()) {
        res = JsonUtils::createJsonErrorResponse(
            400, 
            "Invalid JSON format"
        );
        return;
    }
    
    // 处理数据...
}
```

#### 创建 JSON 响应

```cpp
// 成功响应
JsonFiled::json_object data;
data["user_id"] = JsonFiled(123);
data["username"] = JsonFiled(std::string("alice"));

res = JsonUtils::createJsonSuccessResponse(
    JsonFiled(data),
    "User created successfully"
);

// 响应格式:
// {
//   "status": "ok",
//   "message": "User created successfully",
//   "data": {
//     "user_id": 123,
//     "username": "alice"
//   }
// }
```

```cpp
// 错误响应
res = JsonUtils::createJsonErrorResponse(
    400,
    "Username is required",
    1001  // 可选的错误代码
);

// 响应格式:
// {
//   "error": "Username is required",
//   "code": 1001
// }
```

#### 安全获取字段值

```cpp
JsonFiled json = JsonUtils::parseJsonFromRequest(req);

// 带默认值的安全访问
std::string username = JsonUtils::getJsonString(json, "username", "guest");
int age = JsonUtils::getJsonInt(json, "age", 0);
bool active = JsonUtils::getJsonBool(json, "active", false);
double score = JsonUtils::getJsonDouble(json, "score", 0.0);

// 不会抛异常，字段不存在或类型错误时返回默认值
```

---

## 💡 实战示例

### 示例 1: 用户注册

```cpp
void handleRegister(const HttpRequest& req, HttpResponse& res) {
    // 检查请求类型
    if (!JsonUtils::isJsonRequest(req)) {
        res = JsonUtils::createJsonErrorResponse(400, "Invalid content type");
        return;
    }
    
    // 解析请求
    JsonFiled json = JsonUtils::parseJsonFromRequest(req);
    if (json.isNull()) {
        res = JsonUtils::createJsonErrorResponse(400, "Invalid JSON");
        return;
    }
    
    // 提取字段
    std::string username = JsonUtils::getJsonString(json, "username");
    std::string email = JsonUtils::getJsonString(json, "email");
    std::string password = JsonUtils::getJsonString(json, "password");
    
    // 验证
    if (username.empty() || email.empty() || password.empty()) {
        res = JsonUtils::createJsonErrorResponse(
            400, 
            "Username, email and password are required"
        );
        return;
    }
    
    // 创建用户（伪代码）
    int userId = createUser(username, email, password);
    
    // 返回成功响应
    JsonFiled::json_object data;
    data["user_id"] = JsonFiled(userId);
    data["username"] = JsonFiled(username);
    data["email"] = JsonFiled(email);
    
    res = JsonUtils::createJsonSuccessResponse(
        JsonFiled(data),
        "Registration successful"
    );
}
```

### 示例 2: 游戏统计保存

```cpp
void handleSaveGameStats(const HttpRequest& req, HttpResponse& res) {
    try {
        JsonFiled json = JsonUtils::parseJsonFromRequest(req);
        
        // 验证必需字段
        if (!json.isMember("user_id") || !json["user_id"].isInt()) {
            res = JsonUtils::createJsonErrorResponse(400, "user_id is required");
            return;
        }
        
        if (!json.isMember("game_type") || !json["game_type"].isString()) {
            res = JsonUtils::createJsonErrorResponse(400, "game_type is required");
            return;
        }
        
        if (!json.isMember("score") || !json["score"].isInt()) {
            res = JsonUtils::createJsonErrorResponse(400, "score is required");
            return;
        }
        
        // 提取数据
        int userId = json["user_id"].asInt();
        std::string gameType = json["game_type"].asString();
        int score = json["score"].asInt();
        int duration = JsonUtils::getJsonInt(json, "duration", 0);
        
        // 验证游戏类型
        static const std::vector<std::string> validGames = {
            "tetris", "fighter", "fishing"
        };
        if (std::find(validGames.begin(), validGames.end(), gameType) == 
            validGames.end()) {
            res = JsonUtils::createJsonErrorResponse(400, "Invalid game_type");
            return;
        }
        
        // 保存到数据库（伪代码）
        saveGameStats(userId, gameType, score, duration);
        
        // 返回成功
        JsonFiled::json_object result;
        result["user_id"] = JsonFiled(userId);
        result["game_type"] = JsonFiled(gameType);
        result["score"] = JsonFiled(score);
        result["timestamp"] = JsonFiled(getCurrentTimestamp());
        
        res = JsonUtils::createJsonSuccessResponse(
            JsonFiled(result),
            "Game statistics saved"
        );
        
    } catch (const std::exception& e) {
        res = JsonUtils::createJsonErrorResponse(500, e.what());
    }
}
```

### 示例 3: 批量操作

```cpp
void handleBatchOperations(const HttpRequest& req, HttpResponse& res) {
    JsonFiled json = JsonUtils::parseJsonFromRequest(req);
    
    if (!json.isMember("operations") || !json["operations"].isArray()) {
        res = JsonUtils::createJsonErrorResponse(
            400, 
            "operations array is required"
        );
        return;
    }
    
    JsonFiled operations = json["operations"];
    std::vector<JsonFiled> results;
    
    // 处理每个操作
    for (size_t i = 0; i < operations.size(); ++i) {
        JsonFiled op = operations[i];
        
        std::string action = JsonUtils::getJsonString(op, "action");
        int itemId = JsonUtils::getJsonInt(op, "item_id");
        
        // 执行操作（伪代码）
        bool success = performOperation(action, itemId);
        
        // 记录结果
        JsonFiled::json_object resultObj;
        resultObj["operation_index"] = JsonFiled(static_cast<int>(i));
        resultObj["action"] = JsonFiled(action);
        resultObj["item_id"] = JsonFiled(itemId);
        resultObj["success"] = JsonFiled(success);
        
        results.push_back(JsonFiled(resultObj));
    }
    
    // 返回批量结果
    JsonFiled::json_object response;
    response["total"] = JsonFiled(static_cast<int>(results.size()));
    response["results"] = JsonFiled(results);
    
    res = JsonUtils::createJsonSuccessResponse(
        JsonFiled(response),
        "Batch operations completed"
    );
}
```

---

## ✅ 最佳实践

### 1. 错误处理

```cpp
void handleRequest(const HttpRequest& req, HttpResponse& res) {
    try {
        // 第一层：检查 Content-Type
        if (!JsonUtils::isJsonRequest(req)) {
            res = JsonUtils::createJsonErrorResponse(400, "Invalid content type");
            return;
        }
        
        // 第二层：解析 JSON
        JsonFiled json = JsonUtils::parseJsonFromRequest(req);
        if (json.isNull()) {
            res = JsonUtils::createJsonErrorResponse(400, "Invalid JSON format");
            return;
        }
        
        // 第三层：验证必需字段
        if (!json.isMember("username")) {
            res = JsonUtils::createJsonErrorResponse(400, "username is required");
            return;
        }
        
        // 第四层：业务逻辑
        // ...
        
    } catch (const std::exception& e) {
        // 捕获所有异常
        res = JsonUtils::createJsonErrorResponse(500, e.what());
    }
}
```

### 2. 类型安全

```cpp
// ❌ 不推荐：直接访问，可能抛异常
int age = json["age"].asInt();

// ✅ 推荐：先检查类型
if (json.isMember("age") && json["age"].isInt()) {
    int age = json["age"].asInt();
} else {
    // 处理类型错误
}

// ✅ 更推荐：使用 JsonUtils 安全获取
int age = JsonUtils::getJsonInt(json, "age", 0);  // 有默认值
```

### 3. 输入验证

```cpp
bool validateUserInput(const JsonFiled& json, std::string& errorMsg) {
    // 验证必需字段存在性
    if (!json.isMember("username") || !json["username"].isString()) {
        errorMsg = "username is required and must be a string";
        return false;
    }
    
    // 验证字段长度
    std::string username = json["username"].asString();
    if (username.length() < 3 || username.length() > 20) {
        errorMsg = "username must be 3-20 characters";
        return false;
    }
    
    // 验证邮箱格式
    if (json.isMember("email")) {
        std::string email = json["email"].asString();
        if (email.find('@') == std::string::npos) {
            errorMsg = "invalid email format";
            return false;
        }
    }
    
    return true;
}
```

### 4. 性能优化

```cpp
// ❌ 避免重复解析
for (int i = 0; i < 100; ++i) {
    JsonFiled json = JsonUtils::parseJsonFromRequest(req);  // 每次都解析
}

// ✅ 解析一次，重复使用
JsonFiled json = JsonUtils::parseJsonFromRequest(req);
for (int i = 0; i < 100; ++i) {
    // 使用 json
}

// ✅ 使用引用避免拷贝
const JsonFiled& userObj = json["user"];  // 引用，不拷贝
std::string name = userObj["name"].asString();
```

---

## 🔍 常见问题

### Q1: 如何处理空值？

```cpp
JsonFiled json = /* ... */;

// 方法 1: 检查是否为 null
if (json.isNull()) {
    std::cout << "Value is null\n";
}

// 方法 2: 检查成员是否存在
if (!json.isMember("optional_field")) {
    std::cout << "Field does not exist\n";
}

// 方法 3: 使用默认值
std::string value = JsonUtils::getJsonString(json, "optional_field", "default");
```

### Q2: 如何构建复杂的嵌套 JSON？

```cpp
// 构建嵌套对象
JsonFiled::json_object user;
user["id"] = JsonFiled(123);
user["name"] = JsonFiled(std::string("Alice"));

JsonFiled::json_object address;
address["city"] = JsonFiled(std::string("Beijing"));
address["zip"] = JsonFiled(std::string("100000"));

user["address"] = JsonFiled(address);

// 添加数组
JsonFiled::json_array tags;
tags.push_back(JsonFiled(std::string("admin")));
tags.push_back(JsonFiled(std::string("developer")));
user["tags"] = JsonFiled(tags);

JsonFiled result(user);
std::cout << result.writeToString() << "\n";
// 输出: {"id":123,"name":"Alice","address":{"city":"Beijing","zip":"100000"},"tags":["admin","developer"]}
```

### Q3: 如何调试 JSON 输出？

```cpp
// 使用 PrintJson 函数美化输出
JsonFiled json = /* ... */;
yoyo::PrintJson(json);

// 或者直接输出
std::cout << json.writeToString() << "\n";
```

### Q4: 如何处理大型 JSON？

```cpp
// JSON 解析器有深度限制（默认 64 层）
// 对于超大 JSON，考虑分批处理

try {
    JsonParser parser(largeJsonStr);
    JsonFiled json = parser.parser();
} catch (const JsonParseError& e) {
    if (std::string(e.what()).find("Maximum JSON depth") != std::string::npos) {
        std::cerr << "JSON too deeply nested\n";
    }
}
```

### Q5: 数字类型如何处理？

```cpp
JsonFiled json = /* ... */;

// JSON 有 int 和 double 两种数字类型
if (json["value"].isInt()) {
    int val = json["value"].asInt();
} else if (json["value"].isDouble()) {
    double val = json["value"].asDouble();
}

// 如果不确定类型，可以尝试转换
try {
    int intVal = json["value"].asInt();
} catch (...) {
    try {
        double doubleVal = json["value"].asDouble();
    } catch (...) {
        // 不是数字类型
    }
}
```

---

## 📚 参考文档

- [快速开始](QUICK_START.md) - 项目快速上手
- [API 参考](API_REFERENCE.md) - 完整 API 文档
- [架构设计](ARCHITECTURE.md) - 系统架构说明

---

## 📝 相关文件

- `src/http/json_parser.hpp` - JSON 解析器实现
- `src/http/json_utils.h` - HTTP 集成工具
- `src/handlers/JsonExampleHandler.cc` - 使用示例

---

**最后更新**: 2026-04-03  
**版本**: 1.0
