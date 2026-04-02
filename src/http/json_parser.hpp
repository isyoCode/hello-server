#ifndef __YOYO_JSON_PARSER_HPP__
#define __YOYO_JSON_PARSER_HPP__
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace yoyo {

enum class JSONTYPE {
  JSON_ARRAY,    // array
  JSON_NULL,     // NULL
  JSON_NUMBER,   // 数字 -, e, 0-9
  JSON_DOUBLE,   // 浮点数字
  JSON_STRING,   // string
  JSON_BOOLEAN,  // false/true
  JSON_OBJECT    // JSON类型(嵌套对象 键值对)
};

using jValueType = JSONTYPE;
// defin jsonFiledObject

class JsonFiled {
 public:
  using json_null = std::monostate;
  using json_int = int32_t;
  using json_bool = bool;
  using json_double = double;
  using json_string = std::string;
  using json_array = std::vector<JsonFiled>;
#if defined(__clang__)  // 如果是 Clang 编译器
  using json_object = std::unordered_map<std::string, JsonFiled>;
#elif defined(__GNUC__)  // 如果是 GCC 编译器
  using json_object = std::map<std::string, JsonFiled>;
#else                    // 对于其他编译器，如 MSVC
  using json_object = std::unordered_map<std::string, JsonFiled>;
#endif
  using jsonValue = std::variant<json_null, json_int, json_bool, json_double,
                                 json_string, json_array, json_object>;

 public:
  JsonFiled() {
    _jType = JSONTYPE::JSON_NULL;
    _jValue = std::monostate{};
  }
  JsonFiled(bool b_value) {
    _jType = JSONTYPE::JSON_BOOLEAN;
    _jValue = b_value;
  }
  JsonFiled(int num) {
    _jType = JSONTYPE::JSON_NUMBER;
    _jValue = num;
  }
  JsonFiled(double num) {
    _jType = JSONTYPE::JSON_DOUBLE;
    _jValue = num;
  }
  JsonFiled(std::string&& str) {
    _jType = JSONTYPE::JSON_STRING;
    _jValue = std::move(str);
  }
  JsonFiled(const std::string& str) {
    _jType = JSONTYPE::JSON_STRING;
    _jValue = str;
  }
  JsonFiled(const std::vector<JsonFiled>& value) {
    _jType = JSONTYPE::JSON_ARRAY;
    _jValue = value;
  }
  JsonFiled(std::vector<JsonFiled>&& value) {
    _jType = JSONTYPE::JSON_ARRAY;
    _jValue = std::move(value);
  }
  JsonFiled(const json_object& value) {
    _jType = JSONTYPE::JSON_OBJECT;
    _jValue = value;
  }
  JsonFiled(json_object&& value) {
    _jType = JSONTYPE::JSON_OBJECT;
    _jValue = std::move(value);
  }

  JsonFiled(const JsonFiled& other) {
    _jType = other._jType;
    _jValue = other._jValue;
  }
  JsonFiled& operator=(const JsonFiled& other) {
    if (this != &other) {
      _jType = other._jType;
      _jValue = other._jValue;
    }
    return *this;
  }
  JsonFiled(JsonFiled&& other) {
    _jType = other._jType;
    _jValue = std::move(other._jValue);
  }
  JsonFiled& operator=(JsonFiled&& other) {
    if (this != &other) {
      _jType = other._jType;
      _jValue = std::move(other._jValue);
    }
    return *this;
  }
  ~JsonFiled() = default;

 public:
  // get value
  template <class T>
  T get() const {
    if constexpr (std::is_same_v<T, json_null>) {
      return std::get<json_null>(_jValue);
    } else if constexpr (std::is_same_v<T, json_int>) {
      return std::get<json_int>(_jValue);
    } else if constexpr (std::is_same_v<T, json_bool>) {
      return std::get<json_bool>(_jValue);
    } else if constexpr (std::is_same_v<T, json_double>) {
      return std::get<json_double>(_jValue);
    } else if constexpr (std::is_same_v<T, json_string>) {
      return std::get<json_string>(_jValue);
    } else if constexpr (std::is_same_v<T, json_array>) {
      return std::get<json_array>(_jValue);
    } else if constexpr (std::is_same_v<T, json_object>) {
      return std::get<json_object>(_jValue);
    } else {
      throw std::logic_error("invalid type");
    }
  }

  // get type
  JSONTYPE getType() const { return _jType; }

  bool isBool() const noexcept { return getType() == JSONTYPE::JSON_BOOLEAN; }
  bool isInt() const noexcept { return getType() == JSONTYPE::JSON_NUMBER; }
  bool isDouble() const noexcept { return getType() == JSONTYPE::JSON_DOUBLE; }
  bool isArray() const noexcept { return getType() == JSONTYPE::JSON_ARRAY; }
  bool isObject() const noexcept { return getType() == JSONTYPE::JSON_OBJECT; }
  bool isString() const noexcept { return getType() == JSONTYPE::JSON_STRING; }
  bool isNull() const noexcept { return getType() == JSONTYPE::JSON_NULL; }

  json_int asInt() const { return static_cast<json_int>(get<json_int>()); }
  json_double asDouble() const {
    return static_cast<json_double>(get<json_double>());
  }
  json_bool asBool() const { return static_cast<json_bool>(get<json_bool>()); }
  json_string asString() const {
    return static_cast<json_string>(get<json_string>());
  }
  json_array asArray() const {
    return static_cast<json_array>(get<json_array>());
  }
  json_object asObject() const {
    return static_cast<json_object>(get<json_object>());
  }
  template <class T>
  bool isMember(T&& key) const {
    if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                  std::is_same_v<std::decay_t<T>, const char*> ||
                  std::is_same_v<std::decay_t<T>, std::string_view>) {
      if (!isObject()) return false;
      const json_object& tObject = get<json_object>();
      return static_cast<bool>(tObject.find(std::forward<T>(key)) !=
                               tObject.end());
    } else {
      return false;
    }
  }

  // 重载类型转换操作符
  operator int() const {
    if (!isInt())
      throw std::logic_error("Cannot convert to json_int, invalid type");
    return get<int>();
  }
  operator std::string() const {
    if (!isString())
      throw std::logic_error("Cannot convert to json_string, invalid type");
    return get<std::string>();
  }
  operator double() const {
    if (!isDouble())
      throw std::logic_error("Cannot convert to json_double, invalid type");
    return get<double>();
  }
  operator bool() const {
    if (!isBool())
      throw std::logic_error("Cannot convert to json_bool, invalid type");
    return get<bool>();
  }
  operator json_array() const {
    if (!isArray())
      throw std::logic_error("Cannot convert to json_array, invalid type");
    return get<json_array>();
  }
  operator json_object() const {
    if (!isObject())
      throw std::logic_error("Cannot convert to json_object, invalid type");
    return get<json_object>();
  }

  template <typename T>
  bool operator==(const T& value) const {
    if constexpr (std::is_same_v<T, std::string>) {
      return isString() && get<json_string>() == value;
    } else if constexpr (std::is_same_v<T, const char*>) {
      return isString() && get<json_string>() == std::string(value);
    } else if constexpr (std::is_same_v<T, int>) {
      return isInt() && get<json_int>() == value;
    } else if constexpr (std::is_same_v<T, double>) {
      return isDouble() && get<json_double>() == value;
    } else if constexpr (std::is_same_v<T, bool>) {
      return isBool() && get<json_bool>() == value;
    } else {
      return false;
    }
  }

  template <typename T>
  bool operator!=(const T& value) const {
    return !(*this == value);
  }

  size_t size() const {
    if (!isArray() && !isObject())
      throw std::logic_error("Cannot get size, invalid type");
    if (isObject()) return get<json_object>().size();
    return get<json_array>().size();
  }
  bool isEmpty() const {
    if (isObject()) return get<json_object>().empty();
    if (isArray()) return get<json_array>().empty();
    return false;
  }
  JsonFiled& operator[](const std::string& sKey) {
    if (!isObject())
      throw std::logic_error("Current object is not k-v obj, invalid type");
    json_object& tObj = std::get<json_object>(_jValue);
    return tObj[sKey];
  }

  JsonFiled& operator[](size_t index) {
    if (!isArray())
      throw std::logic_error("Current obj is not a array, invalid index type");
    json_array& tArray = std::get<json_array>(_jValue);
    if (index >= tArray.size())
      throw std::logic_error("Index out of range for JSON array.");
    return tArray[index];
  }

  template <typename T>
  JsonFiled& operator[](T key) {
    static_assert(std::is_same_v<T, std::string> ||
                      std::is_same_v<T, const char*> || std::is_integral_v<T>,
                  "Unsupported key type for JsonFiled operator[]");

    if constexpr (std::is_integral_v<T>) {
      return (*this)[static_cast<size_t>(key)];  // 如果是整数，调用数组索引重载
    } else {
      return (*this)[std::string(key)];  // 如果是字符串，调用对象键值重载
    }
  }

  void push_back(JsonFiled obj) {
    if (!isArray())
      throw std::logic_error("Current object is not array, invalid type");
    json_array& tArray = std::get<json_array>(_jValue);
    tArray.emplace_back(JsonFiled(std::move(obj)));
  }

 public:
  std::string writeToString() const {
    switch (_jType) {
      case JSONTYPE::JSON_NULL:
        return "null";
      case JSONTYPE::JSON_BOOLEAN:
        return std::get<json_bool>(_jValue) ? "true" : "false";
      case JSONTYPE::JSON_NUMBER:
        return std::to_string(std::get<json_int>(_jValue));
      case JSONTYPE::JSON_DOUBLE:
        return std::to_string(std::get<json_double>(_jValue));
      case JSONTYPE::JSON_STRING:
        return "\"" + escapeString(std::get<json_string>(_jValue)) + "\"";
      case JSONTYPE::JSON_ARRAY:
        return arrayToString(std::get<json_array>(_jValue));
      case JSONTYPE::JSON_OBJECT:
        return objectToString(std::get<json_object>(_jValue));
      default:
        throw std::logic_error("Unknown JSON type");
    }
  }

 private:
  std::string escapeString(const std::string& str) const {
    std::string result;
    for (const char c : str) {
      switch (c) {
        case '\"':
          result += "\\\"";
          break;
        case '\\':
          result += "\\\\";
          break;
        case '\n':
          result += "\\n";
          break;
        case '\t':
          result += "\\t";
          break;
        case '\b':
          result += "\\b";
          break;
        case '\f':
          result += "\\f";
          break;
        case '\r':
          result += "\\r";
          break;
        default:
          if (c >= 0 && c <= 0x1F) {  // 控制字符需要转义为 \uXXXX
            result += "\\u" + intToHex(static_cast<unsigned char>(c), 4);
          } else {
            result += c;
          }
      }
    }
    return result;
  }
  std::string intToHex(unsigned int value, int width) const {
    std::ostringstream oss;
    oss << std::hex << std::setw(width) << std::setfill('0') << value;
    return oss.str();
  }

  std::string arrayToString(const json_array& array) const {
    std::string result = "[";
    bool first = true;
    for (const auto& element : array) {
      if (!first) {
        result += ",";
      }
      result += element.writeToString();
      first = false;
    }
    result += "]";
    return result;
  }
  std::string objectToString(const json_object& obj) const {
    std::string result = "{";
    bool first = true;
    for (const auto& pair : obj) {
      if (!first) {
        result += ",";
      }
      result +=
          "\"" + escapeString(pair.first) + "\":" + pair.second.writeToString();
      first = false;
    }
    result += "}";
    return result;
  }
  friend std::ostream& operator<<(std::ostream& os, const JsonFiled& jsonField);

 private:
  jValueType _jType;
  jsonValue _jValue;
};

class JsonParseError : public std::exception {
 public:
  JsonParseError(const std::string& msg, size_t index)
      : _msg("JSON Parse Error at index " + std::to_string(index) + ": " + msg),
        _index(index) {}
  const char* what() const noexcept override { return _msg.c_str(); }
  size_t getErrorIndex() const { return _index; }

 private:
  std::string _msg;
  size_t _index;
};

class JsonParser {
 public:
  explicit JsonParser(const std::string& jsonstr)
      : _jsonstring(jsonstr), _iIndex(0) {
    if (_jsonstring.empty()) {
      throw std::logic_error("input JSON string is empty");
    }
  }
  JsonParser(const JsonParser& other) {
    _jsonstring = other._jsonstring;
    _iIndex = other._iIndex;
  }
  JsonParser& operator=(const JsonParser& other) {
    if (this != &other) {
      _jsonstring = other._jsonstring;
      _iIndex = other._iIndex;
    }
    return *this;
  }
  JsonParser(JsonParser&& other) {
    _jsonstring = std::move(other._jsonstring);
    _iIndex = other._iIndex;
  }
  JsonParser& operator=(JsonParser&& other) {
    if (this != &other) {
      _jsonstring = std::move(other._jsonstring);
      _iIndex = other._iIndex;
    }
    return *this;
  }
  ~JsonParser() = default;

 public:
  JsonFiled parser(size_t CurrentDepth = 0) {
    if (CurrentDepth > _iMaxDepth) {
      throw JsonParseError("Maximum JSON depth exceeded", CurrentDepth);
    }

    char sToken = getNextToken();

    // 根据当前字符分辨 JSON 类型
    if (sToken == 'n') return parseNull();                      // null
    if (sToken == 't' || sToken == 'f') return parseBoolean();  // true/false
    if (sToken == '-' || std::isdigit(sToken)) return parseNumber();  // 数字
    if (sToken == '\"') return parseString();                 // 字符串
    if (sToken == '[') return parseArray(CurrentDepth + 1);   // 数组
    if (sToken == '{') return parseObject(CurrentDepth + 1);  // 对象
    throw JsonParseError(std::string("Invalid JSON character: ") + sToken,
                         CurrentDepth);
  }

 private:
  char getNextToken() {
    while (std::isspace(_jsonstring[_iIndex])) {
      _iIndex++;
    }
    if (_iIndex >= _jsonstring.size()) {
      throw std::logic_error("unexpected character in parse json");
    }
    return _jsonstring[_iIndex];
  }

  JsonFiled parseNull() {
    // 直接比较 jsonstring 从 index 开始的 4 个字符是否为 "null"
    if (_jsonstring.compare(_iIndex, 4, "null") == 0) {
      _iIndex += 4;  // 移动索引，跳过 "null"
      return JsonFiled();
    }
    throw std::logic_error("parse null error");
  }
  JsonFiled parseBoolean() {
    // 比较 jsonstring 从 index 开始的 4 个字符是否为 "true"
    if (_jsonstring.compare(_iIndex, 4, "true") == 0) {
      _iIndex += 4;  // 移动索引，跳过 "true"
      return JsonFiled(true);
    }
    // 比较 jsonstring 从 index 开始的 5 个字符是否为 "false"
    if (_jsonstring.compare(_iIndex, 5, "false") == 0) {
      _iIndex += 5;  // 移动索引，跳过 "false"
      return JsonFiled(false);
    }
    throw std::logic_error("parse bool error");
  }

  JsonFiled parseNumber() {
    int pos = _iIndex;                           // 记录起始位置
    if (_jsonstring[_iIndex] == '-') _iIndex++;  // 跳过负号

    // 整数部分
    if (_iIndex >= _jsonstring.size() || !std::isdigit(_jsonstring[_iIndex])) {
      throw std::logic_error("Invalid character in number");
    }
    while (_iIndex < _jsonstring.size() && std::isdigit(_jsonstring[_iIndex])) {
      _iIndex++;
    }
    // 小数部分
    if (_iIndex < _jsonstring.size() && _jsonstring[_iIndex] == '.') {
      _iIndex++;
      if (_iIndex >= _jsonstring.size() ||
          !std::isdigit(_jsonstring[_iIndex])) {
        throw std::logic_error(
            "At least one digit required after decimal point");
      }
      while (_iIndex < _jsonstring.size() &&
             std::isdigit(_jsonstring[_iIndex])) {
        _iIndex++;
      }
    }
    // 科学计数法部分
    if (_iIndex < _jsonstring.size() &&
        (_jsonstring[_iIndex] == 'e' || _jsonstring[_iIndex] == 'E')) {
      _iIndex++;
      if (_iIndex < _jsonstring.size() &&
          (_jsonstring[_iIndex] == '+' || _jsonstring[_iIndex] == '-')) {
        _iIndex++;
      }
      if (_iIndex >= _jsonstring.size() ||
          !std::isdigit(_jsonstring[_iIndex])) {
        throw std::logic_error(
            "At least one digit required in scientific notation exponent");
      }
      while (_iIndex < _jsonstring.size() &&
             std::isdigit(_jsonstring[_iIndex])) {
        _iIndex++;
      }
    }
    // 解析部分完成后进行最终转换
    std::string_view numberString{_jsonstring.c_str() + pos,
                                  static_cast<size_t>(_iIndex - pos)};
    if (numberString.find('.') != std::string_view::npos ||
        numberString.find('e') != std::string_view::npos ||
        numberString.find('E') != std::string_view::npos) {
      return JsonFiled(std::stof(std::string(numberString)));  // 浮点数
    } else {
      return JsonFiled(std::stoi(std::string(numberString)));  // 整数
    }
  }

  JsonFiled parseString() {
    std::string str;
    _iIndex++;  // 跳过起始引号 '\"'
    while (_iIndex < _jsonstring.size() && _jsonstring[_iIndex] != '\"') {
      if (_jsonstring[_iIndex] == '\\') {  // 检测转义字符
        _iIndex++;
        if (_iIndex >= _jsonstring.size()) {
          throw std::logic_error("unterminated string in JSON");
        }
        switch (_jsonstring[_iIndex]) {
          case 'n':
            str += '\n';
            break;
          case 't':
            str += '\t';
            break;
          case 'r':
            str += '\r';
            break;
          case '\"':
            str += '\"';
            break;
          case '\\':
            str += '\\';
            break;
          default:
            throw std::logic_error("invalid escape character in string");
        }
      } else {
        str += _jsonstring[_iIndex];
      }
      _iIndex++;
    }
    if (_jsonstring[_iIndex] != '\"') {
      throw std::logic_error("unterminated string in JSON");
    }
    _iIndex++;  // 跳过最后的引号 '\"'
    return JsonFiled(std::move(str));
  }

  JsonFiled parseArray(size_t CurrentDepth) {
    JsonFiled::json_array vJvalue;
    _iIndex++;  // 跳过 '['
    if (_iIndex >= _jsonstring.size()) {
      throw JsonParseError("Unexpected end of input during array parsing",
                           CurrentDepth);
    }
    if (_jsonstring[_iIndex] == ']') {
      _iIndex++;                             // 跳过 ']'
      return JsonFiled(std::move(vJvalue));  // 空数组
    }
    while (true) {
      if (_iIndex >= _jsonstring.size()) {
        throw JsonParseError("Unexpected end of input during array parsing",
                             CurrentDepth);
      }
      vJvalue.push_back(std::move(parser(CurrentDepth + 1)));
      char ch = getNextToken();
      if (ch == ']') {
        _iIndex++;  // 跳过 ']'
        return JsonFiled(std::move(vJvalue));
      }
      if (ch != ',') {
        throw JsonParseError(
            "Expected ',' or ']' in array, but found: " + std::string(1, ch),
            CurrentDepth);
      }
      _iIndex++;  // 跳过 ',' 分隔符
    }
  }

  JsonFiled parseObject(size_t currentDepth) {
    JsonFiled::json_object mJvalue;
    _iIndex++;  // 跳过 '{'
    if (_iIndex >= _jsonstring.size()) {
      throw JsonParseError("Unexpected end of input during object parsing",
                           currentDepth);
    }
    if (getNextToken() == '}') {
      _iIndex++;                             // 跳过 '}'
      return JsonFiled(std::move(mJvalue));  // 空对象
    }
    while (true) {
      if (_iIndex >= _jsonstring.size()) {
        throw JsonParseError("Unexpected end of input during object parsing",
                             currentDepth);
      }
      std::string sKey =
          std::move(parser(currentDepth + 1).get<JsonFiled::json_string>());
      char ch = getNextToken();
      if (ch != ':') {
        throw JsonParseError(
            "Expected ':' in object, but found: " + std::string(1, ch),
            currentDepth);
      }
      _iIndex++;  // 跳过 ':'
      if (mJvalue.find(sKey) != mJvalue.end()) {
        throw JsonParseError("duplicate key in JSON object", currentDepth);
      }
      mJvalue[sKey] = parser(currentDepth + 1);
      ch = getNextToken();
      if (ch == '}') {
        _iIndex++;  // 跳过 '}'
        return JsonFiled(std::move(mJvalue));
      }
      if (ch != ',') {
        throw JsonParseError(
            "Expected ',' or '}' in object, but found: " + std::string(1, ch),
            currentDepth);
      }
      _iIndex++;  // 跳过 ',' 分隔符
    }
  }

 private:
  std::string _jsonstring;
  size_t _iIndex;
  constexpr static size_t _iMaxDepth{64};  // 暂定写死
};

// 重载输出流操作符 friend std::ostream& operator<<(std::ostream& os, const
// JsonFiled& jsonField);
inline std::ostream& operator<<(std::ostream& os, const JsonFiled& jsonField) {
  os << jsonField.writeToString();
  return os;
}

// 封装一个解析方法
using JsonValue = JsonFiled;
inline JsonValue parserJson(const std::string& jsonstr) {
  JsonParser parser(jsonstr);
  return parser.parser();
}

// 一个输出所有解析完成后的json字段的方法
void PrintJson(const yoyo::JsonFiled& jsonField, int indent = 0) {
  // 打印缩进
  auto printIndent = [&]() {
    for (int i = 0; i < indent; ++i) std::cout << "  ";
  };
  // 识别 JSON 值的类型
  switch (jsonField.getType()) {
    case yoyo::JSONTYPE::JSON_NULL:
      printIndent();
      std::cout << "null" << std::endl;
      break;
    case yoyo::JSONTYPE::JSON_BOOLEAN:
      printIndent();
      std::cout << (jsonField.get<yoyo::JsonFiled::json_bool>() ? "true"
                                                                : "false")
                << std::endl;
      break;
    case yoyo::JSONTYPE::JSON_NUMBER:
      printIndent();
      std::cout << jsonField.get<yoyo::JsonFiled::json_int>() << std::endl;
      break;
    case yoyo::JSONTYPE::JSON_DOUBLE:
      printIndent();
      std::cout << jsonField.get<yoyo::JsonFiled::json_double>() << std::endl;
      break;
    case yoyo::JSONTYPE::JSON_STRING:
      printIndent();
      std::cout << "\"" << jsonField.get<yoyo::JsonFiled::json_string>() << "\""
                << std::endl;
      break;
    case yoyo::JSONTYPE::JSON_ARRAY: {
      printIndent();
      std::cout << "[" << std::endl;
      for (const auto& item : jsonField.get<yoyo::JsonFiled::json_array>()) {
        PrintJson(item, indent + 1);  // 递归打印数组的每个元素
      }
      printIndent();
      std::cout << "]" << std::endl;
      break;
    }
    case yoyo::JSONTYPE::JSON_OBJECT: {
      printIndent();
      std::cout << "{" << std::endl;
      for (const auto& [key, value] :
           jsonField.get<yoyo::JsonFiled::json_object>()) {
        printIndent();
        std::cout << "\"" << key << "\": ";
        PrintJson(value, indent + 1);  // 递归打印对象的每个字段
      }
      printIndent();
      std::cout << "}" << std::endl;
      break;
    }
    default:
      throw std::logic_error("Unknown JSON type.");
  }
}

}  // namespace yoyo

#endif  // __JSON_PARSER_HPP__