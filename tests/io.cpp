import std;
import iocpp;

namespace {

    auto expect(
        bool condition,
        std::source_location location = std::source_location::current()
    ) -> void {
        if (condition) {
            return;
        }

        std::println(
            "expectation failed at {}:{}",
            location.file_name(),
            location.line()
        );
        std::abort();
    }

} // namespace

auto main() -> int {
    iocpp::Result<void> result{};
    expect(result.has_value());

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
        iocpp::Io io{ test_context };
        auto const deadline = io.Now() - std::chrono::seconds(10);
        io.SleepUntil(deadline);
        expect(io.Now() == iocpp::Timestamp{});
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
