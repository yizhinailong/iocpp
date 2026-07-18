import std;
import iocpp;

namespace {

    auto expect(bool condition) -> void {
        if (!condition) {
            std::abort();
        }
    }

} // namespace

auto main() -> int {

    {
        auto const error = iocpp::Error{
            .code = iocpp::ErrorCode::Unsupported,
            .operation = iocpp::Operation::Sleep
        };
        expect(error.code == iocpp::ErrorCode::Unsupported);
        expect(error.operation == iocpp::Operation::Sleep);
    }

    {
        iocpp::Result<void> success{};
        iocpp::Result<int> failure = std::unexpected<iocpp::Error>(iocpp::Error{
            .code = iocpp::ErrorCode::Unsupported,
            .operation = iocpp::Operation::Sleep,
        });
        expect(success.has_value());
        expect(!failure.has_value());
    }

    {
        iocpp::ThreadContext thread_context;
        iocpp::Io io{ thread_context };

        auto const timestamp = io.Now().time_since_epoch();
        std::println(
            "io.Now(): {} ns",
            std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp).count()
        );

        io.Sleep(std::chrono::milliseconds(100));
    }

    {
        iocpp::TestContext test_context;
        iocpp::Io io{ test_context };

        auto const before = io.Now();
        io.Sleep(std::chrono::seconds(10));
        auto const after = io.Now();
        expect(after - before == std::chrono::seconds(10));
    }

    {
        iocpp::TestContext test_context;
        iocpp::Io io{ test_context };
        auto const deadline = io.Now() + std::chrono::seconds(10);
        io.SleepUntil(deadline);
        expect(io.Now() == deadline);
    }

    {
        iocpp::TestContext test_context;
        iocpp::Io first{ test_context };
        iocpp::Io second = first;

        second.Sleep(std::chrono::seconds(5));
        expect(first.Now() == second.Now());
        expect(first.Now() == iocpp::Timestamp{} + std::chrono::seconds(5));
    }

    {
        iocpp::TestContext test_context;
        iocpp::Io const io{ test_context };
        io.Sleep(std::chrono::seconds(5));
        expect(io.Now() == iocpp::Timestamp{} + std::chrono::seconds(5));
    }

    return 0;
}
