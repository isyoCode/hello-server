# MySQL 配置加载流程详解

## 📍 配置文件位置

**MySQL 配置文件**: `config/mysql.conf.example` → 复制为 `config/mysql.conf`

```bash
cp config/mysql.conf.example config/mysql.conf
```

---

## 🔄 程序启动时加载 MySQL 配置的完整流程

### 第1步：程序入口

**文件**: `examples/basic_server.cc`

```cpp
// 第 12 行: main 函数开始
int main() {
    using namespace yoyo;

    // 初始化日志系统
    Logger::getInstance()
        ->setPrefixPath(".")
        .setLogDirName("server_logs")
        .setLogFileName("tcp_server")
        .setConsle(true)
        .setWritefile(true);

    Eventloop loop(false);
    InetAddress listenAddr(8888);
    TcpServer server(&loop, listenAddr);
    server.start();
    
    // ... 路由配置
    
    loop.loop();  // 进入事件循环
}
```

### 第2步：处理 HTTP 请求

当客户端发送 HTTP 请求时：

**文件**: `examples/basic_server.cc` 第 73-152 行

```cpp
server.setMessageCallback([&mainRouter](const TcpConnection::TcpConnectionPtr conn, const std::string& smsg){
    // 解析 HTTP 请求
    yoyo::http::HttpRequestParser parser;
    bool bSuccess = parser.parse(smsg);
    
    if(bSuccess) {
        const auto& request = parser.getRequse();
        
        // 使用路由器处理请求
        bool routed = mainRouter.route(request, response);  // ← 触发对应的 Handler
        
        // ...
    }
});
```

### 第3步：Handler 调用 UserService 进行数据库操作

**文件**: `src/handlers/Handler.cc`

```cpp
// Handler 处理请求时调用 UserService
auto userService = yoyo::database::UserService::getInstance();
ServiceResult result = userService->authenticateUser(username, password);  // ← 需要数据库连接
```

### 第4步：UserService 获取数据库连接

**文件**: `src/database/UserService.cc` 第 70-82 行

```cpp
std::shared_ptr<Connection> UserService::getConnection() {
    // 这是关键点：获取连接池单例
    ConnectionPool* pool = ConnectionPool::getConnectionPool();  // ← 首次调用时加载配置！
    
    if (!pool) {
        LOG("ERROR: [getConnection] Failed to get ConnectionPool instance");
        return nullptr;
    }
    
    auto conn = pool->getConnection();
    if (!conn) {
        LOG("ERROR: [getConnection] Failed to obtain connection from pool");
        return nullptr;
    }
    return conn;
}
```

### 第5步：ConnectionPool 加载配置文件

**文件**: `include/connectionPool.h` 第 18-60 行

```cpp
class ConnectionPool {
 public:
  static ConnectionPool* getConnectionPool();  // ← 第 20 行：单例获取方法

  shared_ptr<Connection> getConnection();

 private:
  ConnectionPool();
  ~ConnectionPool();

  bool loadConfigFile();  // ← 第 28 行：加载配置方法
  bool loadConfigFile(const std::string& filename,
                      std::unordered_map<std::string, std::string>& config);

  // ... 其他成员变量
};
```

**执行流程:**

1. **首次调用** `ConnectionPool::getConnectionPool()`:
   - 检查单例是否已存在
   - 如果不存在，创建新的 ConnectionPool 实例
   - 构造函数调用 `loadConfigFile()`

2. **`loadConfigFile()` 方法执行**:
   - 打开 `config/mysql.conf` 文件
   - 逐行读取配置参数
   - 解析键值对: `ip`, `port`, `username`, `passwd`, `dbname`, `initSize`, `MaxSize` 等

3. **初始化连接池**:
   - 根据 `initSize` 参数创建初始数据库连接
   - 启动后台 producer 线程（负责创建新连接）
   - 启动 scanner 线程（负责清理过期连接）

4. **连接池就绪**:
   - ConnectionPool 单例已初始化完成
   - 后续的 `getConnection()` 调用直接从池中获取连接

### 第6步：使用数据库连接执行查询

**文件**: `src/database/UserService.cc` 第 234-263 行

```cpp
ServiceResult UserService::getUserById(unsigned int id) {
    // 获取连接（从已初始化的连接池）
    auto conn = getConnection();
    if (!conn) {
        logError("getUserById", "Failed to get database connection from pool");
        return ServiceResult(false, "Failed to get database connection");
    }

    // 执行 SQL 查询
    std::string sql = "SELECT id, username, email, password_hash, status, "
                      "login_attempts, created_at, updated_at, last_login "
                      "FROM users WHERE id = " + std::to_string(id) +
                      " AND status != 'banned'";

    MYSQL_RES* result = conn->query(sql);  // ← 使用连接执行查询
    if (!result) {
        logError("getUserById", "Query execution failed for ID: " + std::to_string(id));
        return ServiceResult(false, "Database query failed");
    }

    // 处理结果
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return ServiceResult(false, "User not found");
    }

    User user = parseUserFromResult(row);
    mysql_free_result(result);

    return ServiceResult(true, "User found successfully", user);
}
```

---

## 📋 配置文件详解

**文件**: `config/mysql.conf.example`

| 参数 | 默认值 | 说明 | 示例 |
|------|--------|------|------|
| `ip` | 127.0.0.1 | MySQL 服务器地址 | 127.0.0.1 (本地) / 192.168.1.100 (远程) |
| `port` | 3306 | MySQL 服务器端口 | 3306 |
| `username` | root | 数据库用户名 | root / admin / app_user |
| `passwd` | 123456 | 数据库密码 | 你的密码 |
| `dbname` | hello_app | 数据库名称 | hello_app |
| `initSize` | 5 | 初始连接数 | 5 (推荐 5-10) |
| `MaxSize` | 10 | 最大连接数 | 10 (推荐 10-20) |
| `maxIdleTime` | 300 | 最大空闲时间(秒) | 300 (5分钟) |
| `maxConnectionTimeOut` | 3000 | 连接超时(毫秒) | 3000 (3秒) |

---

## 🔑 关键代码位置

### 配置读取

| 位置 | 代码 | 说明 |
|------|------|------|
| `include/connectionPool.h` L20 | `static ConnectionPool* getConnectionPool();` | 单例获取方法 |
| `include/connectionPool.h` L28 | `bool loadConfigFile();` | 配置文件加载方法 |
| `config/mysql.conf.example` | 完整配置 | 配置文件示例 |

### 连接使用

| 位置 | 代码 | 说明 |
|------|------|------|
| `src/database/UserService.cc` L70-82 | `getConnection()` | 获取连接方法 |
| `include/commonConnection.h` L22 | `bool update(string sql);` | 执行 INSERT/UPDATE/DELETE |
| `include/commonConnection.h` L25 | `MYSQL_RES* query(string sql);` | 执行 SELECT 查询 |

### 错误处理

| 位置 | 代码 | 说明 |
|------|------|------|
| `src/database/UserService.cc` L237-238 | 检查 conn 是否 nullptr | 连接获取失败处理 |
| `src/database/UserService.cc` L248-251 | 检查 result 是否 nullptr | 查询失败处理 |
| `src/database/UserService.cc` L254-257 | 检查 row 是否 nullptr | 结果为空处理 |

---

## 🚀 快速配置步骤

### 1️⃣ 准备配置文件

```bash
cd /home/isyo/yoyo_cpp_server

# 复制配置文件
cp config/mysql.conf.example config/mysql.conf
```

### 2️⃣ 编辑配置文件

```bash
# 用编辑器打开
vim config/mysql.conf

# 修改以下内容（根据你的 MySQL 实际配置）:
# ip=127.0.0.1          # 改为你的 MySQL 服务器 IP
# port=3306             # 改为你的 MySQL 端口
# username=root         # 改为你的数据库用户名
# passwd=123456         # 改为你的数据库密码
# dbname=hello_app      # 改为你的数据库名
```

### 3️⃣ 编译项目

```bash
bash scripts/build.sh
```

### 4️⃣ 运行程序

```bash
./build/yoyo-server
```

### 5️⃣ 验证连接

```bash
# 查看日志文件
tail -f server_logs/tcp_server.log

# 或搜索连接相关日志
grep -i "connection\|pool\|database" server_logs/tcp_server.log
```

---

## ✅ 验证连接是否成功

### 日志中查找成功信息

```bash
# 查看日志
cat server_logs/tcp_server.log

# 成功的日志示例:
# [2026-04-02 11:55:11.860][INFO][...] TcpServer Starting...
# [2026-04-02 11:55:11.860][INFO][...] Thread pool started with 4 worker threads
# [2026-04-02 11:55:26.233][INFO][...] New connection from 127.0.0.1:59788 on fd=7
# [2026-04-02 11:55:26.234][INFO][...] HTTP POST /register from fd=7
# [2026-04-02 11:55:26.234][INFO][...] Registration attempt: username=alice email=alice@example.com
# [2026-04-02 11:55:26.317][INFO][...] Registration successful: username=alice
```

### 检查错误信息

```bash
# 查找错误日志
grep -i "error\|failed" server_logs/tcp_server.log

# 常见错误:
# - "Failed to get ConnectionPool instance" → 连接池创建失败
# - "Failed to obtain connection from pool" → 连接获取失败
# - "Query execution failed" → SQL 执行失败
# - "Connection refused" → MySQL 服务未启动或地址错误
```

### 使用 MySQL 客户端验证

```bash
# 测试连接
mysql -h 127.0.0.1 -u root -p123456 -D hello_app

# 查看活跃连接
mysql> SHOW PROCESSLIST;

# 查看连接数统计
mysql> SHOW STATUS WHERE variable_name LIKE 'Threads%';
```

---

## 🔍 故障排查

### 问题 1: ConnectionPool 获取失败

**症状**: 日志显示 "Failed to get ConnectionPool instance"

**原因**:
- MySQL 配置文件不存在
- 配置文件格式错误
- 数据库连接参数错误

**解决**:
```bash
# 检查配置文件是否存在
ls -l config/mysql.conf

# 检查配置文件格式
cat config/mysql.conf

# 验证 MySQL 服务是否运行
mysql -h 127.0.0.1 -u root -p123456 -e "SELECT 1"
```

### 问题 2: 连接数耗尽

**症状**: 日志显示 "Failed to obtain connection from pool"

**原因**:
- 所有连接都在使用中
- 连接泄漏（未正确释放）
- MaxSize 设置太小

**解决**:
```bash
# 检查 MySQL 连接数
mysql> SHOW PROCESSLIST;

# 查看连接池配置
cat config/mysql.conf | grep -E "initSize|MaxSize"

# 增加 MaxSize 值
# 修改 config/mysql.conf 中的 MaxSize=20 (从 10 改为 20)
```

### 问题 3: 查询执行失败

**症状**: 日志显示 "Query execution failed"

**原因**:
- 表不存在
- 权限不足
- SQL 语法错误

**解决**:
```bash
# 检查表是否存在
mysql -h 127.0.0.1 -u root -p123456 -D hello_app -e "SHOW TABLES;"

# 检查用户权限
mysql -h 127.0.0.1 -u root -p123456 -e "SHOW GRANTS FOR 'root'@'%';"

# 查看详细错误信息
grep "Query execution failed" server_logs/tcp_server.log
```

---

## 📚 相关文档

- [MYSQL_CONFIG_GUIDE.md](./MYSQL_CONFIG_GUIDE.md) - MySQL 配置详细指南
- [CONFIGURATION_INDEX.md](./CONFIGURATION_INDEX.md) - 所有配置文件索引
- [LOGGER_GUIDE.md](./LOGGER_GUIDE.md) - 日志系统使用指南
- [API_REFERENCE.md](./API_REFERENCE.md) - API 参考文档

---

**更新时间**: 2026-04-02
