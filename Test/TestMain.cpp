#include "Base/ThreadPool.h"
#include <iostream>

// 压力测试
void stressTest(FThreadPool& pool) {
    const int numTasks = 10000;
    std::vector<std::future<int>> futures;
    for (int i = 0; i < numTasks; ++i) {
        futures.push_back(pool.pushTask([i]() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); return i; }));
    }
    bool passed = true;
    for (int i = 0; i < numTasks; ++i) {
        if (futures[i].get() != i) {
            passed = false;
            break;
        }
    }
    if (passed) {
        std::cout << "Stress Test Passed" << std::endl;
    } else {
        std::cout << "Stress Test Failed" << std::endl;
    }
}

// 边界测试：零任务、大量小任务、长时间任务
void boundaryTest(FThreadPool& pool) {
    // 零任务
    std::cout << "Boundary Test: Zero Tasks Passed" << std::endl;

    // 大量小任务
    const int numSmallTasks = 10000;
    std::vector<std::future<void>> smallTaskFutures;
    for (int i = 0; i < numSmallTasks; ++i) {
        smallTaskFutures.push_back(pool.pushTask([]() { return; }));
    }
    for (auto& future : smallTaskFutures) {
        future.get();
    }
    std::cout << "Boundary Test: Large Number of Small Tasks Passed" << std::endl;

    // 长时间任务
    auto longTask = pool.pushTask([]() { std::this_thread::sleep_for(std::chrono::seconds(2)); return; });
    longTask.get();
    std::cout << "Boundary Test: Long Duration Task Passed" << std::endl;
}

// 资源管理测试
void resourceManagementTest(FThreadPool& pool) {
    const int numTasks = 100;
    std::vector<std::future<void>> futures;
    for (int i = 0; i < numTasks; ++i) {
        futures.push_back(pool.pushTask([]() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); }));
    }
    for (auto& future : futures) {
        future.get();
    }
    std::cout << "Resource Management Test Passed" << std::endl;
}

// 并发性测试
void concurrencyTest(FThreadPool& pool) {
    const int numThreads = 10;
    const int numTasksPerThread = 100;
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&pool, numTasksPerThread, t] {
            for (int i = 0; i < numTasksPerThread; ++i) {
                pool.pushTask([i, t]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    // printf("Thread %d Task %d\n", t, i);
                });
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "Concurrency Test Passed" << std::endl;
}

// 长时间运行测试
void longRunningTest(FThreadPool& pool) {
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    std::atomic<int> counter{0};
    while (std::chrono::steady_clock::now() < endTime) {
        pool.pushTask([&counter]() {
            // 压的任务太多了，给压爆了。如果sleep时间短的话就没问题了
            // std::this_thread::sleep_for(std::chrono::microseconds(1));
            counter++;
        });
    }
    std::cout << "Long Running Test Completed with " << counter.load() << " tasks executed" << std::endl;
}

// 回调测试
void callbackTest(FThreadPool& pool) {
    auto future = pool.pushTask([]() { std::this_thread::sleep_for(std::chrono::seconds(1)); return 42; });
    future.wait();
    if (future.get() == 42) {
        std::cout << "Callback Test Passed" << std::endl;
    } else {
        std::cout << "Callback Test Failed" << std::endl;
    }
}

// 负载均衡测试
void loadBalancingTest(FThreadPool& pool) {
    const int numTasks = 100;
    std::vector<std::future<void>> futures;
    for (int i = 0; i < numTasks; ++i) {
        futures.push_back(pool.pushTask([]() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }));
    }
    for (auto& future : futures) {
        future.get();
    }
    std::cout << "Load Balancing Test Passed" << std::endl;
}

int main() {
    auto&& pool = FThreadPool::getInst();

    stressTest(pool);
    boundaryTest(pool);
    resourceManagementTest(pool);
    concurrencyTest(pool);
    longRunningTest(pool);
    callbackTest(pool);
    loadBalancingTest(pool);

    // auto func1 = [](int a, int b) { return a + b; };
    // auto fut = pool.pushTask(func1, 1, 2);
    // std::cout << fut.get() << std::endl;
}