# Contributing to YoyoCppServer

感谢您对 YoyoCppServer 的兴趣！我们欢迎所有形式的贡献。

## 贡献方式

### 报告 Bug

如果您发现了 Bug，请创建一个 Issue 并包含：

1. **问题描述** - 清楚地描述您遇到的问题
2. **重现步骤** - 详细的步骤来重现问题
3. **预期行为** - 应该发生什么
4. **实际行为** - 实际发生了什么
5. **环境信息** - 操作系统、编译器版本、库版本等
6. **错误日志** - 相关的错误消息或堆栈跟踪

### 提交功能请求

对于新功能建议：

1. **功能描述** - 清晰地描述你想要的功能
2. **用例** - 为什么需要这个功能
3. **设计建议** - 如果可能的话，包含实现思路
4. **相关问题** - 链接到任何相关的问题

### 提交代码

#### 开发流程

1. **Fork 仓库**
   ```bash
   git clone https://github.com/yourusername/yoyo_cpp_server.git
   cd yoyo_cpp_server
   ```

2. **创建特性分支**
   ```bash
   git checkout -b feature/your-feature-name
   # 或 bugfix/issue-name
   ```

3. **本地编译和测试**
   ```bash
   bash scripts/build.sh
   ./build/yoyo-server
   ```

4. **提交更改**
   ```bash
   git add .
   git commit -m "description of changes"
   ```

5. **推送到 GitHub**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **创建 Pull Request**
   - 描述您的更改
   - 引用相关的 Issue（使用 #123）
   - 列出测试方法

#### 代码风格

请遵循以下代码风格指南：

- **C++ 标准**: C++17 或更高版本
- **命名约定**:
  - 类名: PascalCase (e.g., `TcpServer`)
  - 函数名: camelCase (e.g., `handleRequest`)
  - 常量: UPPER_CASE (e.g., `MAX_BUFFER_SIZE`)
  - 私有成员: m_前缀 (e.g., `m_socket`)

- **代码格式**:
  - 缩进: 4 个空格
  - 行长: 最多 100 个字符
  - 使用大括号: Allman 风格

- **文档**:
  - 为公共函数添加注释
  - 使用 Doxygen 风格的文档注释
  - 清晰地解释复杂的算法

#### 示例代码风格

```cpp
// ============================================================================
// TcpServer.h - TCP 服务器实现
// ============================================================================

namespace yoyo {
namespace network {

class TcpServer {
public:
    /// 创建 TCP 服务器
    /// @param port 监听的端口号
    /// @return 成功返回 true
    bool start(int port);

    /// 停止服务器
    void stop();

private:
    // 私有成员变量
    int m_port;
    int m_socket;
    
    // 私有方法
    void handleConnection();
};

}  // namespace network
}  // namespace yoyo
```

### 提交 PR 检查清单

提交 Pull Request 前，请确保：

- [ ] 代码遵循项目风格指南
- [ ] 已添加/更新相关文档
- [ ] 代码能够成功编译（`bash scripts/build.sh`）
- [ ] 已测试新功能/Bug 修复
- [ ] 没有引入新的编译警告
- [ ] 提交信息清晰有意义
- [ ] PR 描述详细完整

## 开发设置

### 依赖

- C++17 编译器 (g++, clang)
- MySQL 开发库（如果需要数据库支持）
- OpenSSL 开发库
- Linux/Unix 系统（推荐）

### 构建项目

```bash
# 编译完整项目
bash scripts/build.sh

# 编译输出
./build/yoyo-server

# 使用数据库支持编译（如果需要）
# 编辑 scripts/build.sh 中的 WITH_MYSQL 变量
```

### 运行测试

```bash
# 编译后运行
cd build
./run-tests

# 或使用 examples
cd ../examples
./run-example
```

## 代码审查流程

所有 PR 都会进行代码审查，审查内容包括：

1. **正确性** - 代码是否正确解决了问题？
2. **性能** - 是否有性能考虑？
3. **可维护性** - 代码是否易于理解和维护？
4. **测试** - 是否有充分的测试？
5. **文档** - 是否包含必要的文档？

## 社区指南

- 尊重其他贡献者
- 友好和建设性地讨论
- 避免人身攻击或不尊重的言论
- 遵守开源社区的行为准则

## 许可证

通过提交代码，您同意您的贡献将根据项目的 MIT 许可证进行许可。

## 问题或建议？

- 在 GitHub Issues 中提问
- 查看现有的文档
- 联系项目维护者

感谢您的贡献！🎉
