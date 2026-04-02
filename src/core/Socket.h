#ifndef _yoyo_http_Socket_h_
#define _yoyo_http_Socket_h_

#include <fcntl.h>

#include "InetAddress.hpp"
namespace yoyo {

/*
    @breif: 结合InetAddress 将sokcet的基本操作封装
*/
class Socket {
 public:
  Socket();
  explicit Socket(int socketfd);
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket(Socket&&) = default;
  Socket& operator=(Socket&&) = default;
  ~Socket();

  void bind(const InetAddress& addr);
  void listen();
  int accept(InetAddress& addr);
  void setReuseAddr();
  void setNonBlocking();

  int getFd() const { return socketfd_; }

 private:
  int socketfd_;
};

}  // namespace yoyo

#endif