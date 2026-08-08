// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <system_error>

#include "tools/training/utils/thread_pool.h"
#include "openzl/cpp/Exception.hpp"
#include "tools/logger/Logger.h"

namespace openzl::training {
using namespace openzl::tools::logger;

ThreadPool::ThreadPool(size_t numThreads_) : numThreads(numThreads_)
{
    if (numThreads == 0) {
        throw Exception("Number of threads must not be 0");
    }
    Logger::log_c(
            VERBOSE1, "Creating thread pool with %zu threads", numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        try {
            threads_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(lock, [this]() {
                            return stop_ || !taskQueue_.empty();
                        });
                        if (stop_ && taskQueue_.empty()) {
                            return;
                        }
                        task = std::move(taskQueue_.front());
                        taskQueue_.pop();
                    }
                    task();
                }
            });
        } catch (const std::system_error& e) {
            Logger::log_c(
                    WARNINGS,
                    "Failed to create thread %zu/%zu (%s). Proceeding with %zu threads.",
                    i + 1,
                    numThreads,
                    e.what(),
                    threads_.size());
            break;
        }
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (auto& thread : threads_) {
        thread.join();
    }
}
} // namespace openzl::training
