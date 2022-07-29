#include "threadpool.h"

ThreadPool::ThreadPool(unsigned min_thread_count, unsigned max_thread_count,
                       std::chrono::seconds timeout)
    : min_thread_count_{min_thread_count}, max_thread_count_{max_thread_count},
      timeout_{timeout}, age_{0}, active_jobs_{0} {
    std::scoped_lock lk{threads_mtx_};
    for (unsigned i = 0; i < min_thread_count_; ++i) {
        create_thread();
    }
}

void ThreadPool::queue_job(const std::function<void()> &job) {
    std::scoped_lock threads_lk{threads_mtx_};
    {
        std::scoped_lock jobs_lk{jobs_mtx_};
        if (active_jobs_ + jobs_.size() >= threads_.size()) {
            create_thread();
        }
        jobs_.push(job);
    }

    cv_.notify_one();
    if (++age_ >= 1) {
        cleanup();
    }
}

void ThreadPool::grow(unsigned new_thread_count) {
    if (new_thread_count >= max_thread_count_) {
        new_thread_count = max_thread_count_;
    }
    std::scoped_lock lk{threads_mtx_};
    if (new_thread_count > threads_.size()) {
        unsigned to_create = new_thread_count - threads_.size();
        while (to_create-- > 0) {
            create_thread();
        }
    }
}

void ThreadPool::cleanup() {
    age_ = 0;
    auto now = std::chrono::steady_clock::now();

    for (auto it = threads_.begin(), end = threads_.end(); it != end;) {
        if (threads_.size() > min_thread_count_ && it->first.joinable() &&
            it->second.load() != TimePoint{} &&
            now - it->second.load() > timeout_) {
            it = threads_.erase(it);
        } else {
            ++it;
        }
    }
}

void ThreadPool::create_thread() {
    using namespace std::placeholders;

    if (threads_.size() < max_thread_count_) {
        auto &back = threads_.emplace_back();
        back.first = std::jthread{std::bind(&ThreadPool::worker_loop, this, _1,
                                            std::ref(back.second))};
    }
}

void ThreadPool::worker_loop(std::stop_token stoken,
                             std::atomic<TimePoint> &last_active) {
    std::function<void()> job;
    while (true) {
        last_active = std::chrono::steady_clock::now();
        {
            std::unique_lock lk{jobs_mtx_};
            if (!cv_.wait(lk, stoken, [this]() { return !jobs_.empty(); })) {
                return;
            }

            last_active = TimePoint{};
            job = std::move(jobs_.front());
            jobs_.pop();
        }
        ++active_jobs_;
        job();
        --active_jobs_;
    }
}
