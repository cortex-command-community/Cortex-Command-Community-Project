/**
 * BS_thread_pool.hpp — Emscripten single-threaded stub
 *
 * Replaces BS::thread_pool with a synchronous inline executor for Emscripten.
 * All submitted tasks run immediately on the calling thread (no threads spawned).
 *
 * API surface matches BS::thread_pool v3.5.0 for the subset used by CCCP.
 */
#pragma once

#include <functional>
#include <future>
#include <vector>
#include <type_traits>

namespace BS {

using concurrency_t = unsigned int;

/// Minimal multi_future shim — a vector of futures
template<typename T>
class multi_future : public std::vector<std::future<T>> {
public:
    multi_future() = default;
    // Allow assignment from sync_result (defined below)
    struct sync_result_tag {};
    multi_future(sync_result_tag) {}
    multi_future& operator=(sync_result_tag) { this->clear(); return *this; }
    void wait() {}
    void get() {}
};

/// Synchronous thread pool — tasks run inline on the calling thread.
class thread_pool {
public:
    explicit thread_pool(concurrency_t = 0) {}

    void reset(concurrency_t = 0) {}
    void wait_for_tasks() {}
    void pause()  {}
    void unpause() {}
    bool is_paused() const { return false; }
    concurrency_t get_thread_count() const { return 0; }

    /// Submit a callable with no return value (fire-and-forget).
    template<typename F, typename... A>
    void push_task(F&& f, A&&... args) {
        std::invoke(std::forward<F>(f), std::forward<A>(args)...);
    }

    /// Submit a callable and return a future (runs immediately, future is ready).
    template<typename F, typename... A,
             typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
    [[nodiscard]] std::future<R> submit(F&& f, A&&... args) {
        std::promise<R> promise;
        if constexpr (std::is_void_v<R>) {
            std::invoke(std::forward<F>(f), std::forward<A>(args)...);
            promise.set_value();
        } else {
            promise.set_value(std::invoke(std::forward<F>(f), std::forward<A>(args)...));
        }
        return promise.get_future();
    }

    /// Trivially-waitable return type for parallelize_loop.
    /// Implicitly converts to multi_future<void> so member variables can be assigned.
    struct sync_result {
        void wait() {}
        // Implicit conversion to multi_future<T> for any T
        template<typename T>
        operator multi_future<T>() const { return multi_future<T>{}; }
    };

    /// parallelize_loop(N, func) — runs func(0, N) synchronously, returns waitable
    template<typename T, typename F>
    sync_result parallelize_loop(T count, F&& f, concurrency_t = 0) {
        if (count > 0) f(static_cast<T>(0), count);
        return {};
    }

    /// parallelize_loop(first, last, func) overload
    template<typename T, typename F>
    sync_result parallelize_loop(T first, T last, F&& f, concurrency_t = 0) {
        if (first < last) f(first, last);
        return {};
    }
};

} // namespace BS
