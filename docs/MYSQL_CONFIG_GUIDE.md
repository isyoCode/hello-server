# MySQL 配置指南

## 配置文件位置

MySQL 配置文件位于：**`config/mysql.conf.example`**

## 配置步骤

### 1. 复制配置文件

```bash
cd /home/isyo/yoyo_cpp_server
cp config/mysql.conf.example config/mysql.conf
```

### 2. 编辑配置文件

编辑 `config/mysql.conf`，修改 MySQL 连接参数：

```ini
# MySQL 连接池配置文件
# 该文件被 SqlPool 的 ConnectionPool 读取

# 数据库连接信息
ip=127.0.0.1           # MySQL 服务器 IP 地址
port=3306              # MySQL 服务器端口

# 认证信息
username=root           # 数据库用户名
passwd=123456          # 数据库密码

# 数据库名
dbname=hello_app       # 使用的数据库名称

# 连接池配置
initSize=5             # 初始连接数
MaxSize=10             # 最大连接数
maxIdleTime=300        # 最大空闲时间(秒)
maxConnectionTimeOut=3000  # 连接超时时间(毫秒)
```

## 配置文件加载流程

### 自动加载机制

程序启动时，**ConnectionPool** 会自动加载配置文件：

```cpp
// UserService.cc 第 70-82 行
std::shared_ptr<Connection> UserService::getConnection() {
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
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

**ConnectionPool 的初始化流程：**

1. **单例模式获取**: `ConnectionPool::getConnectionPool()` (connectionPool.h 第 20 行)
   - 返回 ConnectionPool 的单例实例
   - 在首次调用时创建连接池

2. **配置文件加载**: `loadConfigFile()` (connectionPool.h 第 28-31 行)
   - 自动从 `config/mysql.conf` 读取配置
   - 解析 IP、端口、用户名、密码、数据库名等参数

3. **连接池初始化**: 
   - 根据 `initSize` 参数创建初始连接
   - 启动后台线程管理连接
   - 启动扫描线程清理过期连接

### 代码位置详解

| 文件 | 位置 | 说明 |
|------|------|------|
| `include/connectionPool.h` | 第 20 行 | `getConnectionPool()` - 单例获取接口 |
| `include/connectionPool.h` | 第 22 行 | `getConnection()` - 获取连接 |
| `include/connectionPool.h` | 第 28-31 行 | `loadConfigFile()` - 加载配置 |
| `include/commonConnection.h` | 全文 | Connection 类 - 单个数据库连接 |
| `src/database/UserService.cc` | 第 70-82 行 | 获取连接示例 |
| `src/database/UserService.cc` | 第 71-81 行 | 连接失败处理 |
| `config/mysql.conf.example` | 全文 | 配置文件示例 |

## 使用示例

### 在 UserService 中使用

```cpp
// src/database/UserService.cc 第 234-263 行
ServiceResult UserService::getUserById(unsigned int id) {
    // 1. 获取连接池中的连接
    auto conn = getConnection();
    if (!conn) {
        logError("getUserById", "Failed to get database connection from pool");
        return ServiceResult(false, "Failed to get database connection");
    }

    // 2. 使用连接执行查询
    std::string sql = "SELECT id, username, email, password_hash, status, "
                      "login_attempts, created_at, updated_at, last_login "
                      "FROM users WHERE id = " + std::to_string(id) +
                      " AND status != 'banned'";

    MYSQL_RES* result = conn->query(sql);
    if (!result) {
        logError("getUserById", "Query execution failed for ID: " + std::to_string(id));
        return ServiceResult(false, "Database query failed");
    }

    // 3. 处理结果
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return ServiceResult(false, "User not found");
    }

    User user = parseUserFromResult(row);
    mysql_free_result(result);  // 释放结果资源

    return ServiceResult(true, "User found successfully", user);
}
```

## 连接池配置参数详解

### initSize (初始连接数)
- **默认值**: 5
- **含义**: 连接池启动时创建的连接数
- **建议**: 通常设为 5-10，根据并发量调整

### MaxSize (最大连接数)
- **默认值**: 10
- **含义**: 连接池最多能创建的连接数
- **建议**: 通常设为 10-20，不超过 MySQL 服务器的 `max_connections`

### maxIdleTime (最大空闲时间)
- **默认值**: 300 秒 (5 分钟)
- **含义**: 连接空闲超过此时间会被回收
- **建议**: 保持默认值，避免连接被 MySQL 服务器主动断开

### maxConnectionTimeOut (连接超时时间)
- **默认值**: 3000 毫秒 (3 秒)
- **含义**: 获取连接时的最长等待时间
- **建议**: 在高并发场景下可适当增加

## 常见问题

### Q: 修改配置后需要重新编译吗?
**A**: 不需要。程序启动时会动态加载 `config/mysql.conf` 文件，修改配置只需重新启动程序。

### Q: 如果找不到 mysql.conf 文件会怎样?
**A**: ConnectionPool 会使用默认配置或报错。建议始终使用 `config/mysql.conf` 文件进行配置。

### Q: 连接池用完了会怎样?
**A**: 
- 如果连接数达到 `MaxSize`，新的连接请求会等待
- 等待超时时间为 `maxConnectionTimeOut`
- 超时后返回 nullptr，操作失败

### Q: 如何监控连接池状态?
**A**: 
- 查看日志文件: `server_logs/tcp_server.log`
- 搜索 "connection" 相关的日志输出
- 监控 MySQL 服务器的 `SHOW PROCESSLIST;` 命令

## 调试技巧

### 1. 验证配置是否正确

```bash
# 使用 mysql 客户端测试连接
mysql -h 127.0.0.1 -u root -p123456 -D hello_app
```

### 2. 查看连接池日志

```bash
# 实时查看日志
tail -f server_logs/tcp_server.log | grep -i "connection\|connect\|pool"
```

### 3. 检查 MySQL 连接数

```bash
# 登录 MySQL 后查看
mysql> SHOW PROCESSLIST;
mysql> SHOW STATUS WHERE variable_name LIKE 'Threads%';
```

## 最佳实践

1. **开发环境**: 使用默认配置 (initSize=5, MaxSize=10)
2. **生产环境**: 
   - 根据并发量调整 `initSize` 和 `MaxSize`
   - 建议 `MaxSize` 不超过 MySQL 的 `max_connections / 2`
   - 定期检查连接池使用情况

3. **安全考虑**:
   - 不要在代码中硬编码数据库密码
   - 使用环境变量或配置文件管理敏感信息
   - 在生产环境中使用强密码

4. **性能优化**:
   - 合理设置 `maxIdleTime` 避免连接泄漏
   - 监控连接池的使用率
   - 及时释放查询结果 (`mysql_free_result`)

## 参考文件

- [src/database/UserService.h](../src/database/UserService.h) - 用户服务接口
- [src/database/UserService.cc](../src/database/UserService.cc) - 用户服务实现
- [include/connectionPool.h](../include/connectionPool.h) - 连接池接口
- [include/commonConnection.h](../include/commonConnection.h) - 数据库连接类
- [config/mysql.conf.example](../config/mysql.conf.example) - 配置文件示例
