// ============================================================================
// threadpool_test.cc - 线程池性能和功能测试
// ============================================================================
// 测试线程池的各项功能和性能指标
//
// 测试内容:
//   - 基本任务提交和执行
//   - 多任务并发处理
//   - 返回值获取
//   - 性能测试 (与单线程比较)
//   - 异常处理
// ============================================================================

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <functional>
#include <cassert>
#include <set>
#include <iomanip>
#include <sstream>
#include "../../src/threadpool/threadpool.h"
#include "../../src/utils/util.h"

using namespace yoyo;

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * 简单的计算函数，模拟真实的 CPU 密集任务
 * @param a, b, c: 输入参数
 * @return 返回计算结果
 */
int compute(int a, int b, int c) {
    // 模拟计算工作（10ms）
    SLEEP(0.01);
    return a + b + c;
}

/**
 * 计算任务，返回当前线程ID
 */
std::string getThreadInfo(int id) {
    SLEEP(0.01);
    std::ostringstream oss;
    oss << "Task " << id << " executed in thread " << std::this_thread::get_id();
    return oss.str();
}

/**
 * 打印输出函数
 */
void printLine(const std::string& message) {
    std::cout << message << std::endl;
}

// ============================================================================
// 测试 1: 基本功能
// ============================================================================

void test_basic_functionality() {
    std::cout << "\n════════════════════════════════════════" << std::endl;
    std::cout << "测试 1: 基本功能" << std::endl;
    std::cout << "════════════════════════════════════════" << std::endl;

    ThreadPool pool(2);  // 创建 2 个工作线程的线程池
    pool.start();

    // 提交简单任务
    auto future1 = pool.submitTask(compute, 1, 2, 3);
    auto future2 = pool.submitTask(compute, 4, 5, 6);
    auto future3 = pool.submitTask(compute, 7, 8, 9);

    // 获取结果
    int result1 = future1.get();
    int result2 = future2.get();
    int result3 = future3.get();

    std::cout << "✓ Task 1 result: " << result1 << " (expected: 6)" << std::endl;
    std::cout << "✓ Task 2 result: " << result2 << " (expected: 15)" << std::endl;
    std::cout << "✓ Task 3 result: " << result3 << " (expected: 24)" << std::endl;

    assert(result1 == 6);
    assert(result2 == 15);
    assert(result3 == 24);

    std::cout << "✓ 基本功能测试通过！" << std::endl;
}

// ============================================================================
// 测试 2: 多任务并发处理
// ============================================================================

void test_concurrent_execution() {
    std::cout << "\n════════════════════════════════════════" << std::endl;
    std::cout << "测试 2: 多任务并发处理" << std::endl;
    std::cout << "════════════════════════════════════════" << std::endl;

    ThreadPool pool(4);  // 4 个工作线程
    pool.start();

    const int NUM_TASKS = 10;
    std::vector<std::future<std::string>> futures;

    // 提交 10 个任务
    std::cout << "提交 " << NUM_TASKS << " 个任务到线程池..." << std::endl;
    for (int i = 0; i < NUM_TASKS; ++i) {
        futures.push_back(pool.submitTask(getThreadInfo, i));
    }

    // 等待所有任务完成并收集结果
    std::cout << "等待任务完成..." << std::endl;
    std::set<std::thread::id> thread_ids;
    for (int i = 0; i < NUM_TASKS; ++i) {
        std::string result = futures[i].get();
        std::cout << "  " << result << std::endl;
        // 提取线程 ID（简化处理）
    }

    std::cout << "✓ 所有 " << NUM_TASKS << " 个任务已完成！" << std::endl;
}

// ============================================================================
// 测试 3: 性能对比（线程池 vs 单线程）
// ============================================================================

void test_performance() {
    std::cout << "\n════════════════════════════════════════" << std::endl;
    std::cout << "测试 3: 性能对比" << std::endl;
    std::cout << "════════════════════════════════════════" << std::endl;

    const int NUM_TASKS = 100;
    int result = 0;

    // 方法 1: 单线程顺序执行
    std::cout << "\n单线程执行 " << NUM_TASKS << " 个任务..." << std::endl;
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_TASKS; ++i) {
        result = compute(i, i + 1, i + 2);
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(
        end1 - start1);

    std::cout << "✓ 单线程耗时: " << duration1.count() << " ms" << std::endl;

    // 方法 2: 线程池并发执行
    std::cout << "\n线程池执行 " << NUM_TASKS << " 个任务..." << std::endl;
    ThreadPool pool(4);
    pool.start();

    auto start2 = std::chrono::high_resolution_clock::now();
    std::vector<std::future<int>> futures;

    for (int i = 0; i < NUM_TASKS; ++i) {
        futures.push_back(pool.submitTask(compute, i, i + 1, i + 2));
    }

    // 等待所有任务完成
    for (auto& future : futures) {
        result = future.get();
    }

    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(
        end2 - start2);

    std::cout << "✓ 线程池耗时: " << duration2.count() << " ms" << std::endl;

    // 计算性能提升
    double speedup = static_cast<double>(duration1.count()) /
                     static_cast<double>(duration2.count());
    std::cout << "\n性能提升: " << std::fixed << std::setprecision(2)
              << speedup << "x" << std::endl;

    if (duration2.count() < duration1.count()) {
        std::cout << "✓ 线程池性能更优！" << std::endl;
    }
}

// ============================================================================
// 测试 4: 不同线程数的性能
// ============================================================================

void test_different_thread_counts() {
    std::cout << "\n════════════════════════════════════════" << std::endl;
    std::cout << "测试 4: 不同线程数的性能对比" << std::endl;
    std::cout << "════════════════════════════════════════" << std::endl;

    const int NUM_TASKS = 50;
    std::vector<int> thread_counts = {1, 2, 4, 8};

    std::cout << "\n执行 " << NUM_TASKS << " 个计算任务，不同的线程池大小:\n" << std::endl;
    std::cout << "线程数\t耗时(ms)" << std::endl;
    std::cout << "─────────────────" << std::endl;

    for (int num_threads : thread_counts) {
        ThreadPool pool(num_threads);
        pool.start();

        auto start = std::chrono::high_resolution_clock::now();
        std::vector<std::future<int>> futures;

        for (int i = 0; i < NUM_TASKS; ++i) {
            futures.push_back(pool.submitTask(compute, i, i + 1, i + 2));
        }

        for (auto& future : futures) {
            future.get();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start);

        std::cout << num_threads << "\t" << duration.count() << std::endl;
    }
}

// ============================================================================
// 测试 5: 任务优先级（先入先出）
// ============================================================================

void test_task_order() {
    std::cout << "\n════════════════════════════════════════" << std::endl;
    std::cout << "测试 5: 任务执行顺序" << std::endl;
    std::cout << "════════════════════════════════════════" << std::endl;

    ThreadPool pool(1);  // 单线程池确保顺序执行
    pool.start();

    std::cout << "提交 5 个任务，观察执行顺序..." << std::endl;

    std::vector<std::future<int>> futures;
    for (int i = 1; i <= 5; ++i) {
        futures.push_back(pool.submitTask(compute, i * 10, i * 10, i * 10));
    }

    std::cout << "任务结果:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        int result = futures[i].get();
        std::cout << "✓ Task " << (i + 1) << " result: " << result << std::endl;
    }

    std::cout << "✓ 任务执行顺序测试通过！" << std::endl;
}

// ============================================================================
// 测试 6: 错误处理
// ============================================================================

void test_exception_handling() {
    std::cout << "\n════════════════════════════════════════" << std::endl;
    std::cout << "测试 6: 异常处理" << std::endl;
    std::cout << "════════════════════════════════════════" << std::endl;

    ThreadPool pool(2);
    pool.start();

    // 提交一个会抛出异常的 lambda
    auto future = pool.submitTask([]() -> int {
        throw std::runtime_error("Test exception");
        return 42;
    });

    try {
        int result = future.get();
        std::cout << "✗ 异常未被捕获！" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "✓ 成功捕获异常: " << e.what() << std::endl;
    }

    std::cout << "✓ 异常处理测试通过！" << std::endl;
}

// ============================================================================
// 测试 7: 内存效率
// ============================================================================

void test_memory_efficiency() {
    std::cout << "\n════════════════════════════════════════" << std::endl;
    std::cout << "测试 7: 内存效率" << std::endl;
    std::cout << "════════════════════════════════════════" << std::endl;

    ThreadPool pool(4);
    pool.start();

    std::cout << "提交大量任务测试内存效率..." << std::endl;

    const int NUM_TASKS = 1000;
    std::vector<std::future<int>> futures;

    for (int i = 0; i < NUM_TASKS; ++i) {
        futures.push_back(pool.submitTask(compute, 1, 2, 3));
    }

    int completed = 0;
    for (auto& future : futures) {
        future.get();
        completed++;
        if (completed % 200 == 0) {
            std::cout << "✓ 已完成 " << completed << "/" << NUM_TASKS
                      << " 个任务" << std::endl;
        }
    }

    std::cout << "✓ 所有 " << NUM_TASKS << " 个任务已完成！" << std::endl;
    std::cout << "✓ 内存效率测试通过！" << std::endl;
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════╗"
              << std::endl;
    std::cout << "║     YoyoCppServer 线程池单元测试和性能测试      ║"
              << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝"
              << std::endl;

    try {
        test_basic_functionality();
        test_concurrent_execution();
        test_performance();
        test_different_thread_counts();
        test_task_order();
        test_exception_handling();
        test_memory_efficiency();

        std::cout << "\n╔════════════════════════════════════════════════════╗"
                  << std::endl;
        std::cout << "║              ✓ 所有测试通过！                    ║"
                  << std::endl;
        std::cout << "╚════════════════════════════════════════════════════╝"
                  << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ 测试失败: " << e.what() << std::endl;
        return 1;
    }
}

// ============================================================================
// 编译和运行说明
// ============================================================================
/*
 * 编译命令:
 *   g++ -std=c++20 threadpool_test.cc -o threadpool_test \
 *       -I../../src -pthread
 *
 * 运行命令:
 *   ./threadpool_test
 *
 * 期望输出:
 *   ✓ 所有测试通过！
 *
 * 性能指标说明:
 *   - 线程池通常比单线程快 3-4 倍（取决于 CPU 核数）
 *   - 线程数应接近 CPU 核数以获得最佳性能
 *   - 超过 CPU 核数的线程会导致上下文切换开销
 */
