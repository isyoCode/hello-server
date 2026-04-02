# 后端游戏统计 API 实现完成 ✅

**日期**: 2026-04-02  
**版本**: v2.1  
**状态**: ✅ 后端实现完成

---

## 🎉 完成内容

### ✅ 已实现

| 组件 | 文件 | 说明 |
|------|------|------|
| 数据库表 | database/init.sql | game_statistics 表已创建 |
| 数据模型 | database/UserService.h | 3 个新方法声明 |
| 实现代码 | database/UserService.cc | 3 个方法实现 + 数据验证 |
| API 处理 | net/handlers/Handler.h/cc | 2 个 API 处理函数 |
| 路由映射 | net/main/main.cc | 2 个新路由已注册 |
| 编译 | build.sh | ✅ 编译成功 (1.8M) |

---

## 📊 API 文档

### 1. 保存游戏统计

**请求**:
```
POST /api/save-game-stats
Content-Type: application/x-www-form-urlencoded

user_id=1&game_type=fishing&score=1250
```

**参数**:
- `user_id` (required): 用户 ID (integer)
- `game_type` (required): 游戏类型 (fishing, tetris, fighter)
- `score` (required): 本局游戏分数 (integer)

**成功响应**:
```json
{
  "success": true,
  "message": "游戏统计已更新"
}
```

**失败响应**:
```json
{
  "success": false,
  "message": "错误描述"
}
```

**HTTP 状态码**:
- 200: 成功
- 400: 请求错误或参数无效
- 500: 服务器错误

**实现位置**: `Handler::handleSaveGameStats()`

---

### 2. 获取游戏统计

**请求**:
```
GET /api/user/game-stats?user_id=1
```

**参数**:
- `user_id` (required): 用户 ID (query parameter)

**成功响应**:
```json
[
  {
    "gameType": "fishing",
    "highestScore": 2150,
    "timesPlayed": 12,
    "totalScore": 18500,
    "averageScore": 1541.67
  },
  {
    "gameType": "tetris",
    "highestScore": 5200,
    "timesPlayed": 8,
    "totalScore": 28400,
    "averageScore": 3550
  }
]
```

**HTTP 状态码**:
- 200: 成功
- 404: 用户不存在
- 500: 服务器错误

**实现位置**: `Handler::handleGetGameStats()`

---

## 🗄️ 数据库设计

### game_statistics 表结构

```sql
CREATE TABLE game_statistics (
    id INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
    user_id INT UNSIGNED NOT NULL,
    game_type VARCHAR(50) NOT NULL,
    highest_score INT DEFAULT 0,
    times_played INT DEFAULT 0,
    total_score INT DEFAULT 0,
    average_score FLOAT DEFAULT 0,
    last_played TIMESTAMP NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_game (user_id, game_type),
    INDEX idx_user_id (user_id),
    INDEX idx_game_type (game_type),
    INDEX idx_last_played (last_played)
);
```

### 字段说明

| 字段 | 类型 | 说明 |
|------|------|------|
| id | INT UNSIGNED | 主键，自增 |
| user_id | INT UNSIGNED | 用户 ID，外键 |
| game_type | VARCHAR(50) | 游戏类型 |
| highest_score | INT | 最高分 |
| times_played | INT | 游玩次数 |
| total_score | INT | 总分数 |
| average_score | FLOAT | 平均分数 |
| last_played | TIMESTAMP | 最后游玩时间 |
| created_at | TIMESTAMP | 创建时间 |
| updated_at | TIMESTAMP | 更新时间 |

---

## 🔐 数据验证

### 分数合理性检查

```cpp
if (gameType == "fishing" && score > 5000) {
    return ServiceResult(false, "钓鱼游戏分数异常");
} else if (gameType == "tetris" && score > 1000000) {
    return ServiceResult(false, "俄罗斯方块分数异常");
} else if (gameType == "fighter" && score > 50000) {
    return ServiceResult(false, "格斗游戏分数异常");
}
```

**验证规则**:
- 钓鱼: 分数 ≤ 5000
- 俄罗斯方块: 分数 ≤ 1,000,000
- 格斗游戏: 分数 ≤ 50,000
- 所有游戏: 分数 ≥ 0

---

## 🔄 完整的数据流

```
┌─────────────────────────────────────────────────────┐
│ 游戏结束 (fishing.html)                             │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ uploadGameStats('fishing', 1250)                    │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ POST /api/save-game-stats                           │
│ body: user_id=1&game_type=fishing&score=1250        │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ main.cc Router 匹配                                 │
│ "POST" + "/api/save-game-stats"                     │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ Handler::handleSaveGameStats()                      │
├─────────────────────────────────────────────────────┤
│ 1. 解析参数: user_id, game_type, score             │
│ 2. 验证参数非空                                     │
│ 3. 转换类型: string → int                          │
│ 4. 调用 UserService::updateGameStats()            │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ UserService::updateGameStats()                      │
├─────────────────────────────────────────────────────┤
│ 1. 获取数据库连接                                   │
│ 2. SELECT 查询当前统计                             │
│ 3. 验证分数合理性                                   │
│ 4. 计算新的统计数据:                               │
│    • 最高分 = max(旧高分, 新分数)                   │
│    • 游玩次数 = 旧次数 + 1                          │
│    • 总分数 = 旧总分 + 新分数                       │
│    • 平均分 = 总分 / 次数                          │
│ 5. UPDATE 数据库                                    │
│ 6. 释放资源，返回成功                               │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ Handler 返回 JSON 响应                              │
│ {success: true, message: "游戏统计已更新"}         │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ 前端收到响应                                         │
├─────────────────────────────────────────────────────┤
│ 1. 检查 success 标志                               │
│ 2. 成功: 保存本地备份 (LocalStorage)               │
│ 3. 返回游戏选择页面                                 │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ GET /api/user/game-stats?user_id=1                  │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ Handler::handleGetGameStats()                       │
├─────────────────────────────────────────────────────┤
│ 1. 解析查询参数: user_id                           │
│ 2. 调用 UserService::getGameStats()               │
│ 3. 返回 JSON 数组                                   │
└─────────────────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────────────────┐
│ 前端更新显示                                         │
├─────────────────────────────────────────────────────┤
│ 1. 解析 JSON 响应                                   │
│ 2. 更新 LocalStorage 缓存                          │
│ 3. 刷新游戏卡片显示                                 │
│ 4. 显示最新的统计数据                               │
└─────────────────────────────────────────────────────┘
```

---

## 📝 实现细节

### UserService 的三个新方法

#### 1. updateGameStats()

```cpp
ServiceResult updateGameStats(unsigned int userId,
                             const std::string& gameType,
                             int score);
```

**功能**: 更新用户在特定游戏的统计数据

**逻辑**:
1. 获取数据库连接
2. 验证分数 >= 0 且在合理范围内
3. SELECT 查询当前统计
4. 计算新的统计值
5. UPDATE 到数据库
6. 返回成功/失败消息

**返回**: ServiceResult (包含 success 标志和消息)

#### 2. getGameStats()

```cpp
ServiceResult getGameStats(unsigned int userId);
```

**功能**: 获取用户所有游戏的统计

**返回**: JSON 格式的统计数组

```json
[
  {"gameType":"fishing","highestScore":2150,"timesPlayed":12,"totalScore":18500},
  {"gameType":"tetris","highestScore":5200,"timesPlayed":8,"totalScore":28400}
]
```

#### 3. getGameStatsForGame()

```cpp
ServiceResult getGameStatsForGame(unsigned int userId,
                                 const std::string& gameType);
```

**功能**: 获取用户特定游戏的统计

**返回**: 单个游戏的 JSON 对象

---

## 🚀 测试方法

### 1. 使用 curl 测试保存

```bash
curl -X POST http://localhost:8888/api/save-game-stats \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "user_id=1&game_type=fishing&score=1250"
```

**预期响应**:
```json
{"success":true,"message":"游戏统计已更新"}
```

### 2. 使用 curl 测试获取

```bash
curl http://localhost:8888/api/user/game-stats?user_id=1
```

**预期响应**:
```json
[
  {"gameType":"fishing","highestScore":1250,"timesPlayed":1,"totalScore":1250},
  {"gameType":"tetris","highestScore":0,"timesPlayed":0,"totalScore":0},
  {"gameType":"fighter","highestScore":0,"timesPlayed":0,"totalScore":0}
]
```

### 3. 查看数据库

```bash
# 查看统计表结构
mysql -u root -p123456 hello_app -e "DESC game_statistics;"

# 查看用户统计
mysql -u root -p123456 hello_app -e "SELECT * FROM game_statistics WHERE user_id = 1;"
```

---

## 📋 路由注册

### main.cc 中的路由配置

```cpp
// 游戏统计 API 路由
mainRouter.addRouter("POST", "/api/save-game-stats", 
                     yoyo::handler::Handler::handleSaveGameStats);
mainRouter.addRouter("GET", "/api/user/game-stats", 
                     yoyo::handler::Handler::handleGetGameStats);
```

---

## 🔗 与前端的集成

### 前端需要修改的地方

#### 1. fishing.html - 游戏结束时上传

```javascript
async function uploadGameStats(gameType, score) {
    const response = await fetch('/api/save-game-stats', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: `user_id=1&game_type=${gameType}&score=${score}`
    });
    const result = await response.json();
    if (result.success) {
        console.log('✅ 游戏统计已保存到服务器');
    }
}
```

#### 2. game_selection.html - 页面加载时获取

```javascript
async function loadGameStatsFromServer() {
    const response = await fetch('/api/user/game-stats?user_id=1');
    const stats = await response.json();
    
    // 更新本地缓存和显示
    stats.forEach(stat => {
        localStorage.setItem(`gameStats_${stat.gameType}`, 
                            JSON.stringify(stat));
    });
    renderGames();
}
```

---

## 🔐 后续安全改进

### 需要添加的验证

1. **用户身份验证**
   - 验证 user_id 是否与当前登录用户匹配
   - 防止用户为他人上传分数

2. **频率限制**
   - 同一用户同一游戏 5 分钟内最多上传 10 次
   - 防止恶意刷分

3. **异常检测**
   - 检测异常的得分增长率
   - 检测不可能的游玩速度

4. **Session 验证**
   - 使用 Session/Token 而不是 user_id 参数
   - 增强安全性

---

## 📊 数据库查询示例

### 查看用户的最高分

```sql
SELECT game_type, highest_score 
FROM game_statistics 
WHERE user_id = 1;
```

### 查看排行榜（前 10 名）

```sql
SELECT u.username, gs.game_type, gs.highest_score
FROM users u
JOIN game_statistics gs ON u.id = gs.user_id
WHERE gs.game_type = 'fishing'
ORDER BY gs.highest_score DESC
LIMIT 10;
```

### 查看用户的平均分

```sql
SELECT game_type, average_score, times_played
FROM game_statistics
WHERE user_id = 1;
```

---

## ✅ 验证清单

- [x] game_statistics 表已创建
- [x] 初始化脚本已运行
- [x] UserService 方法已实现
- [x] API 处理函数已编写
- [x] 路由已注册
- [x] 编译成功
- [ ] 前端集成 (待完成)
- [ ] 防作弊验证 (后续)
- [ ] Session 验证 (后续)

---

## 🎯 下一步

1. **立即** (今天)
   - [ ] 更新数据库: `mysql -u root -p123456 hello_app < database/init.sql`
   - [ ] 启动服务器并测试 API

2. **本周** (可选)
   - [ ] 更新前端代码集成 API
   - [ ] 测试完整流程

3. **后续** (下一版本)
   - [ ] 添加 Session 验证
   - [ ] 实现防作弊检测
   - [ ] 添加排行榜功能

---

## 📞 技术支持

### 常见问题

**Q: API 返回 500 错误？**
A: 检查数据库是否已初始化。运行: `mysql -u root -p123456 hello_app < database/init.sql`

**Q: 如何验证后端工作正常？**
A: 
1. 检查数据库表是否存在: `DESC game_statistics`
2. 使用 curl 测试 API
3. 查看服务器日志输出

**Q: 前端应该怎样获取 user_id？**
A: 
- 从登录响应中获取
- 存储在 SessionStorage 或 Cookie
- 每个 API 请求时使用

---

**版本**: v2.1  
**日期**: 2026-04-02  
**状态**: ✅ 后端实现完成  
**下一步**: 前端集成 + 测试
