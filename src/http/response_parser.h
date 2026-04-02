#ifndef _yoyo_http_response_parser_h_
#define _yoyo_http_response_parser_h_

#include "response.h"

namespace yoyo {
    namespace http {
        // 响应解析器错误码
        enum class ResponseParseError {
            NO_ERROR,
            INVALID_VERSION,    // 无效HTTP版本
            INVALID_STATUS_CODE,// 无效状态码
            MISSING_CRLF,       // 缺少\r\n分隔符
            INVALID_HEADER,     // 无效响应头
            BODY_TOO_LARGE      // 响应体超过Content-Length
        };

        // 响应解析器状态
        enum class ResponseParseState {
            START,               // 初始状态
            PARSE_VERSION,       // 解析HTTP版本
            PARSE_STATUS_CODE,   // 解析状态码
            PARSE_REASON,        // 解析原因短语
            PARSE_ENMPTY_LINE,   // 解析/r/n
            PARSE_HEADER_KEY,    // 解析响应头键
            PARSE_HEADER_VALUE,  // 解析响应头值
            PARSE_BODY,          // 解析响应体
            COMPLETE,            // 解析完成
            ERROR                // 解析错误
        };


        class ResponseParser {
        public:
            ResponseParser() { reset(); }

            void reset();

            bool parse(const char* data, size_t len) {
                return parse(std::string_view(data, len));
            }

            bool parse(std::string_view data);

            const HttpResponse& getResponse() const { return response_; }
            ResponseParseError getError() const { return error_; }
            std::string getErrorMsg() const {
                switch (error_) {
                    case ResponseParseError::NO_ERROR: return "No error";
                    case ResponseParseError::INVALID_VERSION: return "Invalid HTTP version";
                    case ResponseParseError::INVALID_STATUS_CODE: return "Invalid status code";
                    case ResponseParseError::MISSING_CRLF: return "Missing CRLF (\\r\\n)";
                    case ResponseParseError::INVALID_HEADER: return "Invalid header format";
                    case ResponseParseError::BODY_TOO_LARGE: return "Body exceeds Content-Length";
                    default: return "Unknown error";
                }
            }

        private:
            void processChar(char c);

            // 初始状态处理
            void handleStart(char c);
                

            // 解析HTTP版本（如HTTP/1.1）
            void handleParseVersion(char c);
               
            // 解析状态码（如200）
            void handleParseStatusCode(char c);
        
            // 解析原因短语（如OK）
            void handleParseReason(char c);
               
            void handleParseEmptyline(char c);
               
            // 解析响应头键
            void handleParseHeaderKey(char c);

            // 解析响应头值
            void handleParseHeaderValue(char c);
                
            // 处理头部结束，准备解析响应体
            void processHeaderEnd();
                
            // 解析响应体
            void handleParseBody(char c);

        private:
            HttpResponse response_;
            ResponseParseError error_;
            ResponseParseState state_;
            std::string buffer_;                 // 临时缓存
            std::string headerKey_;              // 当前解析的头键
            size_t contentLength_ = 0;           // 响应体长度（来自Content-Length）
            size_t receivedLength_ = 0;          // 已接收的响应体长度
            bool isChunked_ = false;             // 是否分块传输
            bool expectingLF_ = false;           // 等待\r后的\n
        };

    } // end of http
} // end of yoyo






#endif