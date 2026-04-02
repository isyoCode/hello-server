#ifndef _My_epoll_h_
#define _My_epoll_h_
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <vector>

// #include "Channel.h"
// #include "Channel.h"
#include "../utils/util.h"
namespace yoyo {

class Channel;

class Epoll {
 public:
  /**
    struct epoll_event {
        int fd;
        void* ptr;
    };

    将 epoll封装: 包括创建epoll, 添加/修改/删除 事件, 等待事件发生
    并将其与channel进行绑定
  */

  Epoll() : epfd_(-1), evpool_({}) {
    epfd_ = epoll_create1(0);
    ERR_CHECK(epfd_ == -1, "epoll_create1 error");
    evpool_.resize(MAX_EVENTS);
  }
  ~Epoll() {
    if (epfd_ != -1) {
      close(epfd_);
      epfd_ = -1;
    }
  }

  void addChannel(Channel* channel);
  void modifyChannel(Channel*);
  void updateChannel(Channel*);
  void removeChannel(Channel*);
  bool hasChannel(Channel* channel);

  std::vector<Channel*> poll(int timeOut = -1);

 private:
  int epfd_;
  std::vector<struct epoll_event> evpool_;

  std::unordered_map<int, Channel*> channels_;
  inline constexpr static int MAX_EVENTS = 2048;
};

}  // namespace yoyo

#endif  // _My_epoll_h_