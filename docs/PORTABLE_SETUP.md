# 项目可移植性配置指南

本文档说明 YoyoCppServer 的可移植性设计，确保项目可以在不同服务器和目录下正常运行。

## 📋 概述

YoyoCppServer 采用完全可移植的设计，无需修改任何配置即可：
- ✅ 克隆到任意目录
- ✅ 移动到不同服务器
- ✅ 复制到其他用户目录
- ✅ 无需设置环境变量

---

## 🎯 可移植性特性

### 1. 动态库路径 - 使用 `$ORIGIN`

**原理**: 使用 `$ORIGIN` 变量，让动态链接器在运行时相对于可执行文件位置查找库文件。

**实现**:
```bash
# scripts/build.sh
LIB_FLAGS="-L$SQLPOOL_LIB_PATH -Wl,-rpath,\$ORIGIN/../lib/vendor/sqlpool/v1.0 -lmysqlpool ..."
```

**验证**:
```bash
# 查看嵌入的 rpath
readelf -d build/yoyo-server | grep RUNPATH

# 输出:
# 0x000000000000001d (RUNPATH)  Library runpath: [$ORIGIN/../lib/vendor/sqlpool/v1.0]
```

**工作原理**:
```
项目结构:
project/
├── build/
│   └── yoyo-server              (可执行文件)
└── lib/
    └── vendor/
        └── sqlpool/
            └── v1.0/
                └── libmysqlpool.so

运行时:
$ORIGIN = /path/to/project/build
$ORIGIN/../lib/vendor/sqlpool/v1.0 = /path/to/project/lib/vendor/sqlpool/v1.0
```

---

### 2. 静态资源路径 - 运行时计算

**原理**: 在程序启动时通过 `/proc/self/exe` 获取可执行文件路径，计算相对路径。

**实现**:
```cpp
// examples/basic_server.cc

// 获取可执行文件所在目录
std::string getExecutableDir() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count == -1) {
        std::cerr << "❌ Error: Failed to determine executable path" << std::endl;
        return ".";
    }
    result[count] = '\0';
    char* dir = dirname(result);
    return std::string(dir);
}

// 构建资源目录路径
std::string getResourcePath() {
    std::string execDir = getExecutableDir();
    // 可执行文件在 build/yoyo-server，资源在 resource/
    return execDir + "/../resource";
}

int main() {
    // 使用相对路径
    static const std::string STATIC_FILE_ROOT_ABS = getResourcePath();
    std::cout << "📁 Resource directory: " << STATIC_FILE_ROOT_ABS << std::endl;
    // ...
}
```

**工作原理**:
```
项目结构:
project/
├── build/
│   └── yoyo-server              (可执行文件)
└── resource/
    ├── index.html
    ├── css/
    └── images/

运行时:
可执行文件在: /any/path/project/build/yoyo-server
资源目录为: /any/path/project/resource
```

---

## 🚀 使用方法

### 克隆项目到新位置

```bash
# 克隆项目
git clone https://github.com/user/hello-server.git /new/location/hello-server
cd /new/location/hello-server

# 编译（无需修改任何配置）
bash scripts/build.sh

# 运行（无需设置环境变量）
./build/yoyo-server
```

### 移动已编译的项目

```bash
# 移动整个项目目录
mv /old/path/hello-server /new/path/hello-server

# 直接运行（无需重新编译）
/new/path/hello-server/build/yoyo-server
```

### 在不同服务器间部署

```bash
# 服务器 A: 编译
bash scripts/build.sh

# 打包项目（包含编译后的二进制）
tar czf yoyo-server.tar.gz \
    build/yoyo-server \
    lib/vendor/sqlpool/v1.0/libmysqlpool.so \
    resource/ \
    config/

# 服务器 B: 解压并运行
tar xzf yoyo-server.tar.gz
./build/yoyo-server
```

---

## 📁 必需的目录结构

为确保可移植性，必须保持以下相对目录结构：

```
project_root/
├── build/
│   └── yoyo-server                     # 可执行文件
├── lib/
│   └── vendor/
│       └── sqlpool/
│           └── v1.0/
│               └── libmysqlpool.so     # 动态库
├── resource/                            # 静态资源
│   ├── *.html
│   ├── css/
│   └── images/
└── config/                              # 配置文件（如需要）
    └── mysql.conf
```

**重要**: 只要保持这些目录的**相对位置**不变，项目根目录的绝对路径可以是任意值。

---

## 🔍 验证可移植性

### 测试 1: 库路径验证

```bash
# 查看嵌入的 rpath
readelf -d build/yoyo-server | grep RUNPATH

# 应该显示:
# Library runpath: [$ORIGIN/../lib/vendor/sqlpool/v1.0]

# 注意: 
# - 使用 $ORIGIN 而不是绝对路径
# - ldd 可能显示 "not found"（这是正常的，因为 ldd 不展开 $ORIGIN）
# - 实际运行时会正确加载
```

### 测试 2: 移动测试

```bash
# 1. 复制项目到临时目录
cp -r /home/isyo/hello-server /tmp/test-server
cd /tmp/test-server

# 2. 直接运行（无需重新编译）
./build/yoyo-server

# 3. 检查输出
# 应该显示: 📁 Resource directory: /tmp/test-server/resource
# 不应该出现 "cannot open shared object file" 错误
```

### 测试 3: 不同用户测试

```bash
# 切换到其他用户（如果有权限）
sudo -u otheruser bash -c "
    cp -r /home/isyo/hello-server /home/otheruser/hello-server
    cd /home/otheruser/hello-server
    ./build/yoyo-server
"
```

---

## ⚙️ 高级配置

### 自定义资源目录位置

如果需要将资源目录放在不同位置，修改 `basic_server.cc`:

```cpp
std::string getResourcePath() {
    std::string execDir = getExecutableDir();
    
    // 方案 1: 资源在 build 同级的 resource 目录
    return execDir + "/../resource";
    
    // 方案 2: 资源在 build 子目录中
    // return execDir + "/resource";
    
    // 方案 3: 资源在 data 目录中
    // return execDir + "/../data/static";
}
```

### 添加更多动态库

如果集成新的动态库，确保也使用 `$ORIGIN`:

```bash
# scripts/build.sh
NEW_LIB_PATH="$PROJECT_ROOT/lib/vendor/newlib/v1.0"
LIB_FLAGS="... -Wl,-rpath,\$ORIGIN/../lib/vendor/newlib/v1.0 -lnewlib"
```

---

## ❓ 常见问题

### Q1: `ldd` 显示 "not found" 但程序能运行？

**答**: 这是正常的。`ldd` 在检查时不展开 `$ORIGIN`，因为它不知道可执行文件将从哪里运行。实际运行时，动态链接器会正确展开 `$ORIGIN`。

**验证方法**:
```bash
# 运行程序并检查库加载
LD_DEBUG=libs ./build/yoyo-server 2>&1 | grep libmysqlpool

# 应该显示成功加载的完整路径
```

### Q2: 项目移动后出现 "No such file or directory"？

**可能原因**:
1. 目录结构被破坏（lib 或 resource 目录缺失）
2. 文件权限问题
3. 库文件损坏

**解决方法**:
```bash
# 检查目录结构
ls -R | grep -E "(lib|resource|build)"

# 检查库文件
file lib/vendor/sqlpool/v1.0/libmysqlpool.so

# 检查权限
ls -l build/yoyo-server
ls -l lib/vendor/sqlpool/v1.0/libmysqlpool.so
```

### Q3: 如何在容器中使用？

```dockerfile
# Dockerfile
FROM ubuntu:22.04

# 安装运行时依赖
RUN apt-get update && apt-get install -y \
    libmysqlclient21 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# 复制项目（保持目录结构）
COPY build/ /app/build/
COPY lib/ /app/lib/
COPY resource/ /app/resource/
COPY config/ /app/config/

WORKDIR /app

# 运行
CMD ["./build/yoyo-server"]
```

### Q4: 为什么使用 `$ORIGIN` 而不是 `LD_LIBRARY_PATH`？

**优势**:
- ✅ 无需设置环境变量
- ✅ 不污染全局环境
- ✅ 更安全（只影响本程序）
- ✅ 更符合软件分发标准
- ✅ Docker/容器友好

### Q5: Windows 或 macOS 怎么办？

**Windows**:
- 使用 `GetModuleFileName()` 获取可执行文件路径
- 动态库放在可执行文件同目录或使用相对路径

**macOS**:
- 使用 `@executable_path` 代替 `$ORIGIN`
- 修改 rpath: `-Wl,-rpath,@executable_path/../lib/vendor/sqlpool/v1.0`

---

## 📊 可移植性检查清单

部署前确认以下项目：

- [ ] rpath 使用 `$ORIGIN`（不是绝对路径）
- [ ] 资源路径在运行时计算（不是硬编码）
- [ ] 目录结构完整（build、lib、resource）
- [ ] 库文件存在且有执行权限（755）
- [ ] 配置文件使用相对路径
- [ ] 没有硬编码的绝对路径
- [ ] 在不同目录测试通过
- [ ] 在不同服务器测试通过（如需要）

---

## 🔗 相关文档

- [docs/DYNAMIC_LIBRARY_GUIDE.md](DYNAMIC_LIBRARY_GUIDE.md) - 动态库配置详解
- [scripts/build.sh](../scripts/build.sh) - 编译脚本
- [examples/basic_server.cc](../examples/basic_server.cc) - 主程序入口
- [resource/README.md](../resource/README.md) - 资源目录说明

---

**最后更新**: 2026-04-03  
**维护者**: YoyoCppServer 项目组
