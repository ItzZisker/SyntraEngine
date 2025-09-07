#include "TaskQueue.hpp"

using namespace syng;

TaskQueue& TaskQueue::Instance() {
    static TaskQueue primary;
    return primary;
}

void TaskQueue::enqueue(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(mutex);
    tasks.push(std::move(task));
}

void TaskQueue::executeAll() {
    std::queue<std::function<void()>> local;
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::swap(local, tasks);
    }
    while (!local.empty()) {
        local.front()();
        local.pop();
    }
}