#include <mutex>
#include <vector>

#include "../utils/util.h"
#include "Epoll.h"
#include "Eventloop.h"

using namespace yoyo;
void Eventloop::loop() {
  while (!quit_) {
    auto activeChannels = epoll_->poll(-1);
    for (auto channel : activeChannels) {
      channel->handleEvent();
#ifdef YOYODEBUG
      std::cout << "Event loop:" << quit_ << std::endl;
      std::cout << "channel info:" << channel->writeTostring() << std::endl;
#endif
    }
    // TODO epoll被唤醒 同时处理等待队列中的任务
    doPendingFunctors();
  }
}

void Eventloop::doPendingFunctors() {
  std::vector<Functor> functors;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
  }
  for (const auto& f : functors) f();
}

void Eventloop::quit() { quit_ = true; }

void Eventloop::updateChannel(yoyo::Channel* ch) {
  epoll_->updateChannel(ch);

#ifdef YOYODEBUG
  std::cout << "updateChannel:" << ch->writeTostring() << std::endl;
#endif
}
void Eventloop::removeChannel(Channel* ch) {
  epoll_->removeChannel(ch);
#ifdef YOYODEBUG
  std::cout << "RemoveChannel:" << ch->writeTostring() << std::endl;
#endif
}
bool Eventloop::hasChannel(Channel* ch) {
  return epoll_->hasChannel(ch);
#ifdef YOYODEBUG
  std::cout << "hasChannel:" << ch->writeTostring() << std::endl;
#endif
}

void Eventloop::handleWakeup() {
  uint64_t one;
  ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
  ERR_CHECK(n != sizeof(one), "wakeup eventloop error!");
}
void Eventloop::wakeup() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  ERR_CHECK(n != sizeof(one), "wakeup eventloop error!");
}

void Eventloop::queueInLoop(std::function<void()> cb) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFunctors_.push_back(std::move(cb));
  }
  // 唤醒 epoll_wait
  wakeup();
}

void Eventloop::runInLoop(std::function<void()> fb) {
  if (isInLoopThread()) {
    fb();
  } else {
    queueInLoop(fb);
  }
}
