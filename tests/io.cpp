#include <gtest/gtest.h>

import std;
import iocpp;

namespace {

    class TestIoContext final : public iocpp::IoContext {
    private:
        iocpp::Io::Timestamp m_current_time{};

    public:
        auto Now() const -> iocpp::Io::Timestamp override {
            return m_current_time;
        }

        auto Sleep(iocpp::Io::Duration duration) -> void override {
            m_current_time += duration;
        }
    };

    static_assert(std::derived_from<iocpp::Io::Threaded, iocpp::IoContext>);
    static_assert(std::derived_from<TestIoContext, iocpp::IoContext>);

    TEST(IocppModuleTest, ExposesNestedErrorTypes) {
        iocpp::Io::Result<void> result{};

        EXPECT_TRUE(result.has_value());
    }

    TEST(IocppModuleTest, ExposesNestedFileType) {
        iocpp::Io::File file;

        EXPECT_FALSE(file.Valid());
    }

    TEST(IoTest, SleepAdvancesTime) {
        TestIoContext test_context;
        iocpp::Io io{ test_context };

        auto const before = io.Now();
        io.Sleep(std::chrono::seconds(10));
        auto const after = io.Now();

        EXPECT_EQ(after - before, std::chrono::seconds(10));
    }

    TEST(IoTest, SleepUntilAdvancesToDeadline) {
        TestIoContext test_context;
        iocpp::Io io{ test_context };
        auto const deadline = io.Now() + std::chrono::seconds(10);

        io.SleepUntil(deadline);

        EXPECT_EQ(io.Now(), deadline);
    }

    TEST(IoTest, SleepUntilIgnoresPastDeadline) {
        TestIoContext test_context;
        iocpp::Io io{ test_context };
        auto const deadline = io.Now() - std::chrono::seconds(10);

        io.SleepUntil(deadline);

        EXPECT_EQ(io.Now(), iocpp::Io::Timestamp{});
    }

    TEST(IoTest, CopiesShareContext) {
        TestIoContext test_context;
        iocpp::Io first{ test_context };
        iocpp::Io second = first;

        second.Sleep(std::chrono::seconds(5));

        EXPECT_EQ(first.Now(), second.Now());
        EXPECT_EQ(first.Now(), iocpp::Io::Timestamp{} + std::chrono::seconds(5));
    }

    TEST(IoTest, ConstHandleCanSleep) {
        TestIoContext test_context;
        iocpp::Io const io{ test_context };

        io.Sleep(std::chrono::seconds(5));

        EXPECT_EQ(io.Now(), iocpp::Io::Timestamp{} + std::chrono::seconds(5));
    }

} // namespace
