#ifndef __TCP_server_h__
#define __TCP_server_h__

#include "Eventloop.h"
#include "InetAddress.hpp"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "../threadpool/threadpool.h"
#include <future>
#include <memory>
namespace yoyo {

// #define YOYODEBUG

class TcpConnection;

class TcpServer {
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, const std::string&)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
public:
    TcpServer(Eventloop* loop, const InetAddress& listenAddr)
        : loop_(loop),
          acceptor_(loop, listenAddr),
          threadPool_(std::make_unique<ThreadPool>(4)),
          listenAddr_(listenAddr) {
        // 设置 Acceptor 回调
        acceptor_.setNewConnectionCallback(
            [this](int sockfd, const InetAddress& peerAddr) {
                newConnection(sockfd, peerAddr);
            });

        #ifdef YOYODEBUG
            std::cout << "TCPServer init:" << std::endl;
            std::cout << "loop addr: " << loop << std::endl;
            std::cout << listenAddr.writeTostring() << std::endl;
        #endif
    }

    // 设置业务层的回调
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }

    void start();

    template<class Func, class ...Args>
    auto submitProxy(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        return threadPool_->submitTask(std::forward<Func>(func), std::forward<Args>(args)...);
    }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);

    void removeConnection(const TcpConnectionPtr& conn);

    Eventloop* loop_;
    Acceptor acceptor_;
    std::unordered_map<int, TcpConnectionPtr> connections_;

    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;

    std::unique_ptr<ThreadPool> threadPool_; // 线程池，后续可用于多线程版本
    InetAddress listenAddr_;  // 保存监听地址，用于日志输出
}; 

} // end of namespace yoyo
#endif