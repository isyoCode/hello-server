#include "request_parser.h"
#include "request.h"
#include <algorithm> // For std::transform
#include <cctype>    // For ::tolower (good practice to explicitly include)



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

// URL 解码
inline static std::string urlDecode(const std::string& str) {
    std::string decoded_str;
    decoded_str.reserve(str.length()); // 预分配空间

    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        if (c == '+') {
            decoded_str += ' '; // '+' 解码为空格
        } else if (c == '%') {
            if (i + 2 < str.length() && std::isxdigit(static_cast<unsigned char>(str[i+1])) && std::isxdigit(static_cast<unsigned char>(str[i+2]))) {
                // 解析 %HH 编码
                std::string hex_byte = str.substr(i + 1, 2);
                decoded_str += static_cast<char>(std::stoi(hex_byte, nullptr, 16));
                i += 2; // 跳过两个十六进制字符
            } else {
                // 无效的 % 编码，可以根据需要处理，这里选择保留 %
                decoded_str += c;
            }
        } else {
            decoded_str += c;
        }
    }
    return decoded_str;
}



namespace yoyo {
    namespace http {
        void HttpRequestParser::reset() {
            _state = HttpParserState::START;
            _error = ParserError::NO_ERROR;
            _isChunk = false;
            _contentLength = 0;
            _receiveLength = 0;
            _buffer.clear();
            _request = HttpRequest();
            _buffer.reserve(256);
        }

        bool HttpRequestParser::parse(std::string_view stream) {
            //TODO 根据当前状态进行逐字符解析
            if(_state == HttpParserState::COMPLETE || _state == HttpParserState::ERROR) {
                return false;
            }
            for(size_t i = 0; i < stream.size(); i++) {
                processStreamByteWise(stream[i]);
                // std::cout << "p state: " << static_cast<int>(_state) << std::endl; 
                if(_state == HttpParserState::ERROR || _state == HttpParserState::COMPLETE) {
                    break;
                }
            }
            #ifdef YOYODEBUG
                std::cout << "_state:" << static_cast<int>(_state) << std::endl;
            #endif
            return _state == HttpParserState::COMPLETE;
        }

        // 作为http头部解析的一个阶段 解析完成后可能进入body解析阶段 也可能直接完成
        bool HttpRequestParser::parseHeader(std::string_view stream) {
            //TODO 根据当前状态进行逐字符解析
            if(_state == HttpParserState::COMPLETE || _state == HttpParserState::ERROR) {
                return false;
            }
            for(size_t i = 0; i < stream.size(); i++) {
                processStreamByteWise(stream[i]);
                // std::cout << "p state: " << static_cast<int>(_state) << std::endl; 
                if(_state == HttpParserState::ERROR || _state == HttpParserState::COMPLETE) {
                    break;
                }
            }
            #ifdef YOYODEBUG
                std::cout << "[parseHeader] _state:" << static_cast<int>(_state) << std::endl;
                if(_state == HttpParserState::PARSE_BODY || _state == HttpParserState::COMPLETE) {
                    std::cout << "header parse complete " << std::endl;
                    std::cout << "request:" << _request.writeToString() << std::endl;

                    // auto header = _request.getheader();
                    // for(const auto& [key, value] : header) {
                    //     std::cout << "header key: " << key << "  value: " << value << std::endl;
                    // }
                }
            #endif
            return _state == HttpParserState::PARSE_BODY || _state == HttpParserState::COMPLETE;
        }

        void HttpRequestParser::processStreamByteWise(char c) {
            switch (_state) {
                case  HttpParserState::START : 
                    //TODO 非空解析
                    if(!isspace(c)){
                        _buffer += c;
                        _state = HttpParserState::PARSE_METHOD;            
                    }
                    break;
                case HttpParserState::PARSE_METHOD :
                    //TODO 直到为空为止
                    if(isspace(c)) {
                        if(!isValidMethod(_buffer)) {
                            _state = HttpParserState::ERROR;
                            _error = ParserError::INVALID_METHOD;
                            return;
                        }
                        _request.method_ = _buffer.substr(0);       // _buffer 初始时应使用reserve 防止内存多次分配扩容
                        _buffer.clear();
                        _state = HttpParserState::PARSE_URL;
                    } else {
                        _buffer += c;
                    }
                    break; 
                case HttpParserState::PARSE_URL :
                    if(!isspace(c)) {
                        _buffer += c;
                    }else {
                        //TODO 解析完整的URL 此处的uri应该是单纯的资源定位符？
                        if(_buffer.empty()) {
                            _state = HttpParserState::ERROR;
                            _error = ParserError::INVALID_URL;
                            return;
                        }
                        
                        // TODO 解析参数
                        size_t queryPos = _buffer.find("?");
                        if(queryPos != std::string::npos) {
                            _request.path_ = _buffer.substr(0, queryPos);
                            std::string sQuery = _buffer.substr(queryPos + 1);
                            
                            size_t start = 0;
                            while(start < sQuery.size()) {
                                size_t end = sQuery.find('&', start);
                                if(end == std::string::npos) {
                                    end = sQuery.size();
                                }
                                
                                std::string sParameter = sQuery.substr(start, end - start);
                                start = end + 1;
                                
                                // 按 = 进行分割
                                size_t eq_ops = sParameter.find('=');
                                if(eq_ops != std::string::npos) {
                                    std::string sKey = sParameter.substr(0, eq_ops);
                                    std::string sValue = sParameter.substr(eq_ops + 1);
                                    _request.params_[urlDecode(std::move(sKey))] = urlDecode(std::move(sValue));
                                }else {
                                    _request.params_[urlDecode(std::move(sParameter))] = "";
                                }
                            }

                        }else {
                            _request.path_ = _buffer.substr(0);
                        }
                        _state = HttpParserState::PARSE_VERSION;
                        _buffer.clear();
                    }
                    break; 
                case HttpParserState::PARSE_VERSION :
                    if(c == '\r') {
                        // TODO 版本解析完成 检查版本是否有效
                        if(_buffer.empty() || !isValidVersion(_buffer)) {
                            _error = ParserError::INVALID_VERSION;
                            _state = HttpParserState::ERROR;
                            return;
                        }
                        _request.version_ = _buffer.substr(0);
                        _buffer.clear();
                        _state = HttpParserState::PARSE_EMPTY_LINE;
                    }else {
                        _buffer += c;
                    }
                break; 
                case HttpParserState::PARSE_EMPTY_LINE :
                    // TODO 检查空行 -> 当前字符是否为 "\n"
                    if(c != '\n') {
                        _error = ParserError::MISSING_CRLF; //格式错误
                        _state = HttpParserState::ERROR;
                        return;
                    }
                    _crlfCount++;
                    if(_crlfCount == 1) {
                        // 第一个\r\n：可能是请求行结束，进入头部解析
                        _state = HttpParserState::PARSE_HEADER_KEY;
                    } else if(_crlfCount == 2) {
                        // TODO 解析头部/或者body内容
                        auto it = _request.headrs_.find("content-length");  // 头部键已转为小写
                        if(it != _request.headrs_.end()) {
                            try {
                                _contentLength = std::stoull(it->second);
                                std::cout << "parser contenllength: " << _contentLength << std::endl;
                            } catch (...) {
                                _error = ParserError::INVALID_HEADER;
                                _state = HttpParserState::ERROR;
                                return;
                            }
                        }

                        it = _request.headrs_.find("transfer-encoding");
                        if(it != _request.headrs_.end() && it->second == "chunked") {
                            _isChunk = true;
                        }

                        // 判断是否有消息体
                        if(_contentLength == 0 && !_isChunk) {
                            _state = HttpParserState::COMPLETE;
                        } else {
                            _state = HttpParserState::PARSE_BODY;
                        }
                        _crlfCount = 0;  // 重置计数器
                    }
                    break; 
                case HttpParserState::PARSE_HEADER_KEY :
                    // TODO 解析头部 到 ':' 或者 '\r' 
                    if(c == ':') {
                        if(_buffer.empty()) {
                            _state = HttpParserState::ERROR;
                            _error = ParserError::INVALID_HEADER;
                            return;
                        }
                        // TODO 统一大小写问题
                        std::transform(_buffer.begin(), _buffer.end(), _buffer.begin(), ::tolower);
                        _sHeaderKey = _buffer.substr(0);
                        // std::cout << "current header: " << _sHeaderKey << std::endl;
                        _buffer.clear();
                        _state = HttpParserState::PARSE_HEADER_VALUE;
                    } else if (c == '\r') {
                        _state = HttpParserState::PARSE_EMPTY_LINE;
                    }else {
                        _buffer += c;
                        _crlfCount = 0;
                    }
                break; 
                case HttpParserState::PARSE_HEADER_VALUE :
                    if( c != '\r') {
                        _buffer += c;
                    }else {
                        //TODO 去除空白内容
                        size_t start = _buffer.find_first_not_of(" \t");
                        size_t end = _buffer.find_last_not_of(" \t");
                        if (start != std::string::npos && end != std::string::npos) {
                            _request.headrs_[_sHeaderKey] = _buffer.substr(start, end - start + 1);
                        }else {
                            _request.headrs_[_sHeaderKey] = "";
                        }
                        // std::cout << "valuee: " << _request.headrs_[_sHeaderKey] << std::endl;
                        _sHeaderKey.clear();
                        _buffer.clear();
                        _state = HttpParserState::PARSE_EMPTY_LINE;
                    }
                break; 
                case HttpParserState::PARSE_BODY :
                    // TINY版本 未处理分块逻辑
                    _request.body_.push_back(static_cast<unsigned char>(c));
                    _receiveLength++;
                    if(_receiveLength > _contentLength) {
                        _error = ParserError::BODY_TOO_LARGE;
                        _state = HttpParserState::ERROR;
                        return;
                    }
                    if(_receiveLength == _contentLength) {
                        _state = HttpParserState::COMPLETE;
                    }
                    #ifdef YOYODEBUG
                        std::cout << "body size: " << _request.body_.size()  << "  body: " << std::string(_request.body_.begin(), _request.body_.end()) << std::endl;
                        std::cout << "receive length :" << _receiveLength << std::endl;
                    #endif
                    break; 
                case HttpParserState::ERROR :
                    std::cout << "解析状态错误, 终止解析!" << "\n";
                break; 
                case HttpParserState::COMPLETE :
                    std::cout << "解析已完成!" << "\n";
                break; 
                default :
                break; 
            }
        }


    } // end of http
} // end of yoyo