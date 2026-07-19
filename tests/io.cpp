#include <gtest/gtest.h>

import std;
import iocpp;

namespace {

    TEST(IocppModuleTest, ReexportsErrorModule) {
        iocpp::Result<void> result{};

        EXPECT_TRUE(result.has_value());
    }

    TEST(IocppModuleTest, ReexportsFileModule) {
        iocpp::File file;

        EXPECT_FALSE(file.Valid());
    }

    TEST(IoTest, SleepAdvancesTime) {
        iocpp::TestContext test_context;
        iocpp::Io io{ test_context };

        auto const before = io.Now();
        io.Sleep(std::chrono::seconds(10));
        auto const after = io.Now();

        EXPECT_EQ(after - before, std::chrono::seconds(10));
    }

    TEST(IoTest, SleepUntilAdvancesToDeadline) {
        iocpp::TestContext test_context;
        iocpp::Io io{ test_context };
        auto const deadline = io.Now() + std::chrono::seconds(10);

        io.SleepUntil(deadline);

        EXPECT_EQ(io.Now(), deadline);
    }

    TEST(IoTest, SleepUntilIgnoresPastDeadline) {
        iocpp::TestContext test_context;
        iocpp::Io io{ test_context };
        auto const deadline = io.Now() - std::chrono::seconds(10);

        io.SleepUntil(deadline);

        EXPECT_EQ(io.Now(), iocpp::Timestamp{});
    }

    TEST(IoTest, CopiesShareContext) {
        iocpp::TestContext test_context;
        iocpp::Io first{ test_context };
        iocpp::Io second = first;

        second.Sleep(std::chrono::seconds(5));

        EXPECT_EQ(first.Now(), second.Now());
        EXPECT_EQ(first.Now(), iocpp::Timestamp{} + std::chrono::seconds(5));
    }

    TEST(IoTest, ConstHandleCanSleep) {
        iocpp::TestContext test_context;
        iocpp::Io const io{ test_context };

        io.Sleep(std::chrono::seconds(5));

        EXPECT_EQ(io.Now(), iocpp::Timestamp{} + std::chrono::seconds(5));
    }

} // namespace
