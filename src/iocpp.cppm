export module iocpp;

export import iocpp.error;
export import iocpp.file;

import std;

export namespace iocpp {

    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;
    using Timestamp = Clock::time_point;

    struct IoContext {
        virtual ~IoContext() = default;

        virtual auto Now() const -> Timestamp = 0;
        virtual auto Sleep(Duration duration) -> void = 0;
    };

    struct ThreadContext final : IoContext {
        auto Now() const -> Timestamp override {
            return Clock::now();
        }

        auto Sleep(Duration duration) -> void override {
            std::this_thread::sleep_for(duration);
        }
    };

    struct TestContext final : IoContext {
        Timestamp current_time{};

        auto Now() const -> Timestamp override {
            return current_time;
        }

        auto Sleep(Duration duration) -> void override {
            this->current_time += duration;
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

        auto SleepUntil(Timestamp deadline) const -> void {
            auto const now = this->Now();
            if (deadline > now) {
                this->Sleep(deadline - now);
            }
        }

    private:
        std::reference_wrapper<IoContext> m_context;
    };

} // namespace iocpp
