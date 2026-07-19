export module iocpp;

import std;
import iocpp.error;
import iocpp.file;

export namespace iocpp {

    struct IoContext {
        virtual ~IoContext() = default;

        virtual auto Now() const -> std::chrono::steady_clock::time_point = 0;
        virtual auto Sleep(std::chrono::steady_clock::duration duration) -> void = 0;
    };

    class Io {
    public:
        using Clock = std::chrono::steady_clock;
        using Duration = Clock::duration;
        using Timestamp = Clock::time_point;

        using Operation = detail::Operation;
        using ErrorCode = detail::ErrorCode;
        using Error = detail::Error;

        template <typename T>
        using Result = detail::Result<T>;

        using File = detail::File;

        struct Threaded;

        explicit Io(IoContext& context) noexcept
            : m_context(context) {}

        [[nodiscard]]
        auto AdoptFile(int native_handle) const noexcept -> File {
            return File{ native_handle };
        }

        [[nodiscard]]
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

    struct Io::Threaded final : IoContext {
        auto Now() const -> Timestamp override {
            return Clock::now();
        }

        auto Sleep(Duration duration) -> void override {
            std::this_thread::sleep_for(duration);
        }
    };

} // namespace iocpp
