#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

class ThreadPool {
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    constexpr static unsigned CLEANUP_INTERVAL = 10;

  public:
    explicit ThreadPool(
        unsigned min_thread_count = std::thread::hardware_concurrency(),
        unsigned max_thread_count = 100,
        std::chrono::seconds timeout = std::chrono::seconds(60));
    ~ThreadPool() = default;
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    void queue_job(const std::function<void()> &job);
    void grow(unsigned new_thread_count);
    void cleanup();

    unsigned threads() {
        std::scoped_lock lk{threads_mtx_};
        return threads_.size();
    }

    unsigned busy() { return active_jobs_.load(); }

    unsigned available() {
        unsigned t = threads(), b = busy();
        return t > b ? t - b : 0;
    }

  private:
    void create_thread();

    void worker_loop(std::stop_token stoken,
                     std::atomic<TimePoint> &last_active);

    unsigned min_thread_count_;
    unsigned max_thread_count_;
    std::chrono::seconds timeout_;
    int age_;

    mutable std::mutex threads_mtx_;
    mutable std::mutex jobs_mtx_;
    std::condition_variable_any cv_;

    std::queue<std::function<void()>> jobs_;
    std::atomic<unsigned> active_jobs_;

    // Threads must be destroyed first to prevent access violation
    std::list<std::pair<std::jthread, std::atomic<TimePoint>>> threads_;
};
