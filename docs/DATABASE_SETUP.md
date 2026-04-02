# MySQL 数据库初始化指南

## 📋 概述

YoyoCppServer 使用 MySQL 数据库存储用户信息和游戏统计数据。本指南说明如何设置和初始化数据库。

---

## 📁 相关文件

| 文件 | 位置 | 用途 |
|------|------|------|
| **mysql.conf** | 项目根目录 | 数据库连接配置 |
| **init.sql** | `database/init.sql` | SQL 初始化脚本 |
| **mysql.conf.example** | `config/mysql.conf.example` | 配置文件示例 |

---

## 🚀 快速初始化

### 方式 1：使用 SQL 脚本初始化（推荐）

#### 步骤 1：检查 MySQL 是否运行
```bash
mysql -h 127.0.0.1 -u root -p123456 -e "SELECT 1"
```

如果看到 `1 | 1` 表示连接成功。

#### 步骤 2：运行初始化脚本
```bash
# 从项目根目录执行
mysql -h 127.0.0.1 -u root -p123456 < database/init.sql
```

#### 步骤 3：验证数据库创建成功
```bash
mysql -h 127.0.0.1 -u root -p123456 -D hello_app -e "SHOW TABLES;"
```

应该看到：
```
Tables_in_hello_app
users
game_statistics
```

#### 步骤 4：验证表结构
```bash
# 查看 users 表
mysql -h 127.0.0.1 -u root -p123456 -D hello_app -e "DESC users;"

# 查看 game_statistics 表
mysql -h 127.0.0.1 -u root -p123456 -D hello_app -e "DESC game_statistics;"
```

### 方式 2：手动在 MySQL 中执行

```bash
# 1. 连接到 MySQL
mysql -h 127.0.0.1 -u root -p123456

# 2. 在 MySQL 命令行中执行
mysql> SOURCE database/init.sql;

# 或者直接复制 init.sql 的内容到 MySQL 中执行
```

---

## 📊 数据库结构

### users 表（用户信息表）

| 字段 | 类型 | 说明 |
|------|------|------|
| `id` | INT UNSIGNED | 用户 ID (自增主键，从 1000 开始) |
| `username` | VARCHAR(50) | 用户名 (唯一) |
| `email` | VARCHAR(100) | 邮箱 (唯一) |
| `password_hash` | VARCHAR(255) | 密码哈希值 |
| `created_at` | TIMESTAMP | 创建时间 |
| `updated_at` | TIMESTAMP | 更新时间 |
| `status` | ENUM | 账户状态: active, inactive, banned |
| `login_attempts` | INT UNSIGNED | 登录失败次数 |
| `last_login` | TIMESTAMP | 最后登录时间 |

**索引**:
- `PRIMARY KEY (id)`
- `UNIQUE KEY idx_username (username)`
- `UNIQUE KEY idx_email (email)`
- `KEY idx_status (status)`
- `KEY idx_created_at (created_at)`
- `KEY idx_last_login (last_login)`

### game_statistics 表（游戏统计表）

| 字段 | 类型 | 说明 |
|------|------|------|
| `id` | INT UNSIGNED | 统计记录 ID (自增主键) |
| `user_id` | INT UNSIGNED | 用户 ID (外键) |
| `game_type` | VARCHAR(50) | 游戏类型: fishing, tetris, fighter |
| `highest_score` | INT | 最高分 |
| `times_played` | INT | 游玩次数 |
| `total_score` | INT | 总分 |
| `average_score` | FLOAT | 平均分 |
| `last_played` | TIMESTAMP | 最后游玩时间 |
| `created_at` | TIMESTAMP | 创建时间 |
| `updated_at` | TIMESTAMP | 更新时间 |

**索引**:
- `PRIMARY KEY (id)`
- `FOREIGN KEY (user_id) REFERENCES users(id)`
- `UNIQUE KEY unique_user_game (user_id, game_type)`
- `KEY idx_user_id (user_id)`
- `KEY idx_game_type (game_type)`
- `KEY idx_last_played (last_played)`

---

## 🔄 数据库连接配置

### 配置文件位置
**文件**: `mysql.conf` (项目根目录)

或 `config/mysql.conf.example` (示例文件)

### 配置参数

```ini
ip=127.0.0.1               # MySQL 服务器 IP
port=3306                  # MySQL 端口
username=root              # 数据库用户名
passwd=123456              # 数据库密码
dbname=hello_app           # 数据库名
initSize=5                 # 连接池初始连接数
MaxSize=10                 # 连接池最大连接数
maxIdleTime=300            # 最大空闲时间(秒)
maxConnectionTimeOut=3000  # 连接超时(毫秒)
```

### 修改配置

1. 编辑 `mysql.conf` 文件
2. 根据你的 MySQL 实际配置修改参数
3. 保存文件
4. 重启应用程序

---

## 📝 样本数据

### 初始用户

| username | email | password_hash | 说明 |
|----------|-------|---------------|------|
| admin | admin@example.com | 占位符 | 管理员账户 |
| testuser | testuser@example.com | 占位符 | 测试账户 |

**注意**: 样本密码只是占位符，生产环境需要使用真实的哈希值。

### 初始化游戏统计

每个用户在创建时会为 3 种游戏自动创建统计记录：
- fishing (打鱼)
- tetris (俄罗斯方块)
- fighter (格斗)

初始分数都是 0。

---

## 🔍 验证数据库

### 1. 检查数据库是否创建
```bash
mysql -h 127.0.0.1 -u root -p123456 -e "SHOW DATABASES;" | grep hello_app
```

### 2. 检查表是否创建
```bash
mysql -h 127.0.0.1 -u root -p123456 -D hello_app -e "SHOW TABLES;"
```

### 3. 检查样本数据
```bash
mysql -h 127.0.0.1 -u root -p123456 -D hello_app -e "SELECT * FROM users;"
```

### 4. 检查游戏统计
```bash
mysql -h 127.0.0.1 -u root -p123456 -D hello_app -e "SELECT * FROM game_statistics LIMIT 5;"
```

### 5. 检查索引
```bash
mysql -h 127.0.0.1 -u root -p123456 -D hello_app -e "SHOW INDEXES FROM users;"
```

---

## ⚙️ 常见问题

### Q: 为什么程序可以在没有 mysql.init 的情况下运行？

**A**: 
1. MySQL 的 `mysql.conf` 配置文件是程序启动时必须的
2. `mysql.conf` 和 `mysql.init` 是两个不同的文件：
   - `mysql.conf`: 连接参数配置（必需）
   - `mysql.init`: 数据库初始化脚本（可选）
3. 只要数据库和表已经存在，程序就可以直接使用
4. 初始化脚本只在首次设置时需要运行

### Q: 为什么要使用连接池？

**A**:
- 避免频繁创建/销毁数据库连接
- 提高性能：复用已有连接
- 减少资源占用
- 自动管理连接生命周期

### Q: 连接池中的连接会自动清理吗？

**A**: 是的。连接池会自动清理空闲连接：
- 空闲超过 `maxIdleTime` (默认 300 秒)
- 后台扫描线程定期检查和清理

### Q: 如何修改数据库名称？

**A**:
1. 创建新的数据库
2. 运行 SQL 脚本创建表
3. 修改 `mysql.conf` 中的 `dbname` 参数
4. 重启程序

### Q: 密码存储使用的是什么算法？

**A**:
- 代码中使用 SHA256
- 生产环境建议使用 bcrypt 或 Argon2
- 不要存储明文密码

---

## 🔐 安全建议

### 1. 修改默认密码
```bash
# 修改 root 用户密码
mysql -u root -p123456 -e "ALTER USER 'root'@'localhost' IDENTIFIED BY 'new_password';"
```

### 2. 创建专用数据库用户
```bash
mysql -u root -p123456 -e "
CREATE USER 'app_user'@'127.0.0.1' IDENTIFIED BY 'app_password';
GRANT ALL PRIVILEGES ON hello_app.* TO 'app_user'@'127.0.0.1';
FLUSH PRIVILEGES;
"
```

### 3. 不要在配置文件中硬编码密码
使用环境变量或密钥管理系统：
```bash
# 在启动脚本中
export DB_PASSWORD="your_password"
```

### 4. 定期备份数据库
```bash
# 备份整个数据库
mysqldump -h 127.0.0.1 -u root -p123456 hello_app > backup.sql

# 恢复备份
mysql -h 127.0.0.1 -u root -p123456 hello_app < backup.sql
```

---

## 📚 参考文档

- [MYSQL_CONFIG_GUIDE.md](./MYSQL_CONFIG_GUIDE.md) - MySQL 配置指南
- [MYSQL_LOADING_FLOW.md](./MYSQL_LOADING_FLOW.md) - 配置加载流程
- [README.md](../README.md) - 项目主文档

---

**最后更新**: 2026-04-02
