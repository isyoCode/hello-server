#ifndef __yoyo__eveent_loop_h__
#define __yoyo__eveent_loop_h__
#include <sys/eventfd.h>

#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "Channel.h"
#include "Epoll.h"
#include "../utils/util.h"

namespace yoyo {

class Eventloop {
  /**
      eventloop用来管理Epoll对象
  */
 public:
  explicit Eventloop(bool isETMod = false)
      : epoll_(std::make_unique<yoyo::Epoll>()),
        quit_(false),
        isETMod_(isETMod),
        threadId_(std::this_thread::get_id()) {
    wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    ERR_CHECK(wakeupFd_ < 0, "Failed to create eventfd");
    // 创建 wakeupChannel
    wakeupChannel_ = std::make_unique<yoyo::Channel>(wakeupFd_, epoll_.get(), isETMod_);
    wakeupChannel_->setReadCallback([this]() { handleWakeup(); });
    wakeupChannel_->enableReading();
  }

  /**
      核心方: 启动事件循环 -> loop
             修改循环标志 -> setQuit

          // 处理epoll中的实际每个事件

  */

  void loop();
  void quit();
  void runInLoop(std::function<void()> fb);

  void updateChannel(yoyo::Channel*);
  void removeChannel(Channel*);
  bool hasChannel(Channel*);
  Epoll* getEpoll() const { return epoll_.get(); }
  bool isUsingETMod() const {
    return isETMod_;
  }


  void queueInLoop(std::function<void()> fb);
  bool isInLoopThread() const {
    return std::this_thread::get_id() == threadId_;
  }
 private:
  using Functor = std::function<void()>;
  void doPendingFunctors();
  void handleWakeup();
  void wakeup();

  

  std::unique_ptr<yoyo::Epoll> epoll_;
  bool quit_;
  bool isETMod_;
  std::thread::id threadId_;
  std::vector<Functor> pendingFunctors_;
  mutable std::mutex mutex_;

  // TODO 用于跨线程任务的通知方法处理
  int wakeupFd_;
  std::unique_ptr<yoyo::Channel> wakeupChannel_;
};
}  // namespace yoyo

#endif
