#include "Concurrency.hpp"

namespace syng
{
namespace Concurrency
{
    std::thread::id g_mainThreadId;
    bool g_initialized = false;

    void initMainThread() {
        if (!g_initialized) {
            g_mainThreadId = std::this_thread::get_id();
            g_initialized = true;
        }
    }

    bool isMainThreadInitialized() {
        return g_initialized;
    }

    bool isMainThread() {
        return g_initialized && (std::this_thread::get_id() == g_mainThreadId);
    }

    std::thread::id getMainThread_ID() {
        return g_mainThreadId;
    }
}
}
