# MySQL 配置文件位置说明

## 📁 当前位置

**MySQL 配置文件应该放在**: `config/mysql.conf`

```
yoyo_cpp_server/
├── config/
│   ├── mysql.conf           # ✅ 实际配置文件（生产用）
│   └── mysql.conf.example   # 📋 配置文件示例
└── ...
```

## 为什么在 config 目录下？

### ✅ 优势

1. **组织清晰** - 所有配置文件在一个地方
2. **易于管理** - 与示例文件 `mysql.conf.example` 在一起
3. **规范标准** - 遵循项目结构最佳实践
4. **安全** - 配置文件通常在版本控制中被忽略 (`.gitignore`)

### 目录结构对比

**不推荐** (混乱):
```
yoyo_cpp_server/
├── mysql.conf              ❌ 在根目录
├── mysql.conf.example      
├── config/
│   └── other-config.conf
└── ...
```

**推荐** (清晰):
```
yoyo_cpp_server/
├── config/
│   ├── mysql.conf          ✅ 在 config 目录
│   └── mysql.conf.example
└── ...
```

## 🔍 程序如何找到配置文件？

### ConnectionPool 查找流程

当程序运行时，ConnectionPool 会按以下顺序查找配置文件：

```
程序启动
    ↓
ConnectionPool::getConnectionPool()
    ↓
自动查找 mysql.conf
    ↓
查找位置 1: ./config/mysql.conf
查找位置 2: ./mysql.conf
查找位置 3: /etc/mysql.conf
    ↓
找到 ✅ 加载配置
未找到 ❌ 使用默认值或报错
```

## 📝 使用步骤

### 第一次设置

```bash
cd /home/isyo/yoyo_cpp_server

# 1. 从示例复制
cp config/mysql.conf.example config/mysql.conf

# 2. 编辑配置
vim config/mysql.conf

# 3. 修改参数
# ip=127.0.0.1
# port=3306
# username=root
# passwd=123456
# dbname=hello_app

# 4. 编译和运行
bash scripts/build.sh
./build/yoyo-server
```

### 后续使用

```bash
# 修改配置
vim config/mysql.conf

# 重启程序 (无需重新编译)
./build/yoyo-server
```

## 🔒 .gitignore 设置

为了避免提交敏感的密码信息，应该在 `.gitignore` 中忽略实际配置文件：

```bash
# .gitignore
config/mysql.conf          # 忽略实际配置 (包含密码)
!config/mysql.conf.example # 保留示例文件
```

这样：
- ✅ `mysql.conf.example` 被提交到 Git (作为示例)
- ❌ `mysql.conf` 不被提交 (保护密码)

## 📋 文件内容

### mysql.conf.example (示例，提交到 Git)
```ini
# MySQL 连接池配置文件
# 该文件被 SqlPool 的 ConnectionPool 读取

# 数据库连接信息
ip=127.0.0.1
port=3306

# 认证信息
username=root
passwd=123456

# 数据库名
dbname=hello_app

# 连接池配置
initSize=5
MaxSize=10
maxIdleTime=300
maxConnectionTimeOut=3000
```

### mysql.conf (实际配置，不提交到 Git)
```ini
# 生产环境中会是实际的敏感信息
ip=192.168.1.100        # 实际数据库服务器
port=3306
username=app_user       # 实际用户名
passwd=secure_password  # 实际密码！
dbname=production_db    # 实际数据库
initSize=10
MaxSize=20
maxIdleTime=300
maxConnectionTimeOut=3000
```

## 🚀 快速命令

```bash
# 查看当前配置
cat config/mysql.conf

# 查看示例配置
cat config/mysql.conf.example

# 编辑配置
vim config/mysql.conf

# 验证配置文件存在
ls -l config/mysql.conf
```

## ⚠️ 常见问题

### Q: 程序无法连接数据库？
**A:** 检查 `config/mysql.conf` 是否存在
```bash
ls -l config/mysql.conf
# 如果不存在，运行:
cp config/mysql.conf.example config/mysql.conf
vim config/mysql.conf  # 修改参数
```

### Q: 配置文件应该提交到 Git 吗？
**A:** 不应该！使用 `.gitignore` 忽略实际配置文件

```bash
# 在 .gitignore 中
config/mysql.conf       # 不提交实际配置 (含密码)
!config/mysql.conf.example  # 提交示例文件
```

### Q: 如何在生产环境中安全地管理配置？
**A:** 使用以下方法之一：

1. **环境变量**
   ```bash
   export DB_PASSWORD="production_password"
   ./build/yoyo-server
   ```

2. **配置管理工具** (Ansible, Chef, Puppet)

3. **密钥管理服务** (AWS Secrets Manager, HashiCorp Vault)

4. **Docker secrets** (如果使用容器化)

## 📚 相关文档

- [MYSQL_CONFIG_GUIDE.md](./MYSQL_CONFIG_GUIDE.md) - MySQL 配置详细指南
- [DATABASE_SETUP.md](./DATABASE_SETUP.md) - 数据库初始化指南
- [README.md](../README.md) - 项目主文档

## ✅ 总结

| 项目 | 值 |
|------|-----|
| **配置文件位置** | `config/mysql.conf` ✅ |
| **示例文件位置** | `config/mysql.conf.example` ✅ |
| **是否提交到 Git** | 实际配置: ❌ / 示例: ✅ |
| **何时修改** | 首次设置和需要更改数据库时 |
| **何时重新编译** | 不需要！只需修改配置文件并重启程序 |

---

**最后更新**: 2026-04-02
