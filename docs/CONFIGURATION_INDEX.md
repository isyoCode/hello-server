# YoyoCppServer 配置文件索引

项目中所有的配置文件和动态加载配置的位置快速查询。

## 📋 配置文件一览

### MySQL 数据库配置

| 配置项 | 文件路径 | 加载位置 | 说明 |
|--------|--------|--------|------|
| **MySQL 连接** | `config/mysql.conf.example` | `include/connectionPool.h` L28-31: `loadConfigFile()` | 数据库连接参数 |

**配置文件内容:**
```ini
ip=127.0.0.1
port=3306
username=root
passwd=123456
dbname=hello_app
initSize=5
MaxSize=10
maxIdleTime=300
maxConnectionTimeOut=3000
```

**使用方法:**
```bash
cp config/mysql.conf.example config/mysql.conf
# 编辑 config/mysql.conf 修改你的数据库连接信息
```

**代码加载流程:**
1. `ConnectionPool::getConnectionPool()` - 获取单例 (connectionPool.h L20)
2. 自动调用 `loadConfigFile()` - 读取 config/mysql.conf
3. 解析配置参数，创建连接池

---

### 日志系统配置

| 配置项 | 文件路径 | 加载位置 | 说明 |
|--------|--------|--------|------|
| **日志输出** | 无配置文件 | `examples/basic_server.cc` L16-24 | 代码中动态配置 |

**代码配置示例:**
```cpp
// examples/basic_server.cc 第 16-24 行
Logger::getInstance()
    ->setPrefixPath(".")           // 日志存储基路径
    .setLogDirName("server_logs")  // 日志目录名
    .setLogFileName("tcp_server")  // 日志文件名
    .setConsle(true)               // 启用控制台输出
    .setColor(true)                // 启用彩色输出
    .setWritefile(true)            // 启用文件输出
    .setRotate(true)               // 启用日志轮转
    .setFileMaxSize(10 * 1024 * 1024);  // 10MB自动轮转
```

**输出文件:**
```
server_logs/
├── tcp_server.log          # 当前日志文件
├── tcp_server.log.1        # 备份日志 1
├── tcp_server.log.2        # 备份日志 2
└── ...
```

**详细指南:** [docs/LOGGER_GUIDE.md](./LOGGER_GUIDE.md)

---

### 静态资源配置

| 配置项 | 文件路径 | 加载位置 | 说明 |
|--------|--------|--------|------|
| **静态文件根目录** | 无配置文件 | `examples/basic_server.cc` L32 | 代码硬编码 |

**代码配置:**
```cpp
// examples/basic_server.cc 第 32 行
const static std::string STATIC_FILE_ROOT_ABS = 
    "/home/isyo/hello-git/net/resource";
```

**修改方式:**
编辑 `examples/basic_server.cc` 第 32 行，将路径改为你的资源目录。

**建议结构:**
```
resource/
├── index.html
├── login.html
├── register.html
├── css/
│   └── style.css
└── images/
    ├── 1.png
    ├── 3.jpg
    └── ...
```

---

### 服务器绑定端口

| 配置项 | 文件路径 | 加载位置 | 说明 |
|--------|--------|--------|------|
| **监听端口** | 无配置文件 | `examples/basic_server.cc` L27 | 代码硬编码 |

**代码配置:**
```cpp
// examples/basic_server.cc 第 27 行
InetAddress listenAddr(8888);  // 监听端口 8888
```

**修改方式:**
编辑 `examples/basic_server.cc` 第 27 行，改为你需要的端口。

---

## 🔧 配置修改指南

### 1. 修改 MySQL 连接信息

```bash
# 步骤 1: 复制配置文件
cp config/mysql.conf.example config/mysql.conf

# 步骤 2: 编辑配置
vim config/mysql.conf

# 步骤 3: 重启服务器
# 程序启动时会自动加载新配置
```

### 2. 修改日志输出

编辑 `examples/basic_server.cc` 的第 16-24 行：

```cpp
Logger::getInstance()
    ->setPrefixPath(".")                  // 改为你的日志基路径
    .setLogDirName("my_logs")             // 改为你的日志目录
    .setLogFileName("my_app")             // 改为你的日志文件名
    .setConsle(true)                      // 是否输出到控制台
    .setColor(true)                       // 是否启用颜色
    .setWritefile(true)                   // 是否保存到文件
    .setRotate(true)                      // 是否启用日志轮转
    .setFileMaxSize(10 * 1024 * 1024);    // 日志文件最大大小
```

### 3. 修改静态资源目录

编辑 `examples/basic_server.cc` 的第 32 行：

```cpp
const static std::string STATIC_FILE_ROOT_ABS = 
    "/path/to/your/resource";  // 改为你的资源目录
```

### 4. 修改服务器监听端口

编辑 `examples/basic_server.cc` 的第 27 行：

```cpp
InetAddress listenAddr(8888);  // 改为你需要的端口
```

---

## 📂 相关文件位置速查

### 配置相关

| 文件 | 路径 | 说明 |
|------|------|------|
| MySQL 配置示例 | `config/mysql.conf.example` | 数据库连接配置 |
| 主程序 | `examples/basic_server.cc` | 包含大多数硬编码配置 |

### 实现相关

| 文件 | 路径 | 说明 |
|------|------|------|
| 连接池头文件 | `include/connectionPool.h` | ConnectionPool 类定义 |
| 数据库连接 | `include/commonConnection.h` | Connection 类定义 |
| 用户服务 | `src/database/UserService.cc` | 数据库操作实现 |
| 日志系统 | `src/utils/logger.hpp` | Logger 类定义 |

### 文档相关

| 文件 | 路径 | 说明 |
|------|------|------|
| MySQL 配置指南 | `docs/MYSQL_CONFIG_GUIDE.md` | 详细的 MySQL 配置说明 |
| 日志使用指南 | `docs/LOGGER_GUIDE.md` | 详细的日志系统说明 |
| JSON 集成指南 | `docs/JSON_INTEGRATION.md` | JSON 模块使用说明 |

---

## 🚀 快速开始

### 第一次运行

```bash
# 1. 复制 MySQL 配置
cp config/mysql.conf.example config/mysql.conf

# 2. 编辑 MySQL 配置（修改数据库信息）
vim config/mysql.conf

# 3. 编译项目
bash scripts/build.sh

# 4. 运行程序
./build/yoyo-server
```

### 修改后重新运行

```bash
# 如果只修改了配置文件（如 mysql.conf）
./build/yoyo-server

# 如果修改了代码（如日志输出、监听端口等）
bash scripts/build.sh
./build/yoyo-server
```

---

## ⚠️ 注意事项

1. **MySQL 配置文件必须存在**
   - 程序启动前必须创建 `config/mysql.conf`
   - 可从 `config/mysql.conf.example` 复制

2. **代码中的硬编码配置**
   - 日志输出配置在 `examples/basic_server.cc` 第 16-24 行
   - 静态资源路径在 `examples/basic_server.cc` 第 32 行
   - 监听端口在 `examples/basic_server.cc` 第 27 行
   - 修改这些配置需要重新编译

3. **数据库连接池**
   - 连接数不应超过 MySQL 的 `max_connections / 2`
   - 建议生产环境设置 `MaxSize` 为 20-50

4. **日志文件**
   - 默认存储在 `server_logs/` 目录
   - 启用日志轮转后，单个文件不超过 10MB
   - 定期清理旧日志文件以节省磁盘空间

---

## 📚 详细文档

- [MySQL 配置指南](./MYSQL_CONFIG_GUIDE.md) - MySQL 连接和连接池配置
- [日志使用指南](./LOGGER_GUIDE.md) - 日志系统的使用和配置
- [JSON 集成指南](./JSON_INTEGRATION.md) - JSON 解析和处理
- [项目README](../README.md) - 项目总体介绍

---

**最后更新**: 2026-04-02
