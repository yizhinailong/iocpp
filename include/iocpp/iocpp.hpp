#pragma once

#include <chrono>
#include <functional>
#include <thread>

namespace iocpp {

    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;
    using Timestamp = Clock::time_point;

    struct IoContext {
        virtual ~IoContext() = default;

        virtual auto Now() const -> Timestamp = 0;
        virtual auto Sleep(Duration duration) const -> void = 0;
    };

    struct ThreadContext : IoContext {
        virtual ~ThreadContext() = default;

        auto Now() const -> Timestamp override {
            return Clock::now();
        }

        auto Sleep(Duration duration) const -> void override {
            std::this_thread::sleep_for(duration);
        }
    };

    class Io {
    public:
        explicit Io(IoContext& context)
            : m_context(context) {}

        auto Now() const -> Timestamp {
            return m_context.get().Now();
        }

        auto Sleep(Duration duration) const -> void {
            m_context.get().Sleep(duration);
        }

    private:
        std::reference_wrapper<IoContext> m_context;
    };

} // namespace iocpp
