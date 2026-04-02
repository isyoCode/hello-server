#ifndef _yoyo_http_InetAddress_hpp_
#define _yoyo_http_InetAddress_hpp_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

namespace yoyo {

class InetAddress {
 public:
  explicit InetAddress(const std::string& ip, uint16_t port)
      : addrLength_(sizeof(_addr)) {
    bzero(&_addr, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(ip.c_str());
    _addr.sin_port = htons(port);
  }

  explicit InetAddress(const char* ip, uint16_t port)
      : addrLength_(sizeof(_addr)) {
    bzero(&_addr, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(ip);
    _addr.sin_port = htons(port);
  }

  explicit InetAddress(uint16_t port) : addrLength_(sizeof(_addr)) {
    bzero(&_addr, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = htonl(INADDR_ANY);
    _addr.sin_port = htons(port);
  }
  InetAddress() : _addr(), addrLength_(sizeof(_addr)) {}
  InetAddress(const struct sockaddr_in& addr)
      : _addr(addr), addrLength_(sizeof(_addr)) {}
  ~InetAddress() = default;

  const sockaddr_in& getAddr() const { return _addr; }
  socklen_t& getAddrLen() { return addrLength_; }

  uint16_t getPort() const { return ntohs(_addr.sin_port); }

  std::string getIp() const {
      char buf[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf));
      return std::string(buf);
  }

  void setAddr(const struct sockaddr_in& addr) { _addr = addr; }

  std::string writeTostring() const {
    std::string saaddr = std::to_string(_addr.sin_addr.s_addr);
    std::string sProt = std::to_string(_addr.sin_port);
    std::string sTestPort = std::to_string(htons(8888));
    return "addr: " + saaddr + "|sPort: " + sProt + "|testPort:" + sTestPort;
    
  }

 private:
  struct sockaddr_in _addr;
  socklen_t addrLength_;
};

}  // namespace yoyo

#endif