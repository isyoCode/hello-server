#include "Channel.h"

using namespace yoyo;
void Epoll::addChannel(Channel* channel) {
  struct epoll_event ev;
  bzero(&ev, sizeof(ev));
  ev.data.ptr = channel;
  ev.events = channel->getEvents();
  int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, channel->getFd(), &ev);
  ERR_CHECK(ret == -1, "epoll_ctl(EPOLL_CTL_ADD) error");
  channels_[channel->getFd()] = channel;
}

void Epoll::modifyChannel(Channel* channel) {
  struct epoll_event ev;
  bzero(&ev, sizeof(ev));
  ev.data.ptr = channel;
  ev.events = channel->getEvents();
  int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, channel->getFd(), &ev);
  ERR_CHECK(ret == -1, "epoll_ctl(EPOLL_CTL_MOD) error");
  channels_[channel->getFd()] = channel;
}

void Epoll::removeChannel(Channel* channel) {
  auto it = channels_.find(channel->getFd());
  if(it == channels_.end()) {
    return;
  }
  struct epoll_event ev;
  bzero(&ev, sizeof(ev));
  ev.data.ptr = channel;
  ev.events = channel->getEvents();
  int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, channel->getFd(), &ev);
  if(ret == - 1) {
    if (errno == EBADF || errno == ENOENT) {
        std::cout << "[inremove] fd已经被析构." << std::endl;
    } else {
        perror("epoll_ctl DEL error");
        ERR_CHECK(ret == -1, "epoll_ctl(EPOLL_CTL_DEL) error");
        return; // 或者记录日志
    }
  }
  channels_.erase(channel->getFd());
}

std::vector<Channel*> Epoll::poll(int timeOut) {
  int nfds = epoll_wait(epfd_, evpool_.data(), MAX_EVENTS, timeOut);
  ERR_CHECK(nfds == -1, "epoll_wait error");
  std::vector<Channel*> activeEvents;
  activeEvents.reserve(nfds);
  for (int i = 0; i < nfds; i++) {
    Channel* ch = static_cast<Channel*>(evpool_[i].data.ptr);
    ch->setRealEvents(evpool_[i].events);
    activeEvents.push_back(ch);
  }
  return activeEvents;
}

bool Epoll::hasChannel(Channel* channel) {
  return channels_.find(channel->getFd()) != channels_.end();
}
void Epoll::updateChannel(Channel* channel) {
  // 将channel注册到epoll中
  if (!hasChannel(channel)) {
    addChannel(channel);  // 或者单独处理
  } else {
    modifyChannel(channel);  // EPOLL_CTL_MOD
  }
}
