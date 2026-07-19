#include <gtest/gtest.h>

import std;
import iocpp;

namespace {

    auto make_error() -> iocpp::Io::Error {
        return {
            .code = iocpp::Io::ErrorCode::Unsupported,
            .operation = iocpp::Io::Operation::Sleep,
        };
    }

    TEST(ErrorTest, StoresContext) {
        auto const error = make_error();

        EXPECT_EQ(error.code, iocpp::Io::ErrorCode::Unsupported);
        EXPECT_EQ(error.operation, iocpp::Io::Operation::Sleep);
    }

    TEST(ResultTest, HoldsValue) {
        iocpp::Io::Result<int> result{ 42 };

        ASSERT_TRUE(result.has_value());
        EXPECT_TRUE(result);
        EXPECT_EQ(result.value(), 42);
        EXPECT_EQ(*result, 42);
    }

    TEST(ResultTest, HoldsError) {
        iocpp::Io::Result<int> result = std::unexpected<iocpp::Io::Error>{ make_error() };

        ASSERT_FALSE(result.has_value());
        EXPECT_FALSE(result);
        EXPECT_EQ(result.error().code, iocpp::Io::ErrorCode::Unsupported);
        EXPECT_EQ(result.error().operation, iocpp::Io::Operation::Sleep);
        EXPECT_THROW(
            static_cast<void>(result.value()),
            std::bad_expected_access<iocpp::Io::Error>
        );
    }

    TEST(ResultTest, SupportsVoid) {
        iocpp::Io::Result<void> success{};
        iocpp::Io::Result<void> failure = std::unexpected<iocpp::Io::Error>{ make_error() };

        EXPECT_TRUE(success.has_value());
        EXPECT_FALSE(failure.has_value());
    }

    TEST(ResultTest, SupportsMoveOnlyValue) {
        iocpp::Io::Result<std::unique_ptr<int>> result{ std::make_unique<int>(42) };

        ASSERT_TRUE(result.has_value());
        ASSERT_NE(result->get(), nullptr);
        EXPECT_EQ(**result, 42);

        auto moved_value = std::move(result).value();
        ASSERT_NE(moved_value, nullptr);
        EXPECT_EQ(*moved_value, 42);
    }

} // namespace
