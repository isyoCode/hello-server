#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__
#include <cstddef>
#include <functional>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

#include "taskQueue.h"
/* Implement a thread pool */

class ThreadPool {
 public:
  explicit ThreadPool() : _iThreadNum(10), _isStop(false), _iIndex(0) {}

  explicit ThreadPool(std::size_t threadNum);

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  ~ThreadPool();

  void worker(int queueid);
  void start();
  void stop();
  template <class Func, class... Args>
  auto submitTask(Func&& func, Args&&... args)
      -> std::future<decltype(func(args...))>;

 private:
  std::size_t _iThreadNum;
  std::vector<std::thread> _vWorkerThreads;
  // 将线程和消息队列绑定
  std::unordered_map<std::size_t, std::shared_ptr<taskQueue>>
      _umpThreadId2TaskQueen;
  std::mutex _mtx;
  std::atomic<bool> _isStop;
  std::atomic<int> _iIndex;

 private:
  // 辅助函数，获取一个随机的、非当前线程的队列索引
  int getRandomVictimQueueIndex(std::size_t selfQueueId,
                                std::mt19937& generator) {
    if (_iThreadNum <= 1) {
      return -1;
    }

    std::uniform_int_distribution<int> distribution(0, _iThreadNum - 1);
    int victim_idx;
    int attempts = 0;
    const int max_attempts = _iThreadNum * 2;  // 避免极端情况下的无限循环

    do {
      victim_idx = distribution(generator);
      if (attempts++ > max_attempts) {
        return -1;
      }
    } while (victim_idx == (int)selfQueueId);

    return victim_idx;
  }

  std::shared_ptr<taskQueue> getspTaskQueenRandom(std::size_t queueid) {
    while (true) {
      static std::random_device r;
      static std::default_random_engine e1(r());
      static std::uniform_int_distribution<int> uniform_dist(0,
                                                             _iThreadNum - 1);
      std::size_t i = uniform_dist(e1);
      if (i != queueid) {
        return _umpThreadId2TaskQueen[i];
      }
    }
    return std::make_shared<taskQueue>();
  }

  // 修改：接受一个随机数生成器作为参数，并返回一个 TaskQueue 的 shared_ptr
  std::shared_ptr<taskQueue> getspTaskQueenRandom(std::size_t selfQueueId,
                                                  std::mt19937& generator) {
    // 如果只有一个线程或没有线程，无法窃取
    if (_iThreadNum <= 1) {
      return nullptr;  // 或者抛出异常，表示无法窃取
    }

    std::uniform_int_distribution<int> distribution(0, _iThreadNum - 1);
    int victim_idx;
    int attempts = 0;
    const int max_attempts = _iThreadNum * 2;  // 避免极端情况下的无限循环

    do {
      victim_idx = distribution(generator);
      if (attempts++ > max_attempts) {
        // 尝试多次都无法找到一个不同的队列，理论上不应该发生，返回nullptr
        return nullptr;
      }
    } while (victim_idx == (int)selfQueueId);  // 确保不窃取自己的队列

    // 修改：直接返回 map 中已存在的 shared_ptr
    return _umpThreadId2TaskQueen[victim_idx];
  }
};

inline ThreadPool::ThreadPool(std::size_t threadNum) : _isStop(false) {
  _iThreadNum = threadNum > std::thread::hardware_concurrency()
                    ? std::thread::hardware_concurrency()
                    : threadNum;
}
inline ThreadPool::~ThreadPool() { this->stop(); }

inline void ThreadPool::worker(int queueid) {
  // 每个工作线程拥有自己的随机数生成器，避免竞争和熵不足
  thread_local std::mt19937 generator(std::random_device{}() + queueid);
  auto my_task_queue = _umpThreadId2TaskQueen[queueid];  // 获取自己的任务队列
  while (!_isStop.load()) {
    std::function<void()> task =
        my_task_queue->consumeTask();  // 从自己的队列获取任务

    if (task) {
      task();  // 执行自己的任务
    } else {
      // 如果自己的队列为空，尝试从其他队列窃取任务
      std::optional<std::function<void()>> stolen_task_opt;
      bool stolen = false;
      for (size_t i = 0; i < _iThreadNum - 1; ++i) {  // 尝试 _iThreadNum - 1 次
        int victim_idx = getRandomVictimQueueIndex(queueid, generator);
        if (victim_idx == -1) continue;  // 没有可窃取的队列

        auto victim_queue = _umpThreadId2TaskQueen[victim_idx];
        if (!victim_queue || victim_queue->isStopped()) continue;
        stolen_task_opt = victim_queue->tryOfferSteal();
        if (stolen_task_opt) {
          stolen = true;
          break;  // 窃取成功，跳出循环
        }
      }
      if (stolen) {
        (*stolen_task_opt)();
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }
  return;
}

inline void ThreadPool::start() {
  for (size_t i = 0; i < _iThreadNum; i++) {
    _umpThreadId2TaskQueen.emplace(i, std::make_shared<taskQueue>());
  }

  for (std::size_t i = 0; i < _iThreadNum; i++) {
    _vWorkerThreads.emplace_back(
        std::thread(std::bind(&ThreadPool::worker, this, i)));
  }
}
inline void ThreadPool::stop() {
  _isStop = true;
  for (auto& t : _umpThreadId2TaskQueen) {
    t.second->notify_stop();
  }
  for (auto& woker : _vWorkerThreads) {
    if (woker.joinable()) woker.join();
  }
}
template <typename Func, typename... Args>
inline auto ThreadPool::submitTask(Func&& func, Args&&... args)
    -> std::future<decltype(func(args...))> {
  _iIndex++;
  _iIndex = (_iIndex % _iThreadNum);
  return _umpThreadId2TaskQueen[_iIndex]->producerTask(
      std::forward<Func>(func), std::forward<Args>(args)...);
}

#endif