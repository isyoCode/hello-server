#include "threadpool.h"
#include "../include/util.h"
#include <iostream>
#include <utility>
#include "threadPool_v1.h"


const int num = 1000 * 5;



/**测试一下线程池耗时*/
int func(int a, int b, int c){
        SLEEP(0.01);
        return a + b + c;
}

void baseline(){
    std::vector<std::shared_ptr<int>> vbaseline(num);
    for(int i = 0 ;i < num; i++){
       vbaseline.emplace_back(std::make_shared<int>(func( 1, i, i)));
    }
    for(int i = 0 ;i < num;i++){
        auto ans = vbaseline[i];
    }
    std::cout << "无线程池基准计算任务完成" << std::endl;
}


void consumed_one(){
    // 创建一个线程池
    ThreadPool threadpool(4);
    threadpool.start();
    std::vector<std::future<int>> futures(num);
    for(int i = 0 ;i < num; i++){
        auto future = threadpool.submitTask(func, 1, i, i);
        futures[i] = std::move(future);
    }
    for(int i = 0 ;i < num;i++){
        futures[i].get();
    }
    std::cout << "多任务队列线程池任务提交完成" << std::endl;
}

void consumed_v1(){
    // 创建一个线程池
    ThreadPool_v1 threadpool(4);
    threadpool.start();
    std::vector<std::future<int>> futures(num);
    for(int i = 0 ;i < num; i++){
        auto future = threadpool.submitTask(func, 1, i, i);
        futures[i] = std::move(future);
    }
    for(int i = 0 ;i < num;i++){
        futures[i].get();
    }
    std::cout << "单任务队列线程池任务提交完成" << std::endl;
}

int main(){
    // _TIMECOUNT_(baseline(), yoyo::_Ms_, "测试耗时");
    _TIMECOUNT_(consumed_v1(), yoyo::_Ms_, "测试耗时");
    _TIMECOUNT_(consumed_one(), yoyo::_Ms_, "测试耗时");

    return 0;

}


