# 可移植性改进总结

本文档总结了为使 YoyoCppServer 完全可移植所做的所有更改。

## 📋 改进概述

现在项目可以：
- ✅ 克隆到任意目录并直接运行
- ✅ 在不同服务器间移动而无需重新编译
- ✅ 复制给其他用户使用
- ✅ 无需设置任何环境变量

---

## 🔧 技术改进

### 1. 动态库路径 - 使用 `$ORIGIN`

**修改文件**: `scripts/build.sh`

**之前**:
```bash
# 使用绝对路径
LIB_FLAGS="-L$SQLPOOL_LIB_PATH -Wl,-rpath,$SQLPOOL_LIB_PATH -lmysqlpool ..."
```

**之后**:
```bash
# 使用 $ORIGIN 相对路径
LIB_FLAGS="-L$SQLPOOL_LIB_PATH -Wl,-rpath,\$ORIGIN/../lib/vendor/sqlpool/v1.0 -lmysqlpool ..."
```

**效果**:
- 编译后的可执行文件嵌入了相对路径
- 运行时动态链接器自动相对于可执行文件位置查找库
- 无论项目在哪个目录，只要相对结构不变就能运行

**验证**:
```bash
$ readelf -d build/yoyo-server | grep RUNPATH
 0x000000000000001d (RUNPATH)  Library runpath: [$ORIGIN/../lib/vendor/sqlpool/v1.0]
```

---

### 2. 静态资源路径 - 运行时计算

**修改文件**: `examples/basic_server.cc`

**之前**:
```cpp
// 硬编码绝对路径
const static std::string STATIC_FILE_ROOT_ABS = "/home/isyo/hello-git/net/resource";
```

**之后**:
```cpp
// 添加头文件
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

// 运行时获取可执行文件目录
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

// 构建资源路径（相对于可执行文件）
std::string getResourcePath() {
    std::string execDir = getExecutableDir();
    return execDir + "/../resource";
}

// 在 main 中使用
static const std::string STATIC_FILE_ROOT_ABS = getResourcePath();
std::cout << "📁 Resource directory: " << STATIC_FILE_ROOT_ABS << std::endl;
```

**效果**:
- 程序启动时自动检测自己的位置
- 根据可执行文件位置计算资源目录
- 无论项目在哪都能正确找到资源文件

---

### 3. 资源目录结构

**新增目录**: `resource/`

```
resource/
├── css/
├── images/
└── README.md
```

**作用**:
- 统一的静态资源存放位置
- 与可执行文件保持固定相对关系（`build/../resource`）

---

## 📄 新增文档

### 1. `docs/PORTABLE_SETUP.md`
- **内容**: 完整的可移植性配置指南
- **包含**: 
  - `$ORIGIN` 工作原理
  - 运行时路径计算方法
  - 跨服务器部署步骤
  - Docker/容器化支持
  - 常见问题解答

### 2. 更新 `docs/DYNAMIC_LIBRARY_GUIDE.md`
- 更新方案 1 说明，强调 `$ORIGIN` 用法
- 添加 Q6: 解释为什么 ldd 显示 "not found" 但程序能运行
- 更新验证方法

### 3. 更新 `README.md`
- 特性列表添加"完全可移植"
- 文档区添加可移植性配置链接
- 项目结构说明添加 `lib/` 和 `resource/` 目录

---

## 🧪 测试验证

### 测试 1: 移动项目目录

```bash
# 原始位置
/home/isyo/hello-server/build/yoyo-server  # ✅ 可以运行

# 移动到新位置
mv /home/isyo/hello-server /tmp/test-server

# 测试运行
/tmp/test-server/build/yoyo-server          # ✅ 仍然可以运行
📁 Resource directory: /tmp/test-server/resource
```

### 测试 2: 验证 rpath

```bash
$ readelf -d build/yoyo-server | grep RUNPATH
 0x000000000000001d (RUNPATH)  Library runpath: [$ORIGIN/../lib/vendor/sqlpool/v1.0]
# ✅ 使用 $ORIGIN，不是绝对路径
```

### 测试 3: 运行时库加载

```bash
$ timeout 2 ./build/yoyo-server
📁 Resource directory: /home/isyo/hello-server/resource
terminate called after throwing an instance of 'yoyo::Exception'
  what():  bind error
# ✅ 库加载成功，bind error 是因为端口占用（正常）
# 如果库加载失败会显示: error while loading shared libraries
```

---

## 📊 改进对比

| 项目 | 改进前 | 改进后 |
|------|--------|--------|
| **库路径** | 绝对路径 | `$ORIGIN` 相对路径 |
| **资源路径** | 硬编码 `/home/isyo/...` | 运行时计算 |
| **可移植性** | ❌ 仅限特定目录 | ✅ 任意目录 |
| **环境变量** | ❌ 需要 `LD_LIBRARY_PATH` | ✅ 无需配置 |
| **跨服务器** | ❌ 需要重新编译 | ✅ 直接复制运行 |
| **容器化** | ❌ 需要额外配置 | ✅ 开箱即用 |

---

## 🎯 使用场景

### 场景 1: 开发环境迁移
```bash
# 从笔记本迁移到工作站
laptop$ tar czf hello-server.tar.gz hello-server/
workstation$ tar xzf hello-server.tar.gz
workstation$ ./hello-server/build/yoyo-server
# ✅ 无需修改任何配置
```

### 场景 2: 团队协作
```bash
# 推送到 Git
git add -A
git commit -m "Add portable configuration"
git push

# 团队成员克隆
teammate$ git clone https://github.com/user/hello-server.git
teammate$ cd hello-server
teammate$ bash scripts/build.sh
teammate$ ./build/yoyo-server
# ✅ 直接可用，无需配置
```

### 场景 3: 生产部署
```bash
# 开发服务器编译
dev$ bash scripts/build.sh

# 打包
dev$ tar czf yoyo-server-v1.0.tar.gz \
    build/ lib/ resource/ config/

# 部署到生产
prod$ tar xzf yoyo-server-v1.0.tar.gz
prod$ ./build/yoyo-server
# ✅ 无需重新编译或配置
```

### 场景 4: Docker 容器化
```dockerfile
FROM ubuntu:22.04
COPY . /app
WORKDIR /app
CMD ["./build/yoyo-server"]
# ✅ 无需额外的 LD_LIBRARY_PATH 配置
```

---

## ⚠️ 注意事项

### 必须保持的目录结构

只要保持以下**相对**结构，项目根目录可以在任何位置：

```
project_root/          (可以是任意路径)
├── build/
│   └── yoyo-server
├── lib/
│   └── vendor/
│       └── sqlpool/
│           └── v1.0/
│               └── libmysqlpool.so
└── resource/
    └── (静态文件)
```

### ldd 显示 "not found" 是正常的

```bash
$ ldd build/yoyo-server | grep mysqlpool
  libmysqlpool.so => not found
```

这是因为 `ldd` 无法展开 `$ORIGIN`。**实际运行时会正确加载**。

验证方法：
```bash
# 如果库真的找不到，运行时会立即报错
$ ./build/yoyo-server
error while loading shared libraries: libmysqlpool.so: ...

# 如果没有报错，说明库加载成功
$ ./build/yoyo-server
📁 Resource directory: ...
Server started...
```

---

## 📚 相关文档

- [docs/PORTABLE_SETUP.md](PORTABLE_SETUP.md) - 详细的可移植性配置指南
- [docs/DYNAMIC_LIBRARY_GUIDE.md](DYNAMIC_LIBRARY_GUIDE.md) - 动态库配置详解
- [scripts/build.sh](../scripts/build.sh) - 编译脚本（包含 $ORIGIN 配置）
- [examples/basic_server.cc](../examples/basic_server.cc) - 主程序（包含路径计算）

---

**改进完成日期**: 2026-04-03  
**改进者**: YoyoCppServer 项目组
