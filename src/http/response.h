#ifndef _yoyo_http_response_h_
#define _yoyo_http_response_h_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <optional>
#if __has_include(<format>) && defined(__cpp_lib_format)
    #include <format>
    #define USE_STD_FORMAT_AVAILABLE
#endif

namespace yoyo {
    namespace http {
        class HttpResponse {
        public:
            using byte = unsigned char;
            using headers = std::map<std::string, std::string>;

            struct FileInfo {
                int fd;
                size_t size;
                FileInfo(int f, size_t s) : fd(f), size(s) {}
            };

            // ctor
            HttpResponse() = default;
            HttpResponse(const std::string& version, int code, const std::string& reason, const headers& head, const std::vector<byte>& body)
                : version_(version), statusCode_(code), reason_(reason), headers_(head), body_(body)
                {}
            
            const std::string& getVersion() const;
            const std::string& getReason() const;
            int getStatusCode() const;
            const headers& getHeaders() const;
            const std::vector<byte>& getBody() const;
            
            // 访问接口
            void setVersion(const std::string& version);
            void setStatusCode(int iCode);
            void setReason(const std::string& reason);
            void setheaders(const headers& header);
            void setBody(const std::vector<byte>& body);
            void appendBody(byte c);
            void addHeader(const std::string& key, const std::string& value);
            void setBody(const std::string& str);
            void clear();

            // 文件相关方法
            void setFileInfo(int fd, size_t size) { fileInfo_ = FileInfo(fd, size); }
            bool hasFile() const { return fileInfo_.has_value(); }
            const std::optional<FileInfo>& getFileInfo() const { return fileInfo_; }

            std::string serialize() const;
            std::string serializeHeaders(size_t contentLength) const; // 只序列化头部，用于 sendfile
            std::string writeToString() const;
            

        private:
            std::string version_;
            int statusCode_ = 0;
            std::string reason_;
            headers headers_;
            std::vector<byte> body_;
            std::optional<FileInfo> fileInfo_; // 用于 sendfile 的文件信息
        };

    } // end of http
} // end of yoyo


#endif