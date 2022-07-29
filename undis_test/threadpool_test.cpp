#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <vector>

#include "../undis/threadpool.h"

using namespace std::chrono_literals;

constexpr unsigned MIN_THREAD_COUNT = 2;
constexpr unsigned MAX_THREAD_COUNT = 4;
constexpr auto TIMEOUT = 1s;

class ThreadPoolTest : public ::testing::Test {
  protected:
    ThreadPool tp{MIN_THREAD_COUNT, MAX_THREAD_COUNT, TIMEOUT};
};

TEST_F(ThreadPoolTest, ThreadsCreated) {
    EXPECT_EQ(tp.threads(), MIN_THREAD_COUNT);
    EXPECT_EQ(tp.busy(), 0);
    EXPECT_EQ(tp.available(), MIN_THREAD_COUNT);
}

TEST_F(ThreadPoolTest, CompletesJobs) {
    std::vector<std::promise<int>> ps;
    std::vector<std::future<int>> fs;

    for (int i = 0; i < 2 * MAX_THREAD_COUNT; ++i) {
        ps.emplace_back();
        fs.push_back(ps[i].get_future());
        tp.queue_job({[&ps, i] { ps[i].set_value(i); }});
    }

    std::this_thread::sleep_for(100ms);

    for (int i = 0; i < 2 * MAX_THREAD_COUNT; ++i) {
        EXPECT_EQ(fs[i].get(), i);
    }
}

TEST_F(ThreadPoolTest, CreatesMoreThreads) {
    bool start = false;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<unsigned> finished = 0;

    for (int i = 0; i < MAX_THREAD_COUNT + 1; ++i) {
        tp.queue_job({[&]() {
            std::unique_lock lk{mtx};
            cv.wait(lk, [&start]() { return start; });
            ++finished;
        }});
    }

    std::this_thread::sleep_for(200ms);
    std::unique_lock lk{mtx};
    start = true;
    while (finished < MAX_THREAD_COUNT + 1) {
        lk.unlock();
        cv.notify_all();
        std::this_thread::sleep_for(200ms);
        lk.lock();
    }

    EXPECT_EQ(tp.threads(), MAX_THREAD_COUNT);
}

TEST_F(ThreadPoolTest, CleansUpThreads) {
    EXPECT_EQ(tp.threads(), MIN_THREAD_COUNT);
    tp.cleanup();
    EXPECT_EQ(tp.threads(), MIN_THREAD_COUNT);

    tp.grow(MAX_THREAD_COUNT + 1);
    EXPECT_EQ(tp.threads(), MAX_THREAD_COUNT);

    tp.cleanup();
    EXPECT_EQ(tp.threads(), MAX_THREAD_COUNT);

    std::this_thread::sleep_for(TIMEOUT * 2);
    tp.cleanup();
    EXPECT_EQ(tp.threads(), MIN_THREAD_COUNT);
}
