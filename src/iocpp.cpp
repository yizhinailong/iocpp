#include "iocpp/iocpp.hpp"

#include <cassert>
#include <print>

auto main() -> int {

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
        assert(after - before == std::chrono::seconds(10));
    }

    {
        iocpp::TestContext test_context;
        iocpp::Io io(test_context);
        auto const deadline = io.Now() + std::chrono::seconds(10);
        io.SleepUntil(deadline);
        assert(io.Now() == deadline);
    }

    {
        iocpp::TestContext test_context;
        iocpp::Io first(test_context);
        iocpp::Io second = first;

        second.Sleep(std::chrono::seconds(5));
        assert(first.Now() == second.Now());
        assert(first.Now() == iocpp::Timestamp{} + std::chrono::seconds(5));
    }

    {
        iocpp::TestContext test_context;
        iocpp::Io const io(test_context);
        io.Sleep(std::chrono::seconds(5));
        assert(io.Now() == iocpp::Timestamp{} + std::chrono::seconds(5));
    }

    return 0;
}
