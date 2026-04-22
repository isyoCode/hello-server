#include "Channel.h"
#include "../utils/util.h"

namespace yoyo {

Channel::Channel(int fd, Epoll* ep, bool isETMod)
    : fd_(fd), epoll_(ep), events_(0), revents_(0), isETMod_(isETMod) {
  ERR_CHECK(fd_ < 0 || epoll_ == nullptr, "Channel init error");
}

Channel::~Channel() {
  if(epoll_ != nullptr) {
    epoll_->removeChannel(this);
  }
}


int Channel::getFd() const { return fd_; }

uint32_t Channel::getEvents() const { return events_; }

uint32_t Channel::getRealEvents() const { return revents_; }

void Channel::enableReading() {
  // 将当前事件设置为可读事件
  events_ |= kReadEvent;  // 添加读事件
  if(isETMod_) events_ |= EPOLLET;     // 固定加边缘触发
  update();
}

void Channel::enableWriting() {
  events_ |= kWriteEvent;  // 添加写事件
  if(isETMod_) events_ |= EPOLLET;
  // TODO UPDATE;
  update();
}

void Channel::disableReading() {
  events_ &= ~kReadEvent;  // 移除读事件
}
void Channel::disableWriteing() {
  events_ &= ~kWriteEvent;  // 移除写事件
  update();
}

void Channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::handleEvent() {
  if (tied_) {
    // 锁住 weak_ptr，保证 TcpConnection 在整个事件处理期间不被析构
    std::shared_ptr<void> guard = tie_.lock();
    if (guard) {
      handleEventWithGuard();
    }
  } else {
    handleEventWithGuard();
  }
}

void Channel::handleEventWithGuard() {
  uint32_t revents = revents_; // 保存快照，防止回调中修改 revents_ 引发问题
  if (revents & EPOLLHUP && !(revents & EPOLLIN)) {
    if (closeCallback_) closeCallback_();
    return; // 连接已关闭，不再处理其他事件
  }
  if (revents & EPOLLERR) {
    if (errorCallback_) errorCallback_();
    return; // 错误后不继续处理
  }
  if (revents & (EPOLLIN | EPOLLPRI)) {
    if (readCallback_) readCallback_();
  }
  if (revents & EPOLLOUT) {
    if (writeCallback_) writeCallback_();
  }
}

void Channel::update() { epoll_->updateChannel(this); }

void Channel::setRealEvents(uint32_t realEvents) { revents_ = realEvents; }

}  // namespace yoyo
