#include "request.h"


// 辅助函数：判断字符是否需要编码
inline static bool isUnreserved(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) ||
        c == '-' || c == '_' || c == '.' || c == '~';
}


// URL 编码
inline static std::string urlEncode(const std::string& str) {
    std::string encoded_str;
    encoded_str.reserve(str.length() * 1.5); // 预分配一些空间

    for (char c : str) {
        if (isUnreserved(c)) {
            encoded_str += c;
        } else if (c == ' ') {
            encoded_str += '+'; // 空格编码为 '+'
        } else {
            // 所有其他字符编码为 %HH
            encoded_str += '%';
            encoded_str += "0123456789ABCDEF"[ (static_cast<unsigned char>(c) >> 4) & 0xF ];
            encoded_str += "0123456789ABCDEF"[ (static_cast<unsigned char>(c)     ) & 0xF ];
        }
    }
    return encoded_str;
}


namespace yoyo {
    namespace http {
        

        const std::string& HttpRequest::getMethod() const {
            return method_;
        }
        const std::string HttpRequest::getUrl() const {
            if(!url_.empty()) return url_;
            std::string fullUri = path_;
            if (!params_.empty()) {
                fullUri += "?";
                bool first = true;
                for (const auto& pair : params_) {
                    if (!first) {
                        fullUri += "&";
                    }
                    // 确保键和值被正确 URL 编码
                    fullUri += urlEncode(pair.first) + "=" + urlEncode(pair.second);
                    first = false;
                }
            }
            return fullUri;
        }
        const std::string& HttpRequest::getPath() const {
            return path_;
        }
        const std::string& HttpRequest::getVersion() const {
            return version_;
        } 
        const HttpRequest::Headers& HttpRequest::getheader() const {
            return  headrs_;
        }
        const HttpRequest::Paremeters& HttpRequest::getParams() const {
            return params_;
        }
        const std::vector<HttpRequest::byte>& HttpRequest::getBody() const {
            return body_;
        }
        

        // 外部设置接口
        void HttpRequest::setMethod(const std::string& method) {
            method_ = method;
        }
        void HttpRequest::setUrl(const std::string& url) {
            url_ = url;
        }
        void HttpRequest::setVersion(const std::string& version) {
            version_ = version;
        }
        void HttpRequest::setheaders(const HttpRequest::Headers& header) {
            headrs_ = header;
        }
        void HttpRequest::setParams(const HttpRequest::Paremeters& params) {
            params_ = params;
        }
        void HttpRequest::addHeaders(const std::string& key, const std::string& value) {
            headrs_[key] = value;
        }
        void HttpRequest::addParams(const std::string& key, const std::string& value) {
            params_[key] = value;
        }

        void HttpRequest::setBody(const std::vector<HttpRequest::byte>& body) {
            body_ = body;
        }
        void HttpRequest::addBody(const std::vector<HttpRequest::byte>& body) {
            for(auto c : body) body_.push_back(c);
        }
        void HttpRequest::clear() {
            method_.clear();
            url_.clear();
            path_.clear();
            version_.clear();
            headrs_.clear();
            params_.clear();
            body_.clear();
        }

        std::string HttpRequest::getMapString(const std::map<std::string, std::string>& mp) const {
                return my_tostr(mp.begin(), mp.end(), [](auto&& p) {
                    #ifdef USE_STD_FORMAT_AVAILABLE
                        return std::format("key:{0}, value:{1}", p.first, p.second);
                    #else
                        return "key:" +  p.first + ", value:" + p.second;
                    #endif
                });
            }

        std::string HttpRequest::writeToString() const {
            #ifdef USE_STD_FORMAT_AVAILABLE
                return std::format("sMethod:{0}|url:{1}|sVersion:{2}|header:{3}|parameter:{4}", method_, url_, version_, my_tostr(headrs_.begin(), headrs_.end(), [](auto&& p) {
                    return std::format("key:{0}, value:{1}", p.first, p.second);
                }), getMapString(params_));
            #else
                std::string sHeader = my_tostr(headrs_.begin(), headrs_.end(), [](auto&& p) {
                    return "key:" +  p.first + ", value:" + p.second;
                });
                return "sMethod:" + method_ + "|url:" + url_ + "|sVersion:" + version_ + "|header:" + sHeader + "|parameter:" + getMapString(params_);
            #endif
        }

        std::string HttpRequest::serialize() const {
            std::string result;
            // 状态行：HTTP版本 状态码 原因短语\r\n
            #ifdef USE_STD_FORMAT_AVAILABLE
                result = std::format("{0} {1} {2}\r\n", method_, url_, version_);
            #else
                result = method_ + " " + url_ + " " + version_ + "\r\n";
            #endif

            for(const auto& [key, value] : headrs_) {
                std::string format_key = key;
                if(format_key.empty() == false) {
                    format_key[0] = toupper(format_key[0]);
                    for(size_t i = 1; i < format_key.size(); i++) {
                        if(format_key[i - 1] == '-') {
                            format_key[i] = toupper(format_key[i]);
                        }
                    }
                }
                result += format_key + ": " + value + "\r\n";
            }
            
            result += "\r\n";
            result += std::string(body_.begin(), body_.end());

            return result;
        }
    } // end of http
}   // end of yoyo

