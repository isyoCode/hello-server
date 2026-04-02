#include <cstddef>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
/* Implement a thread pool */
class ThreadPool_v1 {
public:
    explicit ThreadPool_v1():
    max_thread_num(20),
    thread_num(5),
    is_stop(false),
    max_task_num(100){}

    explicit ThreadPool_v1(std::size_t threadNum);
    explicit ThreadPool_v1(std::size_t threadNum, std::size_t taskNum);

    ThreadPool_v1(const ThreadPool_v1&) = delete;
    ThreadPool_v1(ThreadPool_v1&&) = delete;
    ThreadPool_v1& operator=(const ThreadPool_v1&) = delete;
    ThreadPool_v1& operator=(ThreadPool_v1&&) = delete;


    ~ThreadPool_v1();

    void worker();
    void start();
    void stop();
    template<class Func, class... Args>
    auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;

private:
    std::size_t max_thread_num;
    std::size_t thread_num;
    std::vector<std::thread> worker_threads;

    std::mutex mtx;
    std::condition_variable cond_cv;

    std::queue<std::function<void()>> task_queue;
    std::size_t max_task_num;

    std::atomic<std::size_t> cur_thread_nums;

    bool is_stop;
};

inline ThreadPool_v1::ThreadPool_v1(std::size_t threadNum):max_thread_num(20), max_task_num(100), is_stop(false) {
    thread_num = threadNum > max_thread_num ? max_thread_num : threadNum;
}
inline ThreadPool_v1::ThreadPool_v1(std::size_t threadNum, std::size_t taskNum):max_task_num(100), is_stop(false) {
    thread_num = threadNum > max_thread_num ? max_thread_num : threadNum;
    max_task_num = taskNum > max_task_num ? max_task_num : taskNum;
}
inline ThreadPool_v1::~ThreadPool_v1() { this->stop(); }

inline void ThreadPool_v1::worker() {
    while (is_stop == false) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cond_cv.wait(lock, [this]()-> bool{ return is_stop || !task_queue.empty(); });   /* get task from task_queue */
            if(is_stop) return;
            task = task_queue.front();
            task_queue.pop();
        }
        cond_cv.notify_one();
        task();
    }
}
inline void ThreadPool_v1::start(){
    for(std::size_t i = 0; i < thread_num; i++){
        worker_threads.emplace_back(std::thread(std::bind(&ThreadPool_v1::worker, this)));
    }
}
inline void ThreadPool_v1::stop(){
    is_stop = true;
    cond_cv.notify_all();
    for (auto& woker : worker_threads) {
        if(woker.joinable())
            woker.join();
    }
}

template <typename Func, typename ...Args>
auto ThreadPool_v1::submitTask(Func &&func, Args&&... args) -> std::future<decltype(func(args...))> {
    // need wrapper to a function<void()> 
    using RetType = decltype(func(args...));
    std::shared_ptr<std::packaged_task<RetType()>> task_ptr = 
                    std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<RetType> result = task_ptr->get_future();
    auto wrapper_task = [task_ptr]() ->void{
        (*task_ptr)();
    };
    // task_queue add the task
    {
        std::unique_lock<std::mutex> lock(mtx);
        cond_cv.wait(lock, [this](){return is_stop || task_queue.size() < max_task_num;});
        task_queue.emplace(wrapper_task);
    }
    cond_cv.notify_one();
    return result;
}

// int main(){
    // ThreadPool_v1 ThreadPool_v1(8);
    // ThreadPool_v1.start();
    // // auto getValue = ThreadPool_v1.submitTask(test_func, 1, 2, 4);
    // auto curtime = Tnow;
    // int num = 1000000;
    // std::vector<std::future<int>> futures(num);
    // for(int i = 0 ; i < num; i++){
    //     // futures.emplace_back(ThreadPool_v1.submitTask(test_func, 1, i, i));
    //     auto future = ThreadPool_v1.submitTask(test_func, 1, i, i);
    //     futures[i] = std::move(future);
    // }
    // for(auto& f : futures){
    //     f.get();
    // }
    // auto endtime = Tnow;
    // std::cout << "consume time " << std::chrono::duration_cast<std::chrono::milliseconds>(endtime - curtime).count() << std::endl;
    // // getchar();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 100));
//     return 0;

// }
