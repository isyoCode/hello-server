#include "FormParser.h"
#include <iostream>

using namespace yoyo::http;

/**
 * FormParser 使用示例
 *
 * 编译：
 *   g++ -std=c++17 FormParserExample.cc -o form_parser_example
 *
 * 运行：
 *   ./form_parser_example
 */

int main() {
    std::cout << "=== FormParser 使用示例 ===" << std::endl << std::endl;

    // 示例 1：登录表单解析
    {
        std::cout << "【示例 1】登录表单解析" << std::endl;
        FormParser parser;
        parser.parse("username=test&password=123");

        std::cout << "原始数据: username=test&password=123" << std::endl;
        std::cout << "username: " << parser.get("username") << std::endl;
        std::cout << "password: " << parser.get("password") << std::endl;
        std::cout << std::endl;
    }

    // 示例 2：URL编码处理
    {
        std::cout << "【示例 2】URL编码处理" << std::endl;
        FormParser parser;
        parser.parse("email=user%40example.com&name=John+Doe");

        std::cout << "原始数据: email=user%40example.com&name=John+Doe" << std::endl;
        std::cout << "email: " << parser.get("email") << std::endl;
        std::cout << "name: " << parser.get("name") << std::endl;
        std::cout << std::endl;
    }

    // 示例 3：注册表单
    {
        std::cout << "【示例 3】注册表单" << std::endl;
        FormParser parser;
        parser.parse(
            "username=alice&email=alice%40example.com&password=Pass%40123&confirm_password=Pass%40123"
        );

        std::cout << "解析后的参数：" << std::endl;
        for (const auto& [key, value] : parser.getAll()) {
            std::cout << "  " << key << " = " << value << std::endl;
        }
        std::cout << std::endl;
    }

    // 示例 4：从字节向量解析
    {
        std::cout << "【示例 4】从字节向量解析" << std::endl;
        std::vector<unsigned char> bodyBytes = {
            'u', 's', 'e', 'r', 'n', 'a', 'm', 'e', '=', 't', 'e', 's', 't',
            '&', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '=', '1', '2', '3'
        };

        FormParser parser;
        parser.parseFromBytes(bodyBytes);

        std::cout << "从字节向量解析: ";
        std::cout << parser.toString() << std::endl;
        std::cout << std::endl;
    }

    // 示例 5：使用默认值和类型转换
    {
        std::cout << "【示例 5】类型转换和默认值" << std::endl;
        FormParser parser;
        parser.parse("id=42&score=98.5&active=true&name=test");

        std::cout << "id (int): " << parser.getInt("id") << std::endl;
        std::cout << "age (int, 默认18): " << parser.getInt("age", 18) << std::endl;
        std::cout << "score (double): " << parser.getDouble("score") << std::endl;
        std::cout << "active (bool): " << (parser.getBool("active") ? "true" : "false") << std::endl;
        std::cout << "name (string, 默认'unknown'): " << parser.get("name", "unknown") << std::endl;
        std::cout << "nickname (string, 默认'unknown'): " << parser.get("nickname", "unknown") << std::endl;
        std::cout << std::endl;
    }

    // 示例 6：检查参数是否存在
    {
        std::cout << "【示例 6】检查参数存在性" << std::endl;
        FormParser parser;
        parser.parse("username=test&password=123");

        std::cout << "has(username): " << (parser.has("username") ? "true" : "false") << std::endl;
        std::cout << "has(email): " << (parser.has("email") ? "true" : "false") << std::endl;
        std::cout << "size: " << parser.size() << std::endl;
        std::cout << std::endl;
    }

    // 示例 7：实际的登录验证流程
    {
        std::cout << "【示例 7】实际登录验证流程" << std::endl;
        FormParser parser;
        parser.parse("username=test&password=123");

        // 获取参数
        std::string username = parser.get("username");
        std::string password = parser.get("password");

        // 验证
        if (username.empty()) {
            std::cout << "❌ 用户名不能为空" << std::endl;
        } else if (password.empty()) {
            std::cout << "❌ 密码不能为空" << std::endl;
        } else if (username.length() < 3) {
            std::cout << "❌ 用户名至少3个字符" << std::endl;
        } else if (password.length() < 6) {
            std::cout << "❌ 密码至少6个字符" << std::endl;
        } else if (username == "test" && password == "123") {
            std::cout << "✅ 登录成功！欢迎 " << username << std::endl;
        } else {
            std::cout << "❌ 用户名或密码错误" << std::endl;
        }
        std::cout << std::endl;
    }

    // 示例 8：特殊字符处理
    {
        std::cout << "【示例 8】特殊字符处理" << std::endl;
        FormParser parser;
        // 常见的URL编码特殊字符
        parser.parse(
            "name=John+Doe&"
            "email=john.doe%40example.com&"
            "message=Hello%2C+World%21&"
            "url=https%3A%2F%2Fexample.com%2Fpath%3Fq%3D123"
        );

        std::cout << "name: " << parser.get("name") << std::endl;
        std::cout << "email: " << parser.get("email") << std::endl;
        std::cout << "message: " << parser.get("message") << std::endl;
        std::cout << "url: " << parser.get("url") << std::endl;
        std::cout << std::endl;
    }

    std::cout << "=== 示例结束 ===" << std::endl;

    return 0;
}
