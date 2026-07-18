#include <gtest/gtest.h>

import std;
import iocpp.error;

namespace {

    auto make_error() -> iocpp::Error {
        return {
            .code = iocpp::ErrorCode::Unsupported,
            .operation = iocpp::Operation::Sleep,
        };
    }

    TEST(ErrorTest, StoresContext) {
        auto const error = make_error();

        EXPECT_EQ(error.code, iocpp::ErrorCode::Unsupported);
        EXPECT_EQ(error.operation, iocpp::Operation::Sleep);
    }

    TEST(ResultTest, HoldsValue) {
        iocpp::Result<int> result{ 42 };

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 42);
    }

    TEST(ResultTest, HoldsError) {
        iocpp::Result<int> result = std::unexpected<iocpp::Error>{ make_error() };

        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().code, iocpp::ErrorCode::Unsupported);
        EXPECT_EQ(result.error().operation, iocpp::Operation::Sleep);
    }

    TEST(ResultTest, SupportsVoid) {
        iocpp::Result<void> success{};
        iocpp::Result<void> failure = std::unexpected<iocpp::Error>{ make_error() };

        EXPECT_TRUE(success.has_value());
        EXPECT_FALSE(failure.has_value());
    }

    TEST(ResultTest, SupportsMoveOnlyValue) {
        iocpp::Result<std::unique_ptr<int>> result{ std::make_unique<int>(42) };

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(**result, 42);

        auto moved_value = std::move(result).value();
        ASSERT_NE(moved_value, nullptr);
        EXPECT_EQ(*moved_value, 42);
    }

} // namespace
