#include "response.h"


namespace yoyo {
    namespace http {


        const std::string& HttpResponse::getVersion() const {
            return version_;
        }

        const std::string& HttpResponse::getReason() const {
            return reason_;
        }

        int HttpResponse::getStatusCode() const {
            return statusCode_;
        } 
        
        const HttpResponse::headers& HttpResponse::getHeaders() const {
            return headers_;
        }
        
        const std::vector<HttpResponse::byte>& HttpResponse::getBody() const {
            return body_;
        }
           
        void HttpResponse::setVersion(const std::string& version) {
            version_ = version;
        }
        void HttpResponse::setStatusCode(int iCode) {
            statusCode_ = iCode;
        }
        void HttpResponse::setReason(const std::string& reason) {
            reason_ = reason;
        }
        void HttpResponse::setheaders(const headers& header) {
            headers_ = header;
        }
        void HttpResponse::setBody(const std::vector<byte>& body) {
            body_ = body;
            fileInfo_.reset(); // 清除文件信息，因为现在使用普通 body
        }
        void HttpResponse::appendBody(byte c) {
            body_.push_back(c);
        }
        void HttpResponse::addHeader(const std::string& key, const std::string& value) {
            std::string lowerKey = key;
            std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower); // 转换为小写
            headers_[lowerKey] = value; // 使用 operator[] 会覆盖现有值，符合大多数HTTP头的行为;
        } 
        void HttpResponse::setBody(const std::string& str) {
            std::vector<byte> vBody(str.begin(), str.end());
            setBody(vBody);
        }

        // clear
        void HttpResponse::clear() {
            version_.clear();
            statusCode_ = 0;
            reason_.clear();
            headers_.clear();
            body_.clear();
        }

        std::string HttpResponse::serialize() const {
            std::string result;
            // 状态行：HTTP版本 状态码 原因短语\r\n
            #ifdef USE_STD_FORMAT_AVAILABLE
                result += std::format("{} {} {}\r\n", version_, statusCode_, reason_);
            #else
                result += version_ + " " + std::to_string(statusCode_) + " " + reason_ + "\r\n";
            #endif
            
            headers finalHeaders = headers_;
            bool isInformational = (statusCode_ >= 100 && statusCode_ < 200);
            bool isNoContent = (statusCode_ == 204);
            bool isNotModified = (statusCode_ == 304);

            if(isInformational || isNoContent || isNotModified) {
                // 1xx、204和304响应不应包含消息体，因此不应包含Content-Length或Transfer-Encoding头
                finalHeaders.erase("content-length");
                finalHeaders.erase("transfer-encoding");
            } else {
                finalHeaders["content-length"] = std::to_string(body_.size());
            }

            // 响应头：键: 值\r\n
            for (const auto& [key, value] : finalHeaders) {
                std::string formattedKey = key;
                if (!formattedKey.empty()) {
                    formattedKey[0] = toupper(formattedKey[0]);
                    for (size_t i = 1; i < formattedKey.size(); ++i) {
                        if (formattedKey[i-1] == '-') {
                            formattedKey[i] = toupper(formattedKey[i]);
                        }
                    }
                }
            result += formattedKey + ": " + value + "\r\n";
        }
        // 空行分隔头和体
        result += "\r\n";
        // 响应体
        result.append(reinterpret_cast<const char*>(body_.data()), body_.size());
        return result;
    }

    std::string HttpResponse::serializeHeaders(size_t contentLength) const {
        std::string result;
        // 状态行：HTTP版本 状态码 原因短语\r\n
        #ifdef USE_STD_FORMAT_AVAILABLE
            result += std::format("{} {} {}\r\n", version_, statusCode_, reason_);
        #else
            result += version_ + " " + std::to_string(statusCode_) + " " + reason_ + "\r\n";
        #endif

        headers finalHeaders = headers_;
        bool isInformational = (statusCode_ >= 100 && statusCode_ < 200);
        bool isNoContent = (statusCode_ == 204);
        bool isNotModified = (statusCode_ == 304);

        if(isInformational || isNoContent || isNotModified) {
            // 1xx、204和304响应不应包含消息体
            finalHeaders.erase("content-length");
            finalHeaders.erase("transfer-encoding");
        } else {
            finalHeaders["content-length"] = std::to_string(contentLength);
        }

        // 响应头：键: 值\r\n
        for (const auto& [key, value] : finalHeaders) {
            std::string formattedKey = key;
            if (!formattedKey.empty()) {
                formattedKey[0] = toupper(formattedKey[0]);
                for (size_t i = 1; i < formattedKey.size(); ++i) {
                    if (formattedKey[i-1] == '-') {
                        formattedKey[i] = toupper(formattedKey[i]);
                    }
                }
            }
            result += formattedKey + ": " + value + "\r\n";
        }
        // 空行分隔头和体
        result += "\r\n";
        return result;
    }

    std::string HttpResponse::writeToString() const {
        std::string body_str;
        if (!body_.empty()) {
            // 仅在 body_ 可能是文本时才尝试转换为字符串
            // 或者提供一个截断的表示，并指出其大小
            if (body_.size() < 1024 && std::all_of(body_.begin(), body_.end(), [](byte b){ return std::isprint(b) || std::isspace(b); })) {
                body_str.assign(body_.begin(), body_.end());
            } else {
                body_str = "<binary data, size=" + std::to_string(body_.size()) + " bytes>";
            }
        }
        #ifdef USE_STD_FORMAT_AVAILABLE
            return std::format("status:{0}|body:{1}", statusCode_, body_str);
        #else
            return "status:" + std::to_string(statusCode_) + "|body:" + body_str;
        #endif
        
    }

    } // end of http
} // end of yoyo