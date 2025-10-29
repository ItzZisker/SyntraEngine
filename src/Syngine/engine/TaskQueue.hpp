#pragma once

#include <functional>
#include <future>
#include <queue>
#include <mutex>

namespace syng
{
namespace Concurrency { bool isMainThread(); }

namespace future {
    template <typename T, typename F>
    void when_complete(std::future<T>&& fut, F&& callback) {
        std::thread([fut = std::move(fut), callback = std::forward<F>(callback)]() mutable {
            try {
                T value = fut.get(); // wait and get result
                callback(std::move(value), std::exception_ptr{});
            } catch (...) {
                callback(T{}, std::current_exception());
            }
        }).detach();
    }
}

class TaskQueue {
public:
    static TaskQueue& Instance();

    void executeAll();
    void enqueue(std::function<void()> task);

    template<typename F>
    auto enqueueFuture(F&& f) {
        using Ret = decltype(f());

        auto task = std::make_shared<std::packaged_task<Ret()>>(std::forward<F>(f));
        auto future = task->get_future();

        if (Concurrency::isMainThread()) {
            (*task)();  
        } else {
            enqueue([task]() mutable {
                (*task)();
            });
        }
        return future;
    }
private:
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
};
}