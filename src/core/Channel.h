#ifndef __yoyo_http_Channel_h__
#define __yoyo_http_Channel_h__

#include <string>
#include <sys/types.h>

#include <cstdint>
#include <functional>

#include "Epoll.h"
#include "Socket.h"
// 将每个链接的socket fd封装成一个Channel对象
namespace yoyo {

class Channel {
 public:
  /**
      将 fd 与关注的事件 及类型 封装成一个对象,
     对不同类型事件提供不同的处理接口(回调方法)
  */

  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void()>;  //读事件

  // 事件常量
  static const int kNoneEvent = 0;
  static const int kReadEvent = EPOLLIN | EPOLLPRI;  // 可读事件
  static const int kWriteEvent = EPOLLOUT;           // 可写事件
  static const int kErrorEvent = EPOLLERR;           // 错误事件

  Channel() : fd_(-1), epoll_(nullptr), events_(0), revents_(0), isETMod_(false) {}

  Channel(int fd, Epoll* ep, bool isETMod = false);
  ~Channel();

  Channel(const Channel&) = delete;
  Channel& operator=(const Channel&) = delete;
  Channel(Channel&&) = default;
  Channel& operator=(Channel&&) = default;

  void enableReading();
  void enableWriting();
  void disableReading();
  void disableWriteing();

  int getFd() const;
  uint32_t getEvents() const;
  uint32_t getRealEvents() const;

  void setRealEvents(uint32_t realEvents);

  void handleEvent();

  void setReadCallback(ReadEventCallback rb) { readCallback_ = std::move(rb); }
  void setWriteCallback(EventCallback wb) { writeCallback_ = std::move(wb); }
  void setErrorCallback(EventCallback eb) { errorCallback_ = std::move(eb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }

  std::string writeTostring() const {
    std::string sFd = std::to_string(fd_);
    std::string logMsg;
    logMsg += "fd:" + sFd;
    logMsg += "|events:" + std::to_string(events_);
    logMsg += "revents:" + std::to_string(revents_);
    return logMsg;
  }
  
 private:
  void update();

 private:
  int fd_;
  Epoll* epoll_;      // 所属的epoll
  uint32_t events_;   // 预期关注事件
  uint32_t revents_;  // 真实接受到的事件

  bool isETMod_;

  // 每个channel需要相关的回调对象
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}  // namespace yoyo

#endif