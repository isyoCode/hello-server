# 浏览器请求示例完整指南

快速参考：如何在浏览器中访问 YoyoCppServer 的各种资源。

---

## 📋 快速参考表

| 资源类型 | 请求方式 | URL | 浏览器直接访问 |
|---------|---------|-----|---------------|
| **首页** | GET | `/` | ✅ |
| **主页** | GET | `/index.html` | ✅ |
| **登录页** | GET | `/login` | ✅ |
| **注册页** | GET | `/register` | ✅ |
| **仪表板** | GET | `/dashboard` | ✅ |
| **游戏1** | GET | `/game1.html` | ✅ |
| **游戏2** | GET | `/game2.html` | ✅ |
| **认证页** | GET | `/auth.html` | ✅ |
| **样式表** | GET | `/css/style.css` | ✅ |
| **图片** | GET | `/images/1.png` | ✅ |
| **用户注册** | POST | `/register` | ❌ (需要工具) |
| **用户登录** | POST | `/login` | ❌ (需要工具) |
| **游戏统计** | GET | `/api/user/game-stats` | ✅ |
| **保存统计** | POST | `/api/save-game-stats` | ❌ (需要工具) |

---

## 🌐 第1类：HTML 页面（直接在浏览器访问）

### 最简单的方式 - 在浏览器地址栏输入

```
http://localhost:8888/
http://localhost:8888/index.html
http://localhost:8888/login
http://localhost:8888/register
http://localhost:8888/dashboard
```

### 详细列表

#### 1. 首页
```
http://localhost:8888/
```
直接在浏览器地址栏输入并按 Enter

#### 2. 登录页面
```
http://localhost:8888/login
```

#### 3. 注册页面
```
http://localhost:8888/register
```

#### 4. 用户仪表板
```
http://localhost:8888/dashboard
```

#### 5. 游戏页面
```
http://localhost:8888/game1.html      # 俄罗斯方块
http://localhost:8888/game2.html      # 格斗竞技
http://localhost:8888/auth.html       # 认证页面
```

---

## 🎨 第2类：静态资源（CSS、图片等）

### 样式表
```
http://localhost:8888/css/style.css
```

### 图片资源
```
http://localhost:8888/images/1.png
http://localhost:8888/images/3.jpg
http://localhost:8888/images/scenery1.jpg
http://localhost:8888/images/scenery2.jpg
http://localhost:8888/images/scenery3.jpg
http://localhost:8888/images/scenery4.jpg
http://localhost:8888/images/background.jpg
http://localhost:8888/images/someone.jpg
```

---

## 📤 第3类：API 请求（需要工具）

### 方式 A：使用浏览器开发者工具 (F12)

#### 步骤 1：打开浏览器开发者工具
按键盘上的 `F12` 或右键点击页面 → 选择 "检查"

#### 步骤 2：切换到 Console 标签页
在开发者工具窗口中选择 **Console** 标签

#### 步骤 3：粘贴并执行代码

#### 获取游戏统计 (GET)

**直接浏览器访问**：
```
http://localhost:8888/api/user/game-stats
```

**在 Console 中获取 JSON 数据**：
```javascript
fetch('http://localhost:8888/api/user/game-stats')
  .then(response => response.json())
  .then(data => console.log(data))
  .catch(error => console.error('Error:', error));
```

**更详细的输出**：
```javascript
fetch('http://localhost:8888/api/user/game-stats')
  .then(response => {
    console.log('Status:', response.status);
    return response.json();
  })
  .then(data => {
    console.log('Games data:', data);
    console.log('Full data:', JSON.stringify(data, null, 2));
  });
```

---

#### 用户注册 (POST)

**在 Console 中运行**：
```javascript
fetch('http://localhost:8888/register', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({
    username: 'alice',
    email: 'alice@example.com',
    password: 'Password123'
  })
})
.then(response => response.json())
.then(data => console.log('注册结果:', data))
.catch(error => console.error('错误:', error));
```

**自定义用户名和密码**：
```javascript
fetch('http://localhost:8888/register', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({
    username: 'myusername',
    email: 'myemail@example.com',
    password: 'MyPassword123'
  })
})
.then(r => r.json())
.then(d => console.log(d));
```

---

#### 用户登录 (POST)

**在 Console 中运行**：
```javascript
fetch('http://localhost:8888/login', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({
    username: 'alice',
    password: 'Password123'
  })
})
.then(response => response.json())
.then(data => console.log('登录结果:', data))
.catch(error => console.error('错误:', error));
```

---

#### 保存游戏统计 (POST)

**在 Console 中运行**：
```javascript
fetch('http://localhost:8888/api/save-game-stats', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({
    userId: 1,
    gameType: 'tetris',
    score: 12345
  })
})
.then(response => response.json())
.then(data => console.log('保存结果:', data))
.catch(error => console.error('错误:', error));
```

**不同的游戏类型**：
```javascript
// 俄罗斯方块
fetch('http://localhost:8888/api/save-game-stats', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({userId: 1, gameType: 'tetris', score: 50000})
}).then(r => r.json()).then(d => console.log(d));

// 格斗游戏
fetch('http://localhost:8888/api/save-game-stats', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({userId: 1, gameType: 'fighter', score: 35000})
}).then(r => r.json()).then(d => console.log(d));

// 打鱼游戏
fetch('http://localhost:8888/api/save-game-stats', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({userId: 1, gameType: 'fishing', score: 4500})
}).then(r => r.json()).then(d => console.log(d));
```

---

### 方式 B：使用 curl 命令行工具

#### 获取游戏统计 (GET)
```bash
curl http://localhost:8888/api/user/game-stats
```

**格式化输出**：
```bash
curl -s http://localhost:8888/api/user/game-stats | json_pp
```

#### 用户注册 (POST)
```bash
curl -X POST http://localhost:8888/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "alice",
    "email": "alice@example.com",
    "password": "Password123"
  }'
```

#### 用户登录 (POST)
```bash
curl -X POST http://localhost:8888/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "alice",
    "password": "Password123"
  }'
```

#### 保存游戏统计 (POST)
```bash
curl -X POST http://localhost:8888/api/save-game-stats \
  -H "Content-Type: application/json" \
  -d '{
    "userId": 1,
    "gameType": "tetris",
    "score": 12345
  }'
```

---

### 方式 C：使用 Postman 等 API 工具

**Postman 设置**：

1. 打开 Postman
2. 创建新请求
3. 选择请求方法 (GET/POST)
4. 输入 URL: `http://localhost:8888/...`
5. 设置 Headers (如需要): `Content-Type: application/json`
6. 设置 Body (如需要): JSON 格式的数据
7. 点击 Send

**示例配置**：

| 字段 | 值 |
|------|-----|
| Method | POST |
| URL | http://localhost:8888/register |
| Headers | Content-Type: application/json |
| Body | {"username":"alice","email":"alice@example.com","password":"Password123"} |

---

## 📝 完整使用流程示例

### 流程 1：注册新用户并查看统计

```javascript
// 步骤 1: 注册用户
fetch('http://localhost:8888/register', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({
    username: 'newuser',
    email: 'new@example.com',
    password: 'NewPass123'
  })
})
.then(r => r.json())
.then(data => {
  console.log('注册结果:', data);
  
  // 步骤 2: 登录用户
  return fetch('http://localhost:8888/login', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({
      username: 'newuser',
      password: 'NewPass123'
    })
  });
})
.then(r => r.json())
.then(data => {
  console.log('登录结果:', data);
  
  // 步骤 3: 获取游戏统计
  return fetch('http://localhost:8888/api/user/game-stats');
})
.then(r => r.json())
.then(data => console.log('游戏统计:', data));
```

### 流程 2：保存游戏分数并查看

```javascript
// 保存多个游戏的分数
const games = [
  {gameType: 'tetris', score: 50000},
  {gameType: 'fighter', score: 35000},
  {gameType: 'fishing', score: 4500}
];

let chain = Promise.resolve();

games.forEach(game => {
  chain = chain.then(() => 
    fetch('http://localhost:8888/api/save-game-stats', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({
        userId: 1,
        gameType: game.gameType,
        score: game.score
      })
    })
    .then(r => r.json())
    .then(d => console.log(`保存 ${game.gameType}: ${d.message}`))
  );
});

// 最后查看所有统计
chain.then(() => 
  fetch('http://localhost:8888/api/user/game-stats')
    .then(r => r.json())
    .then(d => console.log('最终统计:', d))
);
```

---

## 🧪 测试脚本

### 创建自动测试脚本

**文件名**: `test_api.sh`

```bash
#!/bin/bash

SERVER="http://localhost:8888"
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== YoyoCppServer API 测试脚本 ===${NC}\n"

# 测试 1: 获取首页
echo -e "${GREEN}测试 1: 获取首页${NC}"
curl -s "$SERVER/" | head -5
echo -e "\n"

# 测试 2: 访问登录页
echo -e "${GREEN}测试 2: 访问登录页${NC}"
curl -s "$SERVER/login" | head -5
echo -e "\n"

# 测试 3: 用户注册
echo -e "${GREEN}测试 3: 用户注册${NC}"
curl -X POST "$SERVER/register" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser_'$(date +%s)'",
    "email": "test_'$(date +%s)'@example.com",
    "password": "TestPass123"
  }' | json_pp
echo -e "\n"

# 测试 4: 用户登录
echo -e "${GREEN}测试 4: 用户登录${NC}"
curl -X POST "$SERVER/login" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "alice",
    "password": "Password123"
  }' | json_pp
echo -e "\n"

# 测试 5: 获取游戏统计
echo -e "${GREEN}测试 5: 获取游戏统计${NC}"
curl -s "$SERVER/api/user/game-stats" | json_pp
echo -e "\n"

# 测试 6: 保存游戏统计
echo -e "${GREEN}测试 6: 保存游戏统计${NC}"
curl -X POST "$SERVER/api/save-game-stats" \
  -H "Content-Type: application/json" \
  -d '{
    "userId": 1,
    "gameType": "tetris",
    "score": 99999
  }' | json_pp

echo -e "${BLUE}=== 测试完成 ===${NC}"
```

**运行**：
```bash
chmod +x test_api.sh
./test_api.sh
```

---

## 🔍 调试技巧

### 在浏览器 Console 中查看详细信息

```javascript
// 1. 查看完整响应和状态
fetch('http://localhost:8888/api/user/game-stats')
  .then(response => {
    console.log('状态码:', response.status);
    console.log('响应头:', response.headers);
    console.log('完整 URL:', response.url);
    return response.json();
  })
  .then(data => {
    console.log('响应体:', data);
    console.log('JSON 格式:', JSON.stringify(data, null, 2));
  })
  .catch(error => {
    console.error('完整错误:', error);
    console.error('错误信息:', error.message);
    console.error('错误堆栈:', error.stack);
  });
```

### 测试网络连接

```javascript
// 测试服务器是否响应
fetch('http://localhost:8888/')
  .then(r => {
    console.log('✅ 服务器在线，状态码:', r.status);
  })
  .catch(e => {
    console.error('❌ 无法连接到服务器:', e.message);
  });
```

---

## 📊 请求和响应示例

### 注册请求

**请求**:
```json
POST /register HTTP/1.1
Host: localhost:8888
Content-Type: application/json

{
  "username": "alice",
  "email": "alice@example.com",
  "password": "Password123"
}
```

**响应**:
```json
{
  "success": true,
  "message": "User registered successfully"
}
```

### 登录请求

**请求**:
```json
POST /login HTTP/1.1
Host: localhost:8888
Content-Type: application/json

{
  "username": "alice",
  "password": "Password123"
}
```

**响应**:
```json
{
  "success": true,
  "message": "Authentication successful",
  "user": {
    "id": 1,
    "username": "alice",
    "email": "alice@example.com"
  }
}
```

### 游戏统计请求

**请求**:
```
GET /api/user/game-stats HTTP/1.1
Host: localhost:8888
```

**响应**:
```json
[
  {
    "gameType": "tetris",
    "highestScore": 50000,
    "timesPlayed": 5,
    "totalScore": 150000,
    "averageScore": 30000
  }
]
```

---

## ✅ 常见问题

**Q: 在 Console 中粘贴代码后什么都没发生？**
A: 按 Enter 执行代码。结果会在 Console 中显示。

**Q: 如何查看请求/响应的详细信息？**
A: 在 DevTools 中切换到 Network 标签页，可以看到完整的请求和响应。

**Q: POST 请求总是失败？**
A: 确保设置了正确的 Content-Type header: `'Content-Type': 'application/json'`

**Q: 如何批量测试多个端点？**
A: 使用提供的 `test_api.sh` 脚本，或在 Console 中使用循环和 Promise 链。

---

## 📚 相关文档

- [README.md](../README.md) - 项目主文档
- [API_REFERENCE.md](./API_REFERENCE.md) - API 完整参考
- [QUICK_START.md](./QUICK_START.md) - 快速开始指南

---

**更新时间**: 2026-04-02
