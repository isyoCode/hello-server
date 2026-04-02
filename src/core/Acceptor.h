#ifndef __yoyo__acceptor_h__
#define __yoyo__acceptor_h__
#include <functional>

#include "Channel.h"
#include "Eventloop.h"
#include "InetAddress.hpp"
#include "Socket.h"

namespace yoyo {

class Acceptor {
 public:
  using NewConnectionCallback = std::function<void(int, const InetAddress&)>;

  Acceptor(Eventloop* loop, const InetAddress& listenAddr)
      : loop_(loop),
        acceptSocket_(::socket(AF_INET, SOCK_STREAM, 0)),
        acceptChannel_(acceptSocket_.getFd(), loop->getEpoll(), loop_->isUsingETMod()) {
    acceptSocket_.setReuseAddr();
    if(loop_->isUsingETMod()) acceptSocket_.setNonBlocking();
    acceptSocket_.bind(listenAddr);
    acceptSocket_.listen();

    // 设置监听 Channel 回调
    acceptChannel_.setReadCallback([this]() { handleRead(); });
    acceptChannel_.enableReading();

    #ifdef YOYODEBUG
          std::cout << "Acceptor init:" << std::endl;
          std::cout << "acceptFd:" << acceptSocket_.getFd() << std::endl;
    #endif

  }

  void setNewConnectionCallback(NewConnectionCallback cb) {
    newConnectionCallback_ = std::move(cb);
  }

 private:
  void handleRead() {
      do {
        InetAddress peer;
        int connfd = acceptSocket_.accept(peer);
        if (connfd >= 0) {
            if (newConnectionCallback_) newConnectionCallback_(connfd, peer);
            #ifdef YOYODEBUG
            std::cout << "Acceptor accept:" << std::endl;
            std::cout << "accept connectfd:" << connfd << std::endl;
            #endif
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            if (errno == EINTR) continue;
            break; // 其它错误可打印日志
        }
      } while(loop_->isUsingETMod());

  }

  Eventloop* loop_;        // 归属的eventloop
  Socket acceptSocket_;    // 作为监听的socket
  Channel acceptChannel_;  // 用来封装监听的fd
  NewConnectionCallback
      newConnectionCallback_;  // 作为每次新的客户端链接建立时 调用的方法
};

}  // end of namespace yoyo

#endif