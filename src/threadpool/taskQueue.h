#ifndef __THASKQUEUE_H__
#define __THASKQUEUE_H__
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

#include "../utils/util.h"

class taskQueue {
 public:
  taskQueue() : _isStop(false), _iCurrentTaskNum(0), _TaskQueen(200) {}
  ~taskQueue() = default;
  taskQueue(const taskQueue&) = delete;
  taskQueue& operator=(const taskQueue&) = delete;
  taskQueue(taskQueue&&) = delete;
  taskQueue& operator=(taskQueue&&) = delete;

  decltype(auto) consumeTask() {
    std::function<void()> ret;
    {
      std::unique_lock<std::mutex> lock(_mtx);
      _isPopCv.wait(
          lock, [this]() -> bool { return _isStop || !_TaskQueen.empty(); });
      if (_isStop) return ret;
      ret = _TaskQueen.front();
      _TaskQueen.pop();
      _iCurrentTaskNum--;
    };
    _isPushCv.notify_one();
    return ret;
  }

  template <class Func, class... Args>
  auto producerTask(Func&& f, Args&&... args)
      -> std::future<decltype(f(args...))> {
    using RetType = decltype(f(args...));
    std::shared_ptr<std::packaged_task<RetType()>> sTaskPtr =
        std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
    std::future<RetType> ret = sTaskPtr->get_future();
    std::function<void()> wrapper_task = [sTaskPtr]() { (*sTaskPtr)(); };
    {
      std::unique_lock<std::mutex> lock(_mtx);
      _isPushCv.wait(lock,
                     [this]() { return _isStop || !_TaskQueen.isFull(); });
      _TaskQueen.emplace(std::move(wrapper_task));
      _iCurrentTaskNum++;
    };
    _isPopCv.notify_one();
    return ret;
  }

  std::size_t getTaskNum() const { return _iCurrentTaskNum; }
  bool notify_stop() {
    _isStop = true;
    _isPopCv.notify_all();
    return true;
  }

  auto stealTask(std::shared_ptr<taskQueue>& other) -> bool {
    if (!_TaskQueen.empty()) return true;
    if (other->_TaskQueen.empty()) return false;
    std::function<void()> stolen_task;
    {
      std::lock_guard<std::mutex> lock(other->_mtx);
      if (other->_TaskQueen.empty()) {
        other->_isPushCv.notify_one();
        return false;
      }
      stolen_task = std::move(other->_TaskQueen.front());
      other->_TaskQueen.pop();
      other->_iCurrentTaskNum--;
    }
    other->_isPushCv.notify_one();

    // 再给自己的队列上锁
    {
      std::lock_guard<std::mutex> lock(_mtx);
      _TaskQueen.emplace(std::move(stolen_task));
      _iCurrentTaskNum++;
    }
    _isPopCv.notify_one();

    return true;
  }

  // 新的窃取方法：只从本队列安全地“提供”一个任务给窃取者
  std::optional<std::function<void()>> tryOfferSteal() {
    std::unique_lock<std::mutex> lock(_mtx,
                                      std::try_to_lock);  // 尝试获取本队列的锁
    if (!lock.owns_lock() || _TaskQueen.empty()) {
      return std::nullopt;  // 未能获取锁或队列为空，无法提供任务
    }
    std::function<void()> stolen_task = _TaskQueen.front();
    _TaskQueen.pop();
    _iCurrentTaskNum--;
    _isPushCv.notify_one();  // 通知本队列的生产者，队列有空间了
    return stolen_task;
  }

  bool isStopped() const { return _isStop == true; }

 private:
  std::condition_variable _isPopCv;
  std::condition_variable _isPushCv;
  std::mutex _mtx;
  static constexpr std::size_t _iMaxTaskNum = 100;
  std::atomic<bool> _isStop;
  std::size_t _iCurrentTaskNum;

  yoyo::CirculQueen<std::function<void()>> _TaskQueen;
};

#endif
