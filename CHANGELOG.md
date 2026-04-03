# 更新日志 (Changelog)

本文档记录 YoyoCppServer 项目的所有重要变更。

遵循 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/) 规范，版本号采用 [语义化版本](https://semver.org/lang/zh-CN/)。

---

## [v2.1.0] - 2026-04-02

### ✨ 新增功能

- **游戏统计 API 完整实现**
  - 添加 `/api/user/game-stats` GET 端点获取游戏统计
  - 添加 `/api/save-game-stats` POST 端点保存游戏数据
  - 支持三种游戏类型：俄罗斯方块 (tetris)、格斗 (fighter)、打鱼 (fishing)
  - 实现游戏分数、时长、排行榜功能

- **JSON 模块集成**
  - 自定义 JSON 解析器 (`json_parser.hpp`)
  - HTTP-JSON 集成工具 (`json_utils.h`)
  - JSON 请求/响应处理示例 (`JsonExampleHandler`)
  - 支持嵌套对象、数组、类型安全访问

- **MySQL 连接池优化**
  - SqlPool 库集成到项目 (lib/vendor/sqlpool/v1.0/)
  - 项目自包含，无需外部依赖编译
  - 改进连接池配置加载流程
  - 添加连接超时和自动重连机制

### 📚 文档更新

- 添加 [浏览器请求示例](docs/BROWSER_REQUESTS.md) - 完整的 API 测试指南
- 添加 [JSON 集成指南](docs/JSON_INTEGRATION.md) - JSON 模块使用文档
- 添加 [库版本管理](lib/VENDOR_VERSIONS.md) - 第三方库追踪
- 完善 [MySQL 配置指南](docs/MYSQL_CONFIG_GUIDE.md)
- 更新 [架构设计文档](docs/ARCHITECTURE.md)

### 🔧 改进

- **性能优化**
  - 减少不必要的字符串拷贝
  - 优化 JSON 解析性能
  - 改进数据库连接池效率

- **代码质量**
  - 统一错误处理机制
  - 添加输入验证和安全检查
  - 改进日志输出格式

### 🐛 Bug 修复

- 修复 JSON 解析器对转义字符的处理
- 修复数据库连接池偶尔的死锁问题
- 修复 HTTP 响应头 Content-Length 计算错误
- 修复游戏页面静态资源路径问题

### 🔒 安全更新

- 添加 SQL 注入防护
- 改进密码哈希存储 (SHA256)
- 添加输入长度限制
- 完善会话管理机制

---

## [v2.0.0] - 2026-03-15

### ✨ 新增功能

- **用户认证系统**
  - 用户注册和登录功能
  - SHA256 密码哈希
  - 会话管理

- **游戏系统集成**
  - 俄罗斯方块游戏 (game1.html)
  - 格斗竞技游戏 (game2.html)
  - 游戏页面路由

- **静态资源服务**
  - CSS、JavaScript、图片资源支持
  - MIME 类型自动识别
  - 缓存控制头

### 🔧 改进

- 完善路由系统
- 添加请求日志
- 改进错误处理

### 📚 文档

- 添加 [快速开始指南](docs/QUICK_START.md)
- 添加 [API 参考文档](docs/API_REFERENCE.md)
- 添加 [日志系统指南](docs/LOGGER_GUIDE.md)

---

## [v1.5.0] - 2026-02-28

### ✨ 新增功能

- **MySQL 数据库集成**
  - 连接池实现
  - 基本 CRUD 操作
  - 配置文件加载 (mysql.conf)

- **线程池支持**
  - CPU 密集任务异步处理
  - 可配置线程数量

### 🔧 改进

- 优化事件循环性能
- 改进内存管理

---

## [v1.0.0] - 2026-02-01

### ✨ 首次发布

- **核心网络库**
  - Reactor 模式实现
  - Epoll I/O 多路复用
  - 单线程事件驱动

- **HTTP/1.1 支持**
  - 请求解析器
  - 响应生成器
  - HTTP 方法支持 (GET, POST)

- **基础功能**
  - TCP 服务器封装
  - 路由系统
  - 日志模块

- **文档**
  - README
  - 架构设计文档
  - LICENSE (MIT)

---

## 📋 版本说明

### 版本格式

本项目采用 **语义化版本** (Semantic Versioning) 2.0.0:

- **主版本号 (MAJOR)**: 不兼容的 API 变更
- **次版本号 (MINOR)**: 向后兼容的功能新增
- **修订号 (PATCH)**: 向后兼容的问题修复

格式: `MAJOR.MINOR.PATCH` (例如: 2.1.0)

### 变更类型

- `✨ 新增功能` - 新的功能特性
- `🔧 改进` - 对现有功能的改进
- `🐛 Bug 修复` - 问题修复
- `🔒 安全更新` - 安全相关的修复
- `📚 文档` - 文档更新
- `⚠️ 破坏性变更` - 不兼容的 API 变更
- `🗑️ 废弃` - 即将移除的功能
- `🔥 移除` - 已移除的功能

---

## 🚧 开发中 (Unreleased)

### 计划功能

- [ ] WebSocket 支持
- [ ] HTTPS/TLS 加密
- [ ] Redis 缓存集成
- [ ] 压力测试工具
- [ ] Docker 容器化
- [ ] CI/CD 流水线

### 考虑中的改进

- [ ] HTTP/2 支持
- [ ] 配置热重载
- [ ] 插件系统
- [ ] 性能监控面板

---

## 🔗 相关链接

- **项目主页**: https://github.com/yourusername/yoyo_cpp_server
- **问题追踪**: https://github.com/yourusername/yoyo_cpp_server/issues
- **贡献指南**: [CONTRIBUTING.md](../CONTRIBUTING.md)
- **许可证**: [MIT License](../LICENSE)

---

## 📞 反馈

如果您发现任何问题或有功能建议，请通过以下方式联系我们:

- 提交 [GitHub Issue](https://github.com/yourusername/yoyo_cpp_server/issues)
- 发起 [Pull Request](https://github.com/yourusername/yoyo_cpp_server/pulls)
- 在讨论区分享您的想法

---

**维护者**: YoyoCppServer 项目组  
**最后更新**: 2026-04-03
