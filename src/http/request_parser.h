#ifndef __Http_request_Parser_h_
#define __Http_request_Parser_h_

#include <unordered_set>
#include <iostream>
#include <string>
#include "request.h"

namespace yoyo {
    namespace http {

        enum class HttpParserState {
            START,              // 初始状态
            PARSE_METHOD,       // 解析请求方法（如GET/POST）
            PARSE_URL,          // 解析URL路径
            PARSE_VERSION,      // 解析HTTP版本（如HTTP/1.1）
            PARSE_HEADER_KEY,   // 解析头部键（如Content-Length）
            PARSE_HEADER_VALUE, // 解析头部值
            PARSE_EMPTY_LINE,   // 解析头部结束后的空行
            PARSE_BODY,         // 解析消息体
            COMPLETE,           // 解析完成
            ERROR               // 解析错误
        };

        enum class ParserError {
            NO_ERROR,   
            INVALID_VERSION,    // 请求版本错误
            INVALID_METHOD,     // 请求方法错误
            INVALID_URL,        // url错误
            INVALID_HEADER,     // 头部错误
            MISSING_CRLF,       // 分隔符错误
            BODY_TOO_LARGE      //
        };

        class HttpRequest;

        class HttpRequestParser {
        public:
            HttpRequestParser() { reset(); }
            void reset();
            bool parse(const char* stream, size_t len) {
                return parse(std::string_view(stream, len));
            }
            bool parse(std::string_view stream);
            bool parseHeader(std::string_view stream);

            std::string getErrorMsg() const {
                switch (_error) {
                    case ParserError::NO_ERROR: return "No error";
                    case ParserError::INVALID_METHOD: return "Invalid request method";
                    case ParserError::INVALID_URL: return "Invalid URL";
                    case ParserError::INVALID_VERSION: return "Invalid HTTP version";
                    case ParserError::INVALID_HEADER: return "Invalid header format";
                    case ParserError::MISSING_CRLF: return "Missing CRLF (\\r\\n)";
                    case ParserError::BODY_TOO_LARGE: return "Body exceeds Content-Length";
                    default: return "Unknown error";
                }
            }

            const HttpRequest& getRequse() const {
                return _request;
            }

            ParserError getErrorCode() const {
                return _error;
            }


        private:
            void processStreamByteWise(char c);

            bool isValidMethod(std::string sMethod) const {
                return _httpMethod.find(sMethod) != _httpMethod.end();
            }

            bool isValidVersion(const std::string_view& s) {
                return s == "HTTP/1.1" || s == "HTTP/1.0";
            }

            inline const static std::unordered_set<std::string_view> _httpMethod {
                "GET",
                "POST",
                "DELETE",
                "PUT", 
                "HEAD", 
                "OPTIONS", 
                "PATCH"
            };

        private:
            HttpRequest _request;
            HttpParserState _state;
            ParserError _error;
            bool _isHeaderEnd = false;

            std::string _sHeaderKey;

            std::string _buffer;     // 缓存区
            size_t _contentLength;
            size_t _receiveLength;
            bool _isChunk;      // 是否分块传输
            int _crlfCount = 0;            // 连续\r\n计数器（用于判断头部结束）
        };


    } // end of http
} // end of yoyo



#endif
