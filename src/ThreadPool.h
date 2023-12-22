#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <functional>
#include <queue>
#include <condition_variable>
#include <future>



class ThreadPool {
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::mutex counterMutex;
    std::condition_variable condition;
    std::condition_variable counterCondition;
    size_t numTasksEnqueued;
    size_t numTasksCompleted;
    bool stop;
public:
    explicit ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back(
                [this] {
                    while (true) {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                            });
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
                        auto task = std::move(this->tasks.front());
                        this->tasks.pop();
                        lock.unlock();
                        task();

                        // Increment the number of tasks completed
                        std::unique_lock<std::mutex> counterLock(this->counterMutex);
                        ++this->numTasksCompleted;
                        // Notify any waiting threads if all tasks have completed
                        if (this->numTasksCompleted == this->numTasksEnqueued) {
                            this->counterCondition.notify_all();
                        }
                    }
                }
            );
        }
    }
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& thread : threads) {
            thread.join();
        }
    }
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F(Args...)>::type> {
        using return_type = typename std::invoke_result<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks.emplace([task]() { (*task)(); });

            // Increment the number of tasks enqueued
            std::unique_lock<std::mutex> counterLock(counterMutex);
            ++numTasksEnqueued;
        }
        condition.notify_one();
        return res;
    }
    template<typename F, typename... Args>
    void enqueueVoid(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks.emplace([task]() { task(); });

            // Increment the number of tasks enqueued
            std::unique_lock<std::mutex> counterLock(counterMutex);
            ++numTasksEnqueued;
        }
        condition.notify_one();
    }

    void waitForCompletion() {
        std::unique_lock<std::mutex> counterLock(counterMutex);
        counterCondition.wait(counterLock, [this] {
            printf("waiting for completion of thread pool - %d:%d \n", this->numTasksCompleted, this->numTasksEnqueued);
            if (this->numTasksCompleted == this->numTasksEnqueued) {
                this->numTasksCompleted = 0;
                this->numTasksEnqueued = 0;
                return true;
            }
            return false;
            });
    }
};