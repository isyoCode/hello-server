#include "TcpServer.h"
#include "../utils/logger.hpp"

namespace yoyo {

void TcpServer::start() {
    // ========== 初始化服务器基础设施 ==========

    LOGI("========================================");
    LOGI("TcpServer Starting...");
    LOGI("Listening on " + listenAddr_.getIp() + ":" + std::to_string(listenAddr_.getPort()));
    LOGI("========================================");

    // 启动线程池
    threadPool_->start();
    LOGI("Thread pool started with 4 worker threads");

    LOGI("TcpServer initialization complete");
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    // 创建 TcpConnection
    auto conn = std::make_shared<TcpConnection>(loop_, sockfd, peerAddr, this);
    conn->setMessageCallback(messageCallback_);
    conn->setConnectionCallback(connectionCallback_);
    conn->setCloseCallback(
        [this](const TcpConnectionPtr& c) {
            removeConnection(c);
        });

    // 保存连接
    connections_[sockfd] = conn;

    // 调用 ConnectionCallback 告诉业务层有新连接
    if (connectionCallback_) connectionCallback_(conn);

    LOGI("New connection from " + peerAddr.getIp() + ":" + std::to_string(peerAddr.getPort()) + " on fd=" + std::to_string(sockfd));

    #ifdef YOYODEBUG
        std::cout << "TcpConnection:" << "有新连接!" << " fd: " << sockfd << std::endl;
    #endif
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop([this, conn]() {
        int fd = conn->fd();
        connections_.erase(fd);
        if (connectionCallback_) connectionCallback_(conn); // 可用于断开通知

        LOGI("Connection closed on fd=" + std::to_string(fd) + " (active connections: " + std::to_string(connections_.size()) + ")");

        #ifdef YOYODEBUG
            std::cout << "TcpServer::removeConnection - Removing connection on fd " << fd << std::endl;
        #endif

        // 注释掉自动退出逻辑，HTTP服务器应该持续运行
        // if (connections_.empty()) {
        //     #ifdef YOYODEBUG
        //         std::cout << "All clients disconnected. Shutting down server EventLoop.\n";
        //     #endif
        //     loop_->quit(); // 告诉 EventLoop 停止循环
        // }
    });
}

} // end of namespace yoyo
