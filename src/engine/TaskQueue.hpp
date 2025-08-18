#pragma once

#include <functional>
#include <future>
#include <queue>
#include <mutex>

namespace syng
{
namespace Concurrency { bool isMainThread(); }

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