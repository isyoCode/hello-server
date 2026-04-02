#ifndef __MYUTIL_H__
#define __MYUTIL_H__
/*
    提供部分常用工具方法
 */
#include <chrono>
#include <exception>
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>
#include <vector>
namespace yoyo {

// Exception class for error handling
class Exception : public std::exception {
 public:
  Exception(const std::string& sMessage, int iCode = -1)
      : sErrMsg_(sMessage), iErrCode_(iCode) {}
  Exception(std::string&& sMessage, int iCode = -1)
      : sErrMsg_(std::move(sMessage)), iErrCode_(iCode) {}
  ~Exception() = default;
  Exception() = default;

  const char* what() const noexcept override { return sErrMsg_.c_str(); }

  std::string_view getErrMsg() const noexcept { return sErrMsg_; }
  int getErrCode() const noexcept { return iErrCode_; }

 private:
  std::string sErrMsg_;
  int iErrCode_ = -1;
};

// Error checking macro
#define ERR_CHECK(expression, msg) \
  do {                             \
    if (expression) {              \
      throw yoyo::Exception(msg);  \
    }                              \
  } while (0)

/* 封装一些chrono库中的用法*/
#define __now__ std::chrono::high_resolution_clock::now()
using _Ms_ = std::chrono::milliseconds;
using _Second_ = std::chrono::seconds;
#define SLEEP(Time)            \
  std::this_thread::sleep_for( \
      std::chrono::milliseconds(static_cast<int>(Time * 1000)))
static inline std::string GETTYPE(_Ms_) { return " ms"; }
static inline std::string GETTYPE(_Second_) { return " s"; }
#define _TIMECOUNT_(Function, timeType, Desc)                          \
  do {                                                                 \
    auto start = __now__;                                              \
    Function;                                                          \
    auto end = __now__;                                                \
    auto duration = std::chrono::duration_cast<timeType>(end - start); \
    std::cout << Desc << " cousmeTime: " << duration.count()           \
              << yoyo::GETTYPE(timeType()) << std::endl;               \
  } while (0);

static inline std::string getCurrentTime() {
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch());
  auto sectime = std::chrono::duration_cast<std::chrono::seconds>(now_ms);
  int32_t milltime = now_ms.count() % 1000;

  std::time_t timet = sectime.count();
  struct tm curtime;
  localtime_r(&timet, &curtime);

  char buffer[100];
  snprintf(buffer, sizeof(buffer), "%4d-%02d-%02d %02d:%02d:%02d.%03d",
           curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday,
           curtime.tm_hour, curtime.tm_min, curtime.tm_sec, milltime);
  return std::string(buffer);
}

/*单例模式 */
template <class T>
class Singleton {
 public:
  static T* getInstance();

 private:
  static T* data_;
  static std::mutex mtx;
  Singleton(const Singleton&) = delete;

 protected:
  Singleton() = default;
  ~Singleton() = default;
};
template <class T>
inline T* Singleton<T>::data_ = nullptr;
template <class T>
inline std::mutex Singleton<T>::mtx;
template <class T>
T* Singleton<T>::getInstance() {
  if (data_ == nullptr) {
    std::lock_guard<std::mutex> lock(mtx);
    if (data_ == nullptr) {
      data_ = new T;
    }
  }
  return data_;
}

// circulBuffer
template <class T>
class CirculQueen {
 public:
  CirculQueen() = default;
  /*留个空位作为 空/满的标识位置 */
  explicit CirculQueen(size_t maxSize)
      : _iMaxItem(maxSize + 1), _vQueen(_iMaxItem) {}
  CirculQueen(const CirculQueen&) = default;
  CirculQueen& operator=(const CirculQueen&) = default;
  CirculQueen(CirculQueen&& other) { rvalueCtor(std::move(other)); }
  CirculQueen& operator=(CirculQueen&& other) {
    rvalueCtor(std::move(other));
    return *this;
  }
  ~CirculQueen() = default;
  size_t getNum() const { return (_Tail + _iMaxItem - _Head) % _iMaxItem; }
  void setMaxSize(size_t maxSize) {
    _iMaxItem = maxSize + 1;
    _vQueen.resize(_iMaxItem);
  }
  bool isFull() const { return (_Tail + 1) % _iMaxItem == _Head; }

  bool empty() const { return _Tail == _Head; }

  bool emplace(T&& item) {
    if (isFull()) return false;
    _vQueen[_Tail] = std::move(item);
    _Tail = (_Tail + 1) % _iMaxItem;
    return true;
  }
  T front() const { return std::move(_vQueen[_Head]); }

  void pop() { _Head = (_Head + 1) % _iMaxItem; }

 private:
  void rvalueCtor(CirculQueen&& other) {
    _iMaxItem = other._iMaxItem;
    _Head = other._Head;
    _Tail = other._Tail;
    _vQueen = std::move(other._vQueen);

    other._iMaxItem = 0;
    other._Head = 0;
    other._Tail = 0;
    other._iOverCount = 0;
  }

 private:
  std::size_t _iMaxItem;
  std::vector<T> _vQueen;
  typename std::vector<T>::size_type _Head = 0;
  typename std::vector<T>::size_type _Tail = 0;
};

}  // end of namespace yoyo

#endif
