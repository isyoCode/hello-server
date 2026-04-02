#ifndef __HTTP_FORM_PARSER_H__
#define __HTTP_FORM_PARSER_H__

#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <sstream>

namespace yoyo {
    namespace http {

        /**
         * FormParser - 用于解析表单数据（application/x-www-form-urlencoded）
         *
         * 使用场景：
         * - POST 请求的 Body 参数解析
         * - URL 查询字符串解析
         * - 表单数据提取
         *
         * 示例：
         *   FormParser parser;
         *   parser.parse("username=test&password=123");
         *   std::string username = parser.get("username");  // "test"
         */
        class FormParser {
        public:
            using Parameters = std::map<std::string, std::string>;

            FormParser() = default;
            ~FormParser() = default;

            /**
             * 解析表单数据
             * @param formData 表单字符串，格式: key1=value1&key2=value2&...
             * @return 是否解析成功
             */
            bool parse(const std::string& formData) {
                parameters_.clear();

                if (formData.empty()) {
                    return true;
                }

                size_t start = 0;
                while (start < formData.size()) {
                    // 找到下一个 '&' 分隔符
                    size_t end = formData.find('&', start);
                    if (end == std::string::npos) {
                        end = formData.size();
                    }

                    // 提取 key=value 对
                    std::string pair = formData.substr(start, end - start);
                    if (!pair.empty()) {
                        parsePair(pair);
                    }

                    start = end + 1;
                }

                return true;
            }

            /**
             * 从 byte 向量解析
             * @param data byte 数组
             * @return 是否解析成功
             */
            bool parseFromBytes(const std::vector<unsigned char>& data) {
                std::string str(data.begin(), data.end());
                return parse(str);
            }

            /**
             * 获取参数值
             * @param key 参数名
             * @return 参数值，不存在返回空字符串
             */
            std::string get(const std::string& key) const {
                auto it = parameters_.find(key);
                return (it != parameters_.end()) ? it->second : "";
            }

            /**
             * 获取参数值（带默认值）
             * @param key 参数名
             * @param defaultValue 默认值
             * @return 参数值或默认值
             */
            std::string get(const std::string& key, const std::string& defaultValue) const {
                auto it = parameters_.find(key);
                return (it != parameters_.end()) ? it->second : defaultValue;
            }

            /**
             * 检查参数是否存在
             */
            bool has(const std::string& key) const {
                return parameters_.find(key) != parameters_.end();
            }

            /**
             * 获取所有参数
             */
            const Parameters& getAll() const {
                return parameters_;
            }

            /**
             * 获取参数个数
             */
            size_t size() const {
                return parameters_.size();
            }

            /**
             * 清空所有参数
             */
            void clear() {
                parameters_.clear();
            }

            /**
             * 添加或更新参数
             */
            void set(const std::string& key, const std::string& value) {
                parameters_[key] = value;
            }

            /**
             * 获取参数值为整数
             * @param key 参数名
             * @param defaultValue 默认值
             * @return 转换后的整数
             */
            int getInt(const std::string& key, int defaultValue = 0) const {
                try {
                    const auto& value = get(key);
                    return value.empty() ? defaultValue : std::stoi(value);
                } catch (...) {
                    return defaultValue;
                }
            }

            /**
             * 获取参数值为浮点数
             */
            double getDouble(const std::string& key, double defaultValue = 0.0) const {
                try {
                    const auto& value = get(key);
                    return value.empty() ? defaultValue : std::stod(value);
                } catch (...) {
                    return defaultValue;
                }
            }

            /**
             * 获取参数值为布尔值
             */
            bool getBool(const std::string& key, bool defaultValue = false) const {
                const auto& value = get(key);
                if (value.empty()) return defaultValue;

                std::string lower = value;
                for (auto& c : lower) c = std::tolower(static_cast<unsigned char>(c));

                return (lower == "true" || lower == "1" || lower == "yes" || lower == "on");
            }

            /**
             * 输出所有参数（调试用）
             */
            std::string toString() const {
                std::ostringstream oss;
                oss << "FormParser {";
                bool first = true;
                for (const auto& [key, value] : parameters_) {
                    if (!first) oss << ", ";
                    oss << "\"" << key << "\":\"" << value << "\"";
                    first = false;
                }
                oss << "}";
                return oss.str();
            }

        private:
            /**
             * 解析单个 key=value 对
             */
            void parsePair(const std::string& pair) {
                size_t eqPos = pair.find('=');
                if (eqPos == std::string::npos) {
                    // 没有 '=' 符号，整个 pair 作为 key
                    parameters_[urlDecode(pair)] = "";
                    return;
                }

                std::string key = pair.substr(0, eqPos);
                std::string value = pair.substr(eqPos + 1);

                parameters_[urlDecode(key)] = urlDecode(value);
            }

            /**
             * URL 解码
             * 处理两种编码格式：
             * - '+' 表示空格
             * - '%HH' 表示十六进制编码的字符
             *
             * 示例：
             *   "hello+world" → "hello world"
             *   "test%40email.com" → "test@email.com"
             */
            static std::string urlDecode(const std::string& str) {
                std::string decoded;
                decoded.reserve(str.length());

                for (size_t i = 0; i < str.length(); ++i) {
                    char c = str[i];

                    if (c == '+') {
                        // '+' 解码为空格
                        decoded += ' ';
                    } else if (c == '%' && i + 2 < str.length()) {
                        // '%HH' 解码为对应的字符
                        char hex1 = str[i + 1];
                        char hex2 = str[i + 2];

                        if (std::isxdigit(static_cast<unsigned char>(hex1)) &&
                            std::isxdigit(static_cast<unsigned char>(hex2))) {

                            // 转换十六进制为字符
                            int hexValue = (hexCharToInt(hex1) << 4) | hexCharToInt(hex2);
                            decoded += static_cast<char>(hexValue);
                            i += 2;  // 跳过两个十六进制字符
                        } else {
                            // 无效的 % 编码，保留原样
                            decoded += c;
                        }
                    } else {
                        decoded += c;
                    }
                }

                return decoded;
            }

            /**
             * 十六进制字符转整数
             */
            static int hexCharToInt(char c) {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return 0;
            }

            Parameters parameters_;
        };

    } // end of http
}   // end of yoyo

#endif // __HTTP_FORM_PARSER_H__
