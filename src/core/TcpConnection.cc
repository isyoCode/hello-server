#include "TcpConnection.h"
#include "TcpServer.h"
#include <cstddef>
#include <string_view>

#include "../http/request_parser.h"
#include "../http/response.h"
#include "../utils/logger.hpp"

namespace yoyo {

// http请求的相关配置参数
constexpr size_t MAX_HTTP_HEADER_SIZE = 8 * 1024; // 8KB
constexpr size_t MAX_HTTP_BODY_SIZE = 1 * 1024 * 1024; // 1MB 
constexpr size_t MAX_HTTP_REQUEST_SIZE = MAX_HTTP_HEADER_SIZE + MAX_HTTP_BODY_SIZE;


bool TcpConnection::checkCompleteHttpRequest() {
    while(InputBuffer_.readableBytes() > 0) {
        const char* bufferStart = InputBuffer_.peek();
        size_t bufferSize = InputBuffer_.readableBytes();
        std::string_view buffView(bufferStart, bufferSize);

        // 查找请求头的结束位置
        size_t headerEndPos = buffView.find("\r\n\r\n");

        if(headerEndPos != std::string_view::npos) {
            // 请求头完整，继续检查请求体
            size_t contentLengthPos = buffView.find("Content-Length:");
            if(contentLengthPos != std::string_view::npos) {
                // 找到Content-Length，解析请求体长度
                size_t contentLengthStart = contentLengthPos + std::string_view("Content-Length:").size();
                size_t contentLengthEnd = buffView.find("\r\n", contentLengthStart);
                if(contentLengthEnd != std::string_view::npos) {
                    std::string_view contentLengthStr = buffView.substr(contentLengthStart, contentLengthEnd - contentLengthStart);
                    size_t contentLength = std::stoul(std::string(contentLengthStr));
                    size_t totalRequestSize = headerEndPos + 4 + contentLength; // 4是\r\n\r\n的长度
                    if(bufferSize >= totalRequestSize) {
                        return true; // 请求完整
                    } else {
                        return false; // 请求体不完整
                    }
                } else {
                    return false; // Content-Length行不完整
                }
            } else {
                return true; // 没有请求体，头部完整即请求完整
            }
        } else {
            return false; // 请求头不完整
        }
    }
    return true; // 没有数据，认为请求完整（可能是空请求）
}
 


/**
    对于http请求, 我们不需要一次性读取所有数据到内存中, 因为http请求可能很大, 甚至是分块传输的.
    因此, 我们在handleHttpRead中, 每次读取一部分数据到InputBuffer_, 然后提交一个任务到线程池, 让线程池去处理这部分数据.
    最后在业务侧的回调函数中, 业务逻辑可以根据需要, 从InputBuffer_中读取数据进行处理, 也可以选择不读取, 等到下一次handleHttpRead被调用时再读取.
*/
void TcpConnection::handleHttpRead() {
     #ifdef YOYODEBUG
        std::cout << "TcpConnection handleHttpRead()" << std::endl;
    #endif
    while(true) {
        int savedErrno = 0;
        ssize_t n = InputBuffer_.readFd(socket_.getFd(), &savedErrno);
        if(n > 0) {
            while(InputBuffer_.readableBytes() > 0) { // 流式协议需要逐步检查
                const char* bufferStart = InputBuffer_.peek();
                size_t bufferSize = InputBuffer_.readableBytes();
                std::string_view buffView(bufferStart, bufferSize);

                // 查找请求头的结束位置
                size_t headerEndPos = buffView.find("\r\n\r\n");
                if(headerEndPos == std::string_view::npos) {
                    if(bufferSize > MAX_HTTP_HEADER_SIZE) {
                        // 请求头过大，直接关闭连接
                        std::cout << "Http header too large, closing connection." << std::endl;
                        shutdown();
                        return;
                    }
                    break; // 请求头不完整，等待下一次读取
                }

                std::string headerOnly(buffView.substr(0, headerEndPos + 4)); // 包含\r\n\r\n
                #ifdef YOYODEBUG
                    std::cout << "Received HTTP header:\n" << headerOnly << std::endl;
                #endif

                yoyo::http::HttpRequestParser tempHeaderparser;
                if(!tempHeaderparser.parseHeader(headerOnly)) {
                    // 请求头解析失败，直接关闭连接
                    std::cout << "Http header parse error, closing connection." << std::endl;
                    yoyo::http::HttpResponse response;
                    response.setVersion("HTTP/1.1");
                    response.setStatusCode(400);
                    response.setReason("Bad Request");
                    response.addHeader("Connection", "close");
                    send(response.serialize());
                    shutdown();
                    InputBuffer_.retrieve(headerEndPos + 4); // 移除已处理的请求头
                    return;
                }

                /**
                    http请求中 POST / PUT 方法必须携带 Content-Length 或者 transfer-encoding: chunked 来指明请求体的长度,
                    否则我们无法知道请求体何时结束.
                    对于 GET/HEAD/DELETE/OPTIONS 等没有消息体的方法，contentLength 保持为 0
                */
                size_t contentLength = 0;
                const std::string& method = tempHeaderparser.getRequse().getMethod();
                bool expectBody = (method == "POST" || method == "PUT");

                // 检查 Content-Length 头部 
                std::string cl_value; 
                bool has_content_length_header = false;
                const auto& headers = tempHeaderparser.getRequse().getheader();
                for (const auto& pair : headers) { 
                    if (strcasecmp(pair.first.c_str(), "content-length") == 0) {
                        cl_value = pair.second;
                        has_content_length_header = true;
                        break;
                    }
                }

                if(expectBody) {
                    if(!has_content_length_header) {
                        // 需要请求体但没有 Content-Length，直接关闭连接
                        std::cout << "Content-Length header missing for POST/PUT request, closing connection." << std::endl;
                        yoyo::http::HttpResponse response;
                        response.setVersion("HTTP/1.1");
                        response.setStatusCode(411);
                        response.setReason("Length Required");
                        response.addHeader("Connection", "close");
                        send(response.serialize());
                        shutdown();
                        InputBuffer_.retrieve(headerEndPos + 4); // 移除已处理的请求头
                        return;
                    }
                    try {
                        contentLength = std::stoul(cl_value);
                        if(contentLength > MAX_HTTP_BODY_SIZE) {
                            // 请求体过大，直接关闭连接
                            std::cout << "Http body too large, closing connection." << std::endl;
                            yoyo::http::HttpResponse response;
                            response.setVersion("HTTP/1.1");
                            response.setStatusCode(413);
                            response.setReason("Payload Too Large");
                            response.addHeader("Connection", "close");
                            send(response.serialize());
                            shutdown();
                            InputBuffer_.retrieve(headerEndPos + 4); // 移除已处理的请求头
                            return;
                        }
                    } catch(const std::exception& e) {
                        // Content-Length 解析失败，直接关闭连接
                        std::cout << "Content-Length parse error: " << e.what() << ", closing connection." << std::endl;
                        yoyo::http::HttpResponse response;
                        response.setVersion("HTTP/1.1");
                        response.setStatusCode(400);
                        response.setReason("Bad Request");
                        response.addHeader("Connection", "close");
                        send(response.serialize());
                        shutdown();
                        InputBuffer_.retrieve(headerEndPos + 4); // 移除已处理的请求头
                        return;
                    }
                }

                // 单个的http解析完成，提交任务到线程池处理
                size_t totalRequestSize = headerEndPos + 4 + contentLength; // 4是\r\n\r\n的长度
                if(bufferSize >= totalRequestSize) {
                    std::string requestData(bufferStart, totalRequestSize);
                    server_->submitProxy([self=shared_from_this(), msg=std::move(requestData)](){
                        self->messageCallback_(self, msg);
                    });
                    InputBuffer_.retrieve(totalRequestSize); // 移除已处理的请求数据
                } else {
                    break; // 请求体不完整，等待下一次读取
                }

            }
            #ifdef YOYODEBUG
                std::cout << "TcpConnection handleRead string size:" << n << std::endl;
            #endif
            if (!loop_->isUsingETMod()) break; // LT 模式直接退出
            else continue; // ET 模式继续读
        } else if (n == 0) {
            handleClose();
            #ifdef YOYODEBUG
                std::cout << "TcpConnection handleRead() peer closed connection." << std::endl;
            #endif
            break;
        } else { // n < 0
            if (savedErrno == EAGAIN || savedErrno == EWOULDBLOCK) break; // 没数据了
            if (savedErrno == EINTR) continue; // 被信号打断，继续尝试
            #ifdef YOYODEBUG
                std::cout << "TcpConnection handleRead() error: " << strerror(savedErrno) << std::endl;
            #endif
            handleError();
            break;
        }
    }
    return;
}


void TcpConnection::handleRead() {
        #ifdef YOYODEBUG
            std::cout << "TcpConnection hanndleRead()" << std::endl;
        #endif
        while(true) {
            int savedErrno = 0;
            ssize_t n = InputBuffer_.readFd(socket_.getFd(), &savedErrno);
            if(n > 0) {
                if(messageCallback_) {
                    std::string sAllData = InputBuffer_.retrieveAllAsString();
                    // messageCallback_(shared_from_this(), std::move(sAllData));
                    server_->submitProxy([self=shared_from_this(), msg=std::move(sAllData)](){
                        self->messageCallback_(self, msg);
                    });
                }
                #ifdef YOYODEBUG
                    std::cout << "TcpConnection hanndleRead string size:" << n << std::endl;
                #endif
                if (!loop_->isUsingETMod()) break; // LT 模式直接退出
                else continue; // ET 模式继续读
            } else if (n == 0) {
                handleClose();
                break;
            } else { // n < 0
                if (savedErrno == EAGAIN || savedErrno == EWOULDBLOCK) break; // 没数据了
                if (savedErrno == EINTR) continue; // 被信号打断，继续尝试
                handleError();
                break;
            }
        }
    }

    void TcpConnection::handleWrite() {
        // 优先处理文件发送
        if (fileSendContext_) {
            _continueFileSend();
            return;
        }

        // 处理普通写缓冲
        if (OutputBuffer_.readableBytes() > 0) {
            ssize_t n = ::write(socket_.getFd(), OutputBuffer_.peek(), OutputBuffer_.readableBytes());
            if (n > 0) {
                OutputBuffer_.retrieve(n);
                if (OutputBuffer_.readableBytes() == 0) {
                    channel_.disableWriteing();
                }
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            } else {
                handleError();
            }
        } else {
            // 没有数据可写了，关闭写事件（防止空转）
            channel_.disableWriteing();
        }
    }

void TcpConnection::_sendFileInLoop(const std::string& headerStr, int fileFd, size_t fileSize) {
    // 创建文件发送上下文
    fileSendContext_ = std::make_unique<FileSendContext>(fileFd, fileSize, headerStr);

    LOGI("Starting sendfile for fd=" + std::to_string(socket_.getFd()) +
         " file_size=" + std::to_string(fileSize) + " bytes");

    // 尝试立即发送
    _continueFileSend();
}

void TcpConnection::_continueFileSend() {
    if (!fileSendContext_) {
        return;
    }

    // 步骤1：先发送HTTP头部（如果还有待发送的）
    if (!fileSendContext_->pendingHeader.empty()) {
        ssize_t n = ::write(socket_.getFd(),
                          fileSendContext_->pendingHeader.data(),
                          fileSendContext_->pendingHeader.size());
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // socket缓冲区满，等待下次可写事件
                #ifdef YOYODEBUG
                std::cerr << "Header send would block, will retry later\n";
                #endif
                channel_.enableWriting();
                return;
            } else {
                #ifdef YOYODEBUG
                std::cerr << "send header error on fd=" << socket_.getFd()
                    << " errno=" << errno << " " << strerror(errno) << "\n";
                #endif
                fileSendContext_.reset();
                handleError();
                return;
            }
        }

        if ((size_t)n < fileSendContext_->pendingHeader.size()) {
            // 头部只发送了一部分，保存剩余部分
            fileSendContext_->pendingHeader =
                fileSendContext_->pendingHeader.substr(n);
            #ifdef YOYODEBUG
            std::cout << "Partial header sent: " << n << " bytes, "
                     << fileSendContext_->pendingHeader.size() << " bytes remaining\n";
            #endif
            channel_.enableWriting();
            return;
        }

        // 头部发送完成
        fileSendContext_->pendingHeader.clear();
        #ifdef YOYODEBUG
        std::cout << "HTTP header sent completely\n";
        #endif
    }

    // 步骤2：使用sendfile发送文件内容
    constexpr size_t SENDFILE_CHUNK_SIZE = 64 * 1024; // 每次最多发送64KB，避免阻塞事件循环

    while (fileSendContext_->offset < (off_t)fileSendContext_->fileSize) {
        size_t remaining = fileSendContext_->fileSize - fileSendContext_->offset;
        size_t toSend = std::min(remaining, SENDFILE_CHUNK_SIZE);

        ssize_t sent = ::sendfile(socket_.getFd(),
                                 fileSendContext_->fileDesc->fd,
                                 &fileSendContext_->offset,
                                 toSend);

        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // socket缓冲区满，等待下次可写事件
                #ifdef YOYODEBUG
                std::cerr << "sendfile would block, sent " << fileSendContext_->offset
                         << " of " << fileSendContext_->fileSize << " bytes\n";
                #endif
                channel_.enableWriting();
                return;
            } else {
                #ifdef YOYODEBUG
                std::cerr << "sendfile error: " << strerror(errno) << "\n";
                #endif
                fileSendContext_.reset();
                handleError();
                return;
            }
        }

        if (sent == 0) {
            // 连接可能已关闭
            #ifdef YOYODEBUG
            std::cerr << "sendfile returned 0, connection may be closed\n";
            #endif
            fileSendContext_.reset();
            handleClose();
            return;
        }

        #ifdef YOYODEBUG
        std::cout << "sendfile sent " << sent << " bytes, total progress: "
                 << fileSendContext_->offset << "/" << fileSendContext_->fileSize << "\n";
        #endif

        // 发送了一个chunk后，让出控制权给事件循环
        // 避免大文件阻塞事件循环
        if (fileSendContext_->offset < (off_t)fileSendContext_->fileSize) {
            channel_.enableWriting();
            return;
        }
    }

    // 文件发送完成
    #ifdef YOYODEBUG
    std::cout << "File sent completely: " << fileSendContext_->fileSize << " bytes\n";
    #endif
    LOGI("File sent successfully: " + std::to_string(fileSendContext_->fileSize) +
         " bytes on fd=" + std::to_string(socket_.getFd()));
    fileSendContext_.reset();

    // 检查是否还有其他数据要发送
    if (OutputBuffer_.readableBytes() == 0) {
        channel_.disableWriteing();
    }
}


} // end of namespace yoyo