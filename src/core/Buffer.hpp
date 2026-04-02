#ifndef _YOYO_BUFFER_HPP_
#define _YOYO_BUFFER_HPP_

#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <cassert>
#include <sys/uio.h> // readv

namespace yoyo {

class Buffer {
public:
    explicit Buffer(size_t initialSize = 1024)
        : buffer_(initialSize), readIndex_(0), writeIndex_(0) {}
    // 未读数据长度
    size_t readableBytes() const { return writeIndex_ - readIndex_; }
    // 可写数据长度
    size_t writableBytes() const { return buffer_.size() - writeIndex_; }
    // 已读数据长度
    size_t prependableBytes() const { return readIndex_; }

    const char* peek() const { return begin() + readIndex_; }
    
    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            readIndex_ += len;
        } else {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end) {
        assert(peek() <= end && end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveAll() {
        readIndex_ = 0;
        writeIndex_ = 0;
    }

    std::string retrieveAllAsString() {
        std::string str(peek(), readableBytes());
        retrieveAll();
        return str;
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writeIndex_ += len;
    }

    void append(const std::string& str) {
        append(str.data(), str.size());
    }

    ssize_t readFd(int fd, int* savedErrno) {
        char extrabuf[65536];
        struct iovec vec[2];
        size_t writable = writableBytes();

        vec[0].iov_base = beginWrite();
        vec[0].iov_len  = writable;
        vec[1].iov_base = extrabuf;
        vec[1].iov_len  = sizeof extrabuf;

        int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
        ssize_t n = ::readv(fd, vec, iovcnt);
        if (n < 0) {
            *savedErrno = errno;
        } else if (static_cast<size_t>(n) <= writable) {
            writeIndex_ += n;
        } else {
            writeIndex_ = buffer_.size();
            append(extrabuf, n - writable);
        }
        return n;
    }

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    char* beginWrite() { return begin() + writeIndex_; }
    const char* beginWrite() const { return begin() + writeIndex_; }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len) {
            buffer_.resize(writeIndex_ + len);
        } else {
            size_t readable = readableBytes();
            std::copy(begin() + readIndex_,
                      begin() + writeIndex_,
                      begin());
            readIndex_ = 0;
            writeIndex_ = readable;
        }
    }

private:
    std::vector<char> buffer_;
    size_t readIndex_;  // 需要读取的位置
    size_t writeIndex_; // 需要写入的位置
};

} // namespace yoyo

#endif