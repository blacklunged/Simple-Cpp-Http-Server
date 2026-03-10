#include "../include/ThreadPool.hpp"
#include "../include/Logger.hpp"
#include <iostream>

ThreadPool::ThreadPool(size_t num_threads) : m_stopping(false) {
    if (num_threads == 0) {
        throw std::invalid_argument("ThreadPool must have at least 1 thread");
    }

    for (size_t i = 0; i < num_threads; ++i) {
        m_workers.emplace_back(&ThreadPool::workerThread, this);
    }

    Logger::logInfo(
        "ThreadPool created with " + std::to_string(num_threads) + " worker threads"
    );
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_stopping) {
            Logger::logError("Attempted to enqueue task after ThreadPool was stopped");
            throw std::runtime_error("Cannot enqueue task: ThreadPool is stopping");
        }

        m_tasks.push(std::move(task));
    }

    m_cv.notify_one();
}

void ThreadPool::stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_stopping) {
            return; 
        }

        m_stopping = true;
    }

    m_cv.notify_all();

    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    Logger::logInfo(
        "ThreadPool stopped. Total threads joined: " +
        std::to_string(m_workers.size())
    );
}

size_t ThreadPool::getQueueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tasks.size();
}

void ThreadPool::workerThread() {

    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            m_cv.wait(lock, [this]() {
                return m_stopping || !m_tasks.empty();
            });

            if (m_stopping && m_tasks.empty()) {
                break;
            }

            if (m_tasks.empty()) {
                continue;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }

        try {
            task();
        } catch (const std::exception& e) {
            Logger::logError(
                std::string("Worker thread exception: ") + e.what()
            );
        }
    }

    Logger::logInfo("Worker thread terminated");
}
