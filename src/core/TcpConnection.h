
#ifndef __tcp_connection__h__
#define __tcp_connection__h__

#include "Eventloop.h"
#include "Buffer.hpp"
#include <sys/types.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <memory>
namespace yoyo {

class TcpServer;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using MessageCallback   = std::function<void(const TcpConnectionPtr&, const std::string&)>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

    // RAII 文件描述符封装
    struct FileDescriptor {
        int fd;
        FileDescriptor(int f) : fd(f) {}
        ~FileDescriptor() {
            if (fd >= 0) {
                #ifdef YOYODEBUG
                std::cout << "Closing file descriptor: " << fd << std::endl;
                #endif
                ::close(fd);
            }
        }
        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor& operator=(const FileDescriptor&) = delete;
    };

    // 文件发送上下文
    struct FileSendContext {
        std::unique_ptr<FileDescriptor> fileDesc;  // RAII管理的文件描述符
        size_t fileSize;                           // 文件总大小
        off_t offset;                              // 已发送的偏移量
        std::string pendingHeader;                 // 待发送的HTTP头部

        FileSendContext(int fd, size_t size, const std::string& header)
            : fileDesc(std::make_unique<FileDescriptor>(fd)),
              fileSize(size),
              offset(0),
              pendingHeader(header) {}
    };

    TcpConnection(Eventloop* loop, int sockfd, const InetAddress& peerAddr, TcpServer* server)
        : loop_(loop),
          socket_(sockfd),  // 用Socket对象封装fd
          channel_(socket_.getFd(), loop->getEpoll(), loop_->isUsingETMod()),
          peerAddr_(peerAddr),
          server_(server) {

        // *** 如果是 ET 模式，确保连接 fd 非阻塞，防止读写卡死 ***//
        if (loop_->isUsingETMod()) {
            socket_.setNonBlocking();
        }
        channel_.setReadCallback([this]() { handleHttpRead(); });
        channel_.setWriteCallback([this]() { handleWrite(); });
        channel_.setCloseCallback([this]() { handleClose(); });
        channel_.setErrorCallback([this]() { handleError(); });
        channel_.enableReading();
    }

    int fd() const { return socket_.getFd(); }

    // 发送数据（直接写到socket fd）
    void send(const std::string& data) {
        if (loop_->isInLoopThread()) {
            _sendInLoop(data);
        } else {
            // 跨线程调用，通过 runInLoop 在 IO 线程执行
            loop_->runInLoop([self=shared_from_this(), data](){
                self->_sendInLoop(data);
            });
        }
    }

    // 发送文件（使用 sendfile 零拷贝）
    void sendFile(const std::string& headerStr, int fileFd, size_t fileSize) {
        if (loop_->isInLoopThread()) {
            _sendFileInLoop(headerStr, fileFd, fileSize);
        } else {
            loop_->runInLoop([self=shared_from_this(), headerStr, fileFd, fileSize](){
                self->_sendFileInLoop(headerStr, fileFd, fileSize);
            });
        }
    }
    
    void shutdown() {
        if (loop_->isInLoopThread()) {
            handleClose();
        } else {
            loop_->runInLoop([self=shared_from_this()](){
                self->shutdown();
            });
        }
        return;
    }

    void setMessageCallback(MessageCallback cb)        { messageCallback_ = std::move(cb); }
    void setConnectionCallback(ConnectionCallback cb)  { connectionCallback_ = std::move(cb); }
    void setCloseCallback(std::function<void(const TcpConnectionPtr&)> cb) { closeCallback_ = std::move(cb); }

private:
    bool checkCompleteHttpRequest();
    void handleHttpRead();

    void handleRead();
    void handleWrite();

    void handleClose() {
        // 清理文件发送上下文（RAII会自动关闭文件描述符）
        fileSendContext_.reset();
        loop_->removeChannel(&channel_);
        if (closeCallback_) closeCallback_(shared_from_this());
    }

    void handleError() {
        handleClose();
    }

private:
    void _sendInLoop(const std::string& data) {
        if (!(channel_.getEvents() & Channel::kWriteEvent) && OutputBuffer_.readableBytes() == 0) {
            ssize_t n = ::write(socket_.getFd(), data.data(), data.size());
            if (n < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    #ifdef YOYODEBUG
                    std::cerr << "send error on fd=" << socket_.getFd()
                        << " errno=" << errno << " " << strerror(errno) << "\n";
                    #endif
                    handleError();
                }
                n = 0;
            }
            if ((size_t)n < data.size()) {
                OutputBuffer_.append(data.data() + n, data.size() - n);
                channel_.enableWriting();
            }
        } else {
            OutputBuffer_.append(data);
            channel_.enableWriting();
        }
    }

    void _sendFileInLoop(const std::string& headerStr, int fileFd, size_t fileSize);

    void _continueFileSend();

private:
    Eventloop* loop_;
    Socket socket_;      // 用Socket对象替代裸fd
    Channel channel_;
    InetAddress peerAddr_;

    Buffer InputBuffer_;
    Buffer OutputBuffer_;

    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    std::function<void(const TcpConnectionPtr&)> closeCallback_;

    TcpServer *server_; // 反向指向所属的TcpServer，方便访问服务器资源

    // 文件发送上下文（用于异步sendfile）
    std::unique_ptr<FileSendContext> fileSendContext_;
};


} // end of namespace yoyo

#endif
