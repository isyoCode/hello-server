# 动态库配置指南

本文档说明如何正确配置 YoyoCppServer 的动态库路径，确保程序能够正确找到所需的共享库文件。

## 📋 问题描述

当运行 `./build/yoyo-server` 时，可能遇到以下错误：

```bash
./build/yoyo-server: error while loading shared libraries: libmysqlpool.so: cannot open shared object file: No such file or directory
```

这是因为 Linux 动态链接器无法找到 `libmysqlpool.so` 库文件。

---

## ✅ 解决方案（推荐顺序）

### 方案 1: 使用 rpath with $ORIGIN（推荐，已实现）

**原理**: 在编译时使用 `$ORIGIN` 变量嵌入相对库路径，运行时动态链接器自动相对于可执行文件位置查找。

**优点**:
- ✅ 完全可移植，项目可移动到任意目录
- ✅ 无需配置环境变量
- ✅ 克隆到不同服务器无需修改
- ✅ 无需管理员权限
- ✅ Docker/容器友好

**实现方式**:

本项目已在 `scripts/build.sh` 中集成此方案：

```bash
# 编译时添加 -Wl,-rpath 参数，使用 $ORIGIN 相对路径
LIB_FLAGS="-L$SQLPOOL_LIB_PATH -Wl,-rpath,\$ORIGIN/../lib/vendor/sqlpool/v1.0 -lmysqlpool ..."
```

**验证**:

```bash
# 查看嵌入的 rpath
readelf -d build/yoyo-server | grep RUNPATH

# 输出应显示:
# 0x000000000000001d (RUNPATH)  Library runpath: [$ORIGIN/../lib/vendor/sqlpool/v1.0]

# 注意: ldd 可能显示 "not found"，这是正常的
# ldd 不展开 $ORIGIN，但运行时会正确加载
ldd build/yoyo-server | grep mysql

# 测试实际运行
./build/yoyo-server
# 如果库加载失败会立即报错，成功则说明 $ORIGIN 工作正常
```

**使用**:

```bash
# 编译（使用 $ORIGIN rpath）
bash scripts/build.sh

# 直接运行，无需额外配置
./build/yoyo-server

# 或从任何位置运行
/path/to/project/build/yoyo-server

# 移动项目到新位置
mv /old/path/hello-server /new/path/hello-server
/new/path/hello-server/build/yoyo-server  # 仍然可以运行
```

---

### 方案 2: 使用 LD_LIBRARY_PATH 环境变量

**原理**: 临时设置动态库搜索路径。

**优点**:
- ✅ 无需重新编译
- ✅ 灵活，可动态切换库版本

**缺点**:
- ❌ 每次运行都需要设置
- ❌ 环境变量污染

**实现方式**:

```bash
# 方法 1: 每次运行前设置
export LD_LIBRARY_PATH=/home/isyo/hello-server/lib/vendor/sqlpool/v1.0:$LD_LIBRARY_PATH
./build/yoyo-server

# 方法 2: 一次性运行
LD_LIBRARY_PATH=/home/isyo/hello-server/lib/vendor/sqlpool/v1.0 ./build/yoyo-server

# 方法 3: 添加到 ~/.bashrc（永久生效）
echo 'export LD_LIBRARY_PATH=/home/isyo/hello-server/lib/vendor/sqlpool/v1.0:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

---

### 方案 3: 修改系统库搜索路径（需要管理员权限）

**原理**: 将库路径添加到系统级动态库配置中。

**优点**:
- ✅ 永久方案
- ✅ 所有用户都能使用

**缺点**:
- ❌ 需要管理员权限
- ❌ 影响全局，可能引起冲突
- ❌ 不推荐用于开发库

**实现方式**:

```bash
# 创建配置文件
sudo bash -c 'echo "/home/isyo/hello-server/lib/vendor/sqlpool/v1.0" > /etc/ld.so.conf.d/yoyo-server.conf'

# 更新动态链接器缓存
sudo ldconfig

# 验证
ldconfig -p | grep mysqlpool
```

**清理**:

```bash
# 如果不再需要，删除配置
sudo rm /etc/ld.so.conf.d/yoyo-server.conf
sudo ldconfig
```

---

### 方案 4: 符号链接到系统库目录（不推荐）

**原理**: 将库文件链接到标准库路径。

**缺点**:
- ❌ 需要管理员权限
- ❌ 污染系统库目录
- ❌ 不利于版本管理
- ⛔ 不推荐使用

```bash
# 仅供参考，不推荐
sudo ln -s /home/isyo/hello-server/lib/vendor/sqlpool/v1.0/libmysqlpool.so /usr/local/lib/
sudo ldconfig
```

---

## 🔍 诊断工具

### 查看可执行文件依赖

```bash
# 查看所有动态库依赖
ldd build/yoyo-server

# 只查看 MySQL 相关库
ldd build/yoyo-server | grep mysql

# 查看未找到的库
ldd build/yoyo-server | grep "not found"
```

### 查看嵌入的 rpath

```bash
# 方法 1: 使用 readelf
readelf -d build/yoyo-server | grep -E "RPATH|RUNPATH"

# 方法 2: 使用 objdump
objdump -x build/yoyo-server | grep -E "RPATH|RUNPATH"

# 方法 3: 使用 chrpath（需安装）
chrpath -l build/yoyo-server
```

### 查看库文件信息

```bash
# 查看库文件类型
file lib/vendor/sqlpool/v1.0/libmysqlpool.so

# 查看库的依赖
ldd lib/vendor/sqlpool/v1.0/libmysqlpool.so

# 查看库导出的符号
nm -D lib/vendor/sqlpool/v1.0/libmysqlpool.so | grep ConnectionPool
```

### 调试运行时库加载

```bash
# 显示详细的库加载过程
LD_DEBUG=libs ./build/yoyo-server 2>&1 | grep mysql

# 查看所有库搜索路径
LD_DEBUG=libs ./build/yoyo-server 2>&1 | grep "search path"
```

---

## 📁 项目库结构

```
hello-server/
├── lib/
│   └── vendor/
│       └── sqlpool/
│           └── v1.0/
│               └── libmysqlpool.so  (260 KB)
├── build/
│   └── yoyo-server  (可执行文件，包含嵌入的 rpath)
└── scripts/
    ├── build.sh  (编译脚本，已配置 rpath)
    └── run.sh    (启动脚本，便捷运行)
```

---

## 🚀 快速使用

### 编译项目

```bash
# 使用 rpath 编译（推荐）
bash scripts/build.sh
```

### 运行服务器

```bash
# 方法 1: 直接运行（推荐，已嵌入 rpath）
./build/yoyo-server

# 方法 2: 使用启动脚本
bash scripts/run.sh

# 方法 3: 使用绝对路径
/home/isyo/hello-server/build/yoyo-server

# 方法 4: 临时设置环境变量（如果没有 rpath）
LD_LIBRARY_PATH=lib/vendor/sqlpool/v1.0 ./build/yoyo-server
```

---

## ❓ 常见问题

### Q1: rpath 和 RUNPATH 的区别？

- **RPATH**: 旧标准，优先级高于 `LD_LIBRARY_PATH`
- **RUNPATH**: 新标准（推荐），优先级低于 `LD_LIBRARY_PATH`，更灵活

现代 GCC 默认使用 RUNPATH (`-Wl,--enable-new-dtags`)。

### Q2: 为什么推荐 rpath 而不是 LD_LIBRARY_PATH？

- rpath 是永久方案，无需每次设置
- 避免环境变量污染
- 更符合软件分发规范
- Docker/容器化更友好

### Q3: 如何让可执行文件完全可移植？

**已实现**: 项目已使用 `$ORIGIN` 实现完全可移植。

```bash
# build.sh 中已配置
g++ ... -Wl,-rpath,\$ORIGIN/../lib/vendor/sqlpool/v1.0 -o yoyo-server

# 这样只要保持目录结构，可执行文件在任何位置都能运行
```

**测试可移植性**:
```bash
# 移动项目到任意位置
mv /home/isyo/hello-server /tmp/test-server

# 无需重新编译，直接运行
/tmp/test-server/build/yoyo-server  # ✅ 可以正常运行

# 甚至可以复制到其他服务器
scp -r hello-server user@remote:/opt/
ssh user@remote '/opt/hello-server/build/yoyo-server'  # ✅ 仍然可以运行
```

**详细说明**: 参见 [docs/PORTABLE_SETUP.md](PORTABLE_SETUP.md)

### Q4: 如何修改已编译文件的 rpath？

```bash
# 安装 patchelf 工具
sudo apt-get install patchelf

# 修改 rpath
patchelf --set-rpath /new/path/to/libs build/yoyo-server

# 验证
patchelf --print-rpath build/yoyo-server
```

### Q5: rpath 失效怎么办？

```bash
# 检查 rpath 是否存在
readelf -d build/yoyo-server | grep RUNPATH

# 如果没有输出，说明编译时未成功添加，需重新编译
bash scripts/build.sh

# 如果输出的路径不正确，使用 patchelf 修改
patchelf --set-rpath \$ORIGIN/../lib/vendor/sqlpool/v1.0 build/yoyo-server
```

### Q6: ldd 显示 "not found" 但程序能运行？

**这是正常现象！** 当使用 `$ORIGIN` 时，`ldd` 工具无法展开这个变量，因为它不知道可执行文件将从哪里运行。

```bash
# ldd 可能显示:
$ ldd build/yoyo-server | grep mysql
  libmysqlpool.so => not found              # ⚠️ ldd 无法解析 $ORIGIN
  libmysqlclient.so.21 => /lib/...          # ✅ 系统库可以找到

# 但实际运行时，动态链接器会正确展开 $ORIGIN
$ ./build/yoyo-server
📁 Resource directory: /path/to/project/resource
Server started...                            # ✅ 程序正常运行

# 如果真的加载失败，会立即报错:
error while loading shared libraries: libmysqlpool.so: cannot open shared object file
```

**验证方法**:
```bash
# 使用 LD_DEBUG 查看实际加载过程
LD_DEBUG=libs ./build/yoyo-server 2>&1 | grep libmysqlpool

# 应该显示成功加载的完整路径:
# trying file=/path/to/project/lib/vendor/sqlpool/v1.0/libmysqlpool.so
# libmysqlpool.so => /path/to/project/lib/vendor/sqlpool/v1.0/libmysqlpool.so
```

---

## 📚 参考文档

- [Linux 动态链接器 man 手册](https://man7.org/linux/man-pages/man8/ld.so.8.html)
- [GCC 链接选项文档](https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html)
- [ELF 格式规范](https://refspecs.linuxfoundation.org/elf/elf.pdf)

---

## 📝 相关文件

- [scripts/build.sh](../scripts/build.sh) - 编译脚本（已配置 rpath）
- [scripts/run.sh](../scripts/run.sh) - 启动脚本
- [lib/VENDOR_VERSIONS.md](../lib/VENDOR_VERSIONS.md) - 库版本管理
- [docs/QUICK_START.md](QUICK_START.md) - 快速开始指南

---

**最后更新**: 2026-04-03  
**维护者**: YoyoCppServer 项目组
