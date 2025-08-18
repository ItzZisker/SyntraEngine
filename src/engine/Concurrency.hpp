#pragma once

#include <thread>

namespace syng
{
namespace Concurrency
{
    void initMainThread();
    bool isMainThread();
    std::thread::id getMainThread_ID();
    bool isMainThreadInitialized();
}
}