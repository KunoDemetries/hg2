#pragma once
#include <chrono>
#include <stdexcept>
#include <system_error>
#include <thread>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace hg {
// HG-DIAG-012: host-only wait backend. Windows uses an unnamed, non-inheritable
// high-resolution timer; no system-wide timer period or guest state is changed.
// Derived from Microsoft's CreateWaitableTimerExW/SetWaitableTimer contracts.
// Unsupported high-resolution timers fail explicitly for --realtime. The
// ordinary unlimited runner creates no timer and never calls this wait.
class HostDeadlineWaiter {
public:
    using Clock=std::chrono::steady_clock;
    explicit HostDeadlineWaiter(bool enabled):enabled_(enabled) {
#ifdef _WIN32
        if(enabled_) {
            // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION (Windows 10 1803+).
            timer_=CreateWaitableTimerExW(nullptr,nullptr,0x00000002u,
                                          TIMER_MODIFY_STATE|SYNCHRONIZE);
            if(!timer_)throw std::system_error(int(GetLastError()),std::system_category(),
                                              "create high-resolution playback timer");
        }
#endif
    }
    ~HostDeadlineWaiter() {
#ifdef _WIN32
        if(timer_)CloseHandle(timer_);
#endif
    }
    HostDeadlineWaiter(const HostDeadlineWaiter&)=delete;
    HostDeadlineWaiter& operator=(const HostDeadlineWaiter&)=delete;
    void wait_until(Clock::time_point target) {
        if(!enabled_)throw std::logic_error("disabled playback waiter used");
        // Recheck the monotonic deadline even if a platform wakes early.
        for(auto now=Clock::now();now<target;now=Clock::now()) {
#ifdef _WIN32
            const auto remaining=std::chrono::duration_cast<std::chrono::nanoseconds>(target-now).count();
            LARGE_INTEGER due{};
            due.QuadPart=-((remaining+99)/100); // negative, relative 100ns units
            if(!SetWaitableTimer(timer_,&due,0,nullptr,nullptr,FALSE))
                throw std::system_error(int(GetLastError()),std::system_category(),"arm playback timer");
            const auto result=WaitForSingleObject(timer_,1000);
            if(result==WAIT_FAILED)
                throw std::system_error(int(GetLastError()),std::system_category(),"wait for playback timer");
            if(result!=WAIT_OBJECT_0)throw std::runtime_error("playback timer did not signal within one second");
#else
            std::this_thread::sleep_until(target);
#endif
        }
    }
    const char* backend() const {
        if(!enabled_)return "disabled";
#ifdef _WIN32
        return "Windows high-resolution waitable timer";
#else
        return "standard steady-clock sleep";
#endif
    }
private:
    bool enabled_;
#ifdef _WIN32
    HANDLE timer_=nullptr;
#endif
};
}
