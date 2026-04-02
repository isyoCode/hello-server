#include "InetAddress.hpp"
#include "Socket.h"
#include "../utils/util.h"

namespace yoyo {

Socket::Socket() {
  socketfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ERR_CHECK(socketfd_ < 0, "socket init error");
}

Socket::Socket(int socketfd) : socketfd_(socketfd) {
  ERR_CHECK(socketfd_ < 0, "socket init error");
}

Socket::~Socket() {
  if (socketfd_ < 0) {
    return;
  }
  close(socketfd_);
}

void Socket::bind(const InetAddress& addr) {
  const sockaddr_in& tAddress = addr.getAddr();
  int iRet =
      ::bind(socketfd_, reinterpret_cast<const struct sockaddr*>(&tAddress),
             sizeof(tAddress));
  ERR_CHECK(iRet == -1, "bind error");
}

void Socket::listen() {
  int iRet = ::listen(socketfd_, 20);
  ERR_CHECK(iRet == -1, "listen error");
}

int Socket::accept(InetAddress& addr) {
  sockaddr_in clientAddr;
  socklen_t clientLen = sizeof(clientAddr);
  int clientFd = ::accept(socketfd_, (struct sockaddr*)&clientAddr, &clientLen);
  //TODO 这里注意非阻塞模式 如果循环读第二次读可能会导致抛异常, 所以可以将异常的抛出交给上层业务判定
  /*
    tips: 对于网络库的异常等行为 尽量放在业务层, 以防止底层框架崩溃。
  */
  ERR_CHECK((clientFd == -1 && errno != EAGAIN && errno != EWOULDBLOCK), "accept error");
  addr.setAddr(clientAddr);
  return clientFd;
}

void Socket::setNonBlocking() {
  int flags = fcntl(socketfd_, F_GETFL, 0);
  ERR_CHECK(flags == -1, "fcntl get error");
  int iRet = fcntl(socketfd_, F_SETFL, flags | O_NONBLOCK);
  ERR_CHECK(iRet < 0, "fcntl set error");
}

void Socket::setReuseAddr() {
  int optval = 1;
  int iRet =
      setsockopt(socketfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  ERR_CHECK(iRet == -1, "setReuseAddr error");
}

}  // namespace yoyo