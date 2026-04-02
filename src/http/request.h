#ifndef _HTTP_REQUEST_H__
#define _HTTP_REQUEST_H__

#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace yoyo {
    namespace http {
        template <class iter, class F>
            std::string my_tostr(iter begin, iter end, F f, const std::string sep = " ") {
            std::string s;
            for(auto it = begin; it != end; it++) {
                s += f(*it);
                s += sep;
            }
            if(s.empty() == false) s.pop_back();
            return s;
        }

        class HttpRequestParser;

        class HttpRequest {
            friend class HttpRequestParser;
        public:
            using byte = unsigned char;
            using Headers = std::map<std::string, std::string, std::less<std::string>>;
            using Paremeters = std::map<std::string, std::string>; 

            HttpRequest() = default;
            ~HttpRequest() = default;
            HttpRequest(const std::string& method, const std::string& url, const std::string& path, const Headers& header) : 
                method_(method),
                url_(url),
                path_(path),
                headrs_(header) {}
            // 访问接口
            const std::string& getMethod() const;
            const std::string getUrl() const;
            const std::string& getPath() const;
            const std::string& getVersion() const;
            const Headers& getheader() const;
            const Paremeters& getParams() const;
            const std::vector<byte>& getBody() const;
            
            // 外部设置接口
            void setMethod(const std::string& method);
            void setUrl(const std::string& url);
            void setVersion(const std::string& version);
            void setheaders(const Headers& header);
            void setParams(const Paremeters& params);
            void addHeaders(const std::string& key, const std::string& value);
            void addParams(const std::string& key, const std::string& value);
            void setBody(const std::vector<byte>& body);
            void addBody(const std::vector<byte>& body);
            void clear();

            std::string getMapString(const std::map<std::string, std::string>& mp) const;
            std::string writeToString() const;
            std::string serialize() const;
            
        private:
            std::string method_;    // http 请求方法
            std::string url_;       // 完整url(包括查询参数)
            std::string path_;      // url 路径
            std::string version_;   // http版本
            Headers headrs_;
            Paremeters params_;
            std::vector<byte> body_;    
        };

    } // end of http
}   // end of yoyo





#endif