# Use-After-Free 修复记录

## 问题背景

在 `TcpConnection::handleHttpRead()` 的多个错误处理分支中，调用 `shutdown()` 后仍然访问了 `this` 的成员变量 `InputBuffer_`，构成 use-after-free。

## 根本原因

`shutdown()` 的调用链如下：

```
shutdown()
  └─ handleClose()
       ├─ loop_->removeChannel(&channel_)      // 从 epoll 移除 Channel
       └─ closeCallback_(shared_from_this())   // 触发 TcpServer::removeConnection
            └─ loop_->runInLoop(lambda)        // 在 IO 线程直接执行（同线程调用）
                 └─ connections_.erase(fd)     // 销毁 map 中的 shared_ptr
```

`shared_from_this()` 创建的临时 `shared_ptr` 在该语句的分号处析构。若此时
`connections_` map 持有的是最后一个引用，则 `TcpConnection` 对象在
`handleClose()` 末尾被销毁。

之后任何通过 `this` 访问成员变量的操作均为 **use-after-free（未定义行为）**，
在高并发压测（如 wrk）下极易触发 coredump。

---

## 受影响位置（均在 `src/core/TcpConnection.cc`）

### 位置 1 — HTTP 请求头解析失败（400 Bad Request）

**原代码（约第 101 行）：**
```cpp
send(response.serialize());
shutdown();
InputBuffer_.retrieve(headerEndPos + 4); // ← USE-AFTER-FREE
return;
```

**触发条件：** `HttpRequestParser::parseHeader()` 返回 false，即收到格式非法的 HTTP 请求头。

**修复：** 删除 `shutdown()` 后的 `InputBuffer_.retrieve()` 调用。

---

### 位置 2 — POST/PUT 缺少 Content-Length（411 Length Required）

**原代码（约第 136 行）：**
```cpp
send(response.serialize());
shutdown();
InputBuffer_.retrieve(headerEndPos + 4); // ← USE-AFTER-FREE
return;
```

**触发条件：** 方法为 POST 或 PUT，但请求头中不含 `Content-Length` 字段。

**修复：** 删除 `shutdown()` 后的 `InputBuffer_.retrieve()` 调用。

---

### 位置 3 — 请求体超过大小限制（413 Payload Too Large）

**原代码（约第 150 行）：**
```cpp
send(response.serialize());
shutdown();
InputBuffer_.retrieve(headerEndPos + 4); // ← USE-AFTER-FREE
return;
```

**触发条件：** `Content-Length` 解析值超过 `MAX_HTTP_BODY_SIZE`（1 MB）。

**修复：** 删除 `shutdown()` 后的 `InputBuffer_.retrieve()` 调用。

---

### 位置 4 — Content-Length 值解析异常（400 Bad Request）

**原代码（约第 162 行）：**
```cpp
send(response.serialize());
shutdown();
InputBuffer_.retrieve(headerEndPos + 4); // ← USE-AFTER-FREE
return;
```

**触发条件：** `Content-Length` 字段值无法被 `std::stoul()` 解析（非数字或溢出），
抛出 `std::exception`。

**修复：** 删除 `shutdown()` 后的 `InputBuffer_.retrieve()` 调用。

---

## 修复说明

上述四处的共同模式是：

```cpp
send(response.serialize());   // 发送错误响应（安全，此时 this 仍有效）
shutdown();                   // 可能销毁 this
InputBuffer_.retrieve(...);   // 访问已销毁对象的成员 ← 删除
return;                       // 函数返回
```

由于这些分支在 `shutdown()` 后均立即 `return`，`InputBuffer_` 本就不需要
更新——连接即将关闭，buffer 状态已无意义。直接删除这一行即可消除 use-after-free，
逻辑上也完全正确。

---

## 关联修复

同次修复还解决了 `Channel::handleEvent()` 中的类似生命周期问题（Bug 3），
详见提交记录及 `src/core/Channel.cc` 中的 `tie()` 机制实现。
