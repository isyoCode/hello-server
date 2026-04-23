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

## 关联修复：Channel::handleEvent() 生命周期问题

### 问题描述

在高并发场景下，`Channel::handleEvent()` 执行回调（如 `readCallback_`、`closeCallback_`）时，
回调内部可能触发 `TcpConnection` 的销毁（例如调用 `shutdown()` → `handleClose()` →
`connections_.erase(fd)`）。一旦最后一个 `shared_ptr` 被释放，`TcpConnection` 对象析构，
而此时 `Channel::handleEvent()` 仍在该对象的栈帧上执行，后续的任何成员访问均为
**use-after-free（未定义行为）**。

### 触发路径

```
Epoll::wait() 返回事件
  └─ Channel::handleEvent()
       └─ readCallback_()              // 即 TcpConnection::handleRead()
            └─ handleHttpRead()
                 └─ shutdown()
                      └─ handleClose()
                           └─ closeCallback_(shared_from_this())
                                └─ connections_.erase(fd)  // 最后一个 shared_ptr 释放
                                     └─ ~TcpConnection()   // 对象已析构
  ← handleEvent() 继续执行，this 已悬空 → UB / coredump
```

原始 `handleEvent()` 在触发 `closeCallback_` 后没有 `return`，仍会继续检查
`EPOLLIN`、`EPOLLOUT` 事件并调用相应回调，此时 `this`（即 `Channel` 所属的
`TcpConnection`）可能已被销毁。

### 修复方案：`Channel::tie()` + `weak_ptr` 守卫

**核心思路：** `TcpServer` 创建 `TcpConnection` 后，立即调用
`conn->connectEstablished()`，后者将 `shared_from_this()` 通过 `Channel::tie()`
保存为 `weak_ptr<void>`。`handleEvent()` 执行时先将 `weak_ptr` 提升为
`shared_ptr`（`guard`），只要 `guard` 存活，`TcpConnection` 就不会析构。

**新增接口（`Channel.h` / `Channel.cc`）：**

```cpp
// Channel.h
std::weak_ptr<void> tie_;
bool tied_ = false;

void tie(const std::shared_ptr<void>& obj);  // 绑定 owner 的 shared_ptr
void handleEventWithGuard();                  // 实际事件分发逻辑
```

```cpp
// Channel.cc
void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::handleEvent() {
    if (tied_) {
        std::shared_ptr<void> guard = tie_.lock();  // 提升引用计数
        if (guard) {
            handleEventWithGuard();                 // guard 存活期间不会析构
        }
    } else {
        handleEventWithGuard();
    }
}
```

**`handleEventWithGuard()` 附加修复：**

原 `handleEvent()` 存在事件处理顺序问题：`EPOLLHUP` 触发 `closeCallback_()` 后
没有 `return`，会继续执行后续的 `EPOLLIN`/`EPOLLOUT` 分支。新版在每个终止性事件
（`EPOLLHUP`、`EPOLLERR`）后加 `return`，避免对已关闭连接做多余处理：

```cpp
void Channel::handleEventWithGuard() {
    uint32_t revents = revents_;  // 快照，防止回调中修改 revents_ 引发竞态
    if (revents & EPOLLHUP && !(revents & EPOLLIN)) {
        if (closeCallback_) closeCallback_();
        return;  // 连接已关闭，不再处理其他事件
    }
    if (revents & EPOLLERR) {
        if (errorCallback_) errorCallback_();
        return;  // 错误后不继续处理
    }
    if (revents & (EPOLLIN | EPOLLPRI)) {
        if (readCallback_) readCallback_();
    }
    if (revents & EPOLLOUT) {
        if (writeCallback_) writeCallback_();
    }
}
```

**调用点（`TcpConnection.h` / `TcpServer.cc`）：**

```cpp
// TcpConnection.h — 必须在 shared_ptr 已构造后调用，否则 shared_from_this() 会抛异常
void connectEstablished() {
    channel_.tie(shared_from_this());
}

// TcpServer.cc
connections_[sockfd] = conn;
conn->connectEstablished();  // 先存入 map，再绑定 tie，保证引用计数 > 0
```

> **注意：** `connectEstablished()` 必须在 `connections_[sockfd] = conn` 之后调用。
> 若在存入 map 之前调用 `shared_from_this()`，此时对象可能尚无外部 `shared_ptr`
> 持有，`enable_shared_from_this` 内部的 `weak_ptr` 为空，导致抛出
> `std::bad_weak_ptr` 异常——这正是提交信息"修改高并发环境下的 shared_ptr 构造失败问题"
> 所指的根本原因。

### 修复后生命周期保证

```
handleEvent()
  ├─ guard = tie_.lock()       // 引用计数 +1，TcpConnection 不会析构
  └─ handleEventWithGuard()
       └─ readCallback_()      // 即使内部触发 connections_.erase(fd)
                               // guard 仍持有引用，对象安全存活至此函数返回
  guard 析构                   // 引用计数 -1，若已无其他持有者，此处才真正析构
```
