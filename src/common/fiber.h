// Copyright 2020 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include <memory>

namespace boost::context::detail {
struct transfer_t;
}

namespace Common {

/**
 * Fiber class
 * a fiber is a userspace thread with it's own context. They can be used to
 * implement coroutines, emulated threading systems and certain asynchronous
 * patterns.
 *
 * This class implements fibers at a low level, thus allowing greater freedom
 * to implement such patterns. This fiber class is 'threadsafe' only one fiber
 * can be running at a time and threads will be locked while trying to yield to
 * a running fiber until it yields. WARNING exchanging two running fibers between
 * threads will cause a deadlock. In order to prevent a deadlock, each thread should
 * have an intermediary fiber, you switch to the intermediary fiber of the current
 * thread and then from it switch to the expected fiber. This way you can exchange
 * 2 fibers within 2 different threads.
 */
class Fiber {
public:
    Fiber(std::function<void(void*)>&& entry_point_func, void* start_parameter);
    ~Fiber();

    Fiber(const Fiber&) = delete;
    Fiber& operator=(const Fiber&) = delete;

    Fiber(Fiber&&) = default;
    Fiber& operator=(Fiber&&) = default;

    /// Yields control from Fiber 'from' to Fiber 'to'
    /// Fiber 'from' must be the currently running fiber.
    static void YieldTo(std::weak_ptr<Fiber> weak_from, Fiber& to);
    [[nodiscard]] static std::shared_ptr<Fiber> ThreadToFiber();

    void SetRewindPoint(std::function<void(void*)>&& rewind_func, void* rewind_param);

    void Rewind();

    /// Only call from main thread's fiber
    void Exit();

    /// Changes the start parameter of the fiber. Has no effect if the fiber already started
    void SetStartParameter(void* new_parameter);

private:
    Fiber();

    void OnRewind(boost::context::detail::transfer_t& transfer);
    void Start(boost::context::detail::transfer_t& transfer);
    static void FiberStartFunc(boost::context::detail::transfer_t transfer);
    static void RewindStartFunc(boost::context::detail::transfer_t transfer);

    struct FiberImpl;
    std::unique_ptr<FiberImpl> impl;
};

} // namespace Common
