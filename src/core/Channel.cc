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

void Channel::handleEvent() {
  if (revents_ & EPOLLHUP && !(revents_ & EPOLLIN)) {
    // 对端关闭连接
    if (closeCallback_) closeCallback_();
  }
  if (revents_ & EPOLLERR) {
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (EPOLLIN | EPOLLPRI)) {
    if (readCallback_) readCallback_();
  }
  if (revents_ & EPOLLOUT) {
    if (writeCallback_) writeCallback_();
  }
}

void Channel::update() { epoll_->updateChannel(this); }

void Channel::setRealEvents(uint32_t realEvents) { revents_ = realEvents; }

}  // namespace yoyo
