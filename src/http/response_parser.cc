#include "response_parser.h"

namespace yoyo {
    namespace http {

        void ResponseParser::reset() {
            state_ = ResponseParseState::START;
            error_ = ResponseParseError::NO_ERROR;
            buffer_.clear();
            headerKey_.clear();
            contentLength_ = 0;
            receivedLength_ = 0;
            isChunked_ = false;
            expectingLF_ = false;
            response_.clear();
            buffer_.reserve(256);
        }


        bool ResponseParser::parse(std::string_view data) {
            if (state_ == ResponseParseState::COMPLETE || state_ == ResponseParseState::ERROR) {
                return false;
            }
            for (char c : data) {
                processChar(c);
                if (state_ == ResponseParseState::ERROR || state_ == ResponseParseState::COMPLETE) {
                    break;
                }
            }
            return state_ == ResponseParseState::COMPLETE;
        }
        
        // 初始状态处理
        void ResponseParser::handleStart(char c) {
            if (!isspace(c)) {
                buffer_ += c;
                state_ = ResponseParseState::PARSE_VERSION;
            }
        }

        // 解析HTTP版本（如HTTP/1.1）
        void ResponseParser::handleParseVersion(char c) {
            if (isspace(c)) {
                if (buffer_.empty() || !(buffer_ == "HTTP/1.1" || buffer_ == "HTTP/1.0")) {
                    error_ = ResponseParseError::INVALID_VERSION;
                    state_ = ResponseParseState::ERROR;
                    return;
                }
                response_.setVersion(buffer_);
                buffer_.clear();
                state_ = ResponseParseState::PARSE_STATUS_CODE;
            } else {
                buffer_ += c;
            }
        }

        // 解析状态码（如200）
        void ResponseParser::handleParseStatusCode(char c) {
            if (isspace(c)) {
                if (buffer_.empty() || buffer_.size() != 3 || !isdigit(buffer_[0]) || 
                    !isdigit(buffer_[1]) || !isdigit(buffer_[2])) {
                    error_ = ResponseParseError::INVALID_STATUS_CODE;
                    state_ = ResponseParseState::ERROR;
                    return;
                }
                response_.setStatusCode(std::stoi(buffer_));
                buffer_.clear();
                state_ = ResponseParseState::PARSE_REASON;
            } else if (isdigit(c)) {
                buffer_ += c;
            } else {
                error_ = ResponseParseError::INVALID_STATUS_CODE;
                state_ = ResponseParseState::ERROR;
            }
        }

        // 解析原因短语（如OK）
        void ResponseParser::handleParseReason(char c) {
            if (c == '\r') {
                response_.setReason(buffer_);
                buffer_.clear();
                state_ = ResponseParseState::PARSE_ENMPTY_LINE;
            } else {
                buffer_ += c;
            }
        }

        void ResponseParser::handleParseEmptyline(char c) {
            if(c != '\n') {
                error_ = ResponseParseError::MISSING_CRLF;
                state_ = ResponseParseState::ERROR;
                return;
            }
            if(expectingLF_) {  // 连续两次/r/n 换body
                processHeaderEnd();
            }else {
                state_ = ResponseParseState::PARSE_HEADER_KEY;
                expectingLF_ = true;
            }

        }

        // 解析响应头键
        void ResponseParser::handleParseHeaderKey(char c) {
            if (c == ':') {
                if (buffer_.empty()) {
                    error_ = ResponseParseError::INVALID_HEADER;
                    state_ = ResponseParseState::ERROR;
                    return;
                }
                std::string key = buffer_;
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                headerKey_ = key;
                buffer_.clear();
                state_ = ResponseParseState::PARSE_HEADER_VALUE;
            } else if (c == '\r') {
                state_ = ResponseParseState::PARSE_ENMPTY_LINE;
            } else {
                buffer_ += c;
                expectingLF_ = false;
            }
        }

        // 解析响应头值
        void ResponseParser::handleParseHeaderValue(char c) {
            if (c == '\r') {
                size_t start = buffer_.find_first_not_of(" \t");
                size_t end = buffer_.find_last_not_of(" \t");
                std::string value;
                if (start != std::string::npos && end != std::string::npos) {
                    value = buffer_.substr(start, end - start + 1);
                }
                response_.addHeader(headerKey_, value);
                headerKey_.clear();
                buffer_.clear();
                state_ = ResponseParseState::PARSE_ENMPTY_LINE;
            } else {
                buffer_ += c;
            }
        }

        // 处理头部结束，准备解析响应体
        void ResponseParser::processHeaderEnd() {
            auto it = response_.getHeaders().find("content-length");
            if (it != response_.getHeaders().end()) {
                try {
                    contentLength_ = std::stoull(it->second);
                } catch (...) {
                    error_ = ResponseParseError::INVALID_HEADER;
                    state_ = ResponseParseState::ERROR;
                    return;
                }
            }

            it = response_.getHeaders().find("transfer-encoding");
            if (it != response_.getHeaders().end() && it->second == "chunked") {
                isChunked_ = true;
            }

            // 只要有分块或内容长度>0，就解析响应体
            if (isChunked_ || contentLength_ > 0) {
                state_ = ResponseParseState::PARSE_BODY;
            } else {
                state_ = ResponseParseState::COMPLETE;
            }
        }

        // 解析响应体
        void ResponseParser::handleParseBody(char c) {
            response_.appendBody(static_cast<unsigned char>(c));
            receivedLength_++;

            // 检查是否超过Content-Length
            if (receivedLength_ > contentLength_) {
                error_ = ResponseParseError::BODY_TOO_LARGE;
                state_ = ResponseParseState::ERROR;
                return;
            }

            // 响应体长度达标，解析完成
            if (receivedLength_ == contentLength_) {
                state_ = ResponseParseState::COMPLETE;
            }
        }

        void ResponseParser::processChar(char c) {
            // 状态机处理（不变）
            switch (state_) {
                case ResponseParseState::START:
                    handleStart(c);
                    break;
                case ResponseParseState::PARSE_VERSION:
                    handleParseVersion(c);
                    break;
                case ResponseParseState::PARSE_STATUS_CODE:
                    handleParseStatusCode(c);
                    break;
                case ResponseParseState::PARSE_REASON:
                    handleParseReason(c);
                    break;
                case ResponseParseState::PARSE_HEADER_KEY:
                    handleParseHeaderKey(c);
                    break;
                case ResponseParseState::PARSE_ENMPTY_LINE:
                    handleParseEmptyline(c);
                    break;
                case ResponseParseState::PARSE_HEADER_VALUE:
                    handleParseHeaderValue(c);
                    break;
                case ResponseParseState::PARSE_BODY:
                    handleParseBody(c);
                    break;
                default:
                    break;
            }
        }

    } // end of http
} // end of yoyo

