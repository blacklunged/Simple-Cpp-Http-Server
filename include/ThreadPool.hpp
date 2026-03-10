#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);
    void stop();

    size_t getThreadCount() const { return m_workers.size(); }
    size_t getQueueSize() const;

private:
    void workerThread();

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stopping;
};

#endif // THREADPOOL_HPP
