#include <cerrno>

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

import std;
import iocpp.file;

namespace {

    static_assert(!std::is_copy_constructible_v<iocpp::File>);
    static_assert(!std::is_copy_assignable_v<iocpp::File>);
    static_assert(std::is_nothrow_move_constructible_v<iocpp::File>);
    static_assert(std::is_nothrow_move_assignable_v<iocpp::File>);
    static_assert(std::is_nothrow_destructible_v<iocpp::File>);

    auto open_null() -> int {
        return ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }

    auto expect_open(int native_handle) -> void {
        EXPECT_NE(::fcntl(native_handle, F_GETFD), -1);
    }

    auto expect_closed(int native_handle) -> void {
        errno = 0;
        EXPECT_EQ(::fcntl(native_handle, F_GETFD), -1);
        EXPECT_EQ(errno, EBADF);
    }

    TEST(FileTest, DefaultConstructsEmpty) {
        iocpp::File file;

        EXPECT_FALSE(file.Valid());
        EXPECT_EQ(file.NativeHandle(), iocpp::File::INVALID_HANDLE);
    }

    TEST(FileTest, AdoptsNativeHandle) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::File::INVALID_HANDLE);

        iocpp::File file{ native_handle };

        EXPECT_TRUE(file.Valid());
        EXPECT_EQ(file.NativeHandle(), native_handle);
        expect_open(native_handle);
    }

    TEST(FileTest, DestructorClosesNativeHandle) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::File::INVALID_HANDLE);

        {
            iocpp::File file{ native_handle };
        }

        expect_closed(native_handle);
    }

    TEST(FileTest, MoveConstructionTransfersOwnership) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::File::INVALID_HANDLE);
        iocpp::File source{ native_handle };

        iocpp::File destination{ std::move(source) };

        EXPECT_FALSE(source.Valid());
        EXPECT_EQ(source.NativeHandle(), iocpp::File::INVALID_HANDLE);
        EXPECT_TRUE(destination.Valid());
        EXPECT_EQ(destination.NativeHandle(), native_handle);
        expect_open(native_handle);
    }

    TEST(FileTest, MoveAssignmentClosesPreviousHandle) {
        auto const source_handle = open_null();
        ASSERT_NE(source_handle, iocpp::File::INVALID_HANDLE);
        auto const destination_handle = open_null();
        ASSERT_NE(destination_handle, iocpp::File::INVALID_HANDLE);
        iocpp::File source{ source_handle };
        iocpp::File destination{ destination_handle };

        destination = std::move(source);

        EXPECT_FALSE(source.Valid());
        EXPECT_EQ(destination.NativeHandle(), source_handle);
        expect_closed(destination_handle);
        expect_open(source_handle);
    }

    TEST(FileTest, ReleaseRelinquishesOwnership) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::File::INVALID_HANDLE);

        {
            iocpp::File file{ native_handle };

            EXPECT_EQ(file.Release(), native_handle);
            EXPECT_FALSE(file.Valid());
        }

        expect_open(native_handle);
        EXPECT_EQ(::close(native_handle), 0);
    }

    TEST(FileTest, ResetClosesPreviousAndAdoptsReplacement) {
        auto const previous_handle = open_null();
        ASSERT_NE(previous_handle, iocpp::File::INVALID_HANDLE);
        auto const replacement_handle = open_null();
        ASSERT_NE(replacement_handle, iocpp::File::INVALID_HANDLE);

        {
            iocpp::File file{ previous_handle };

            file.Reset(replacement_handle);

            expect_closed(previous_handle);
            expect_open(replacement_handle);
            EXPECT_EQ(file.NativeHandle(), replacement_handle);
        }

        expect_closed(replacement_handle);
    }

    TEST(FileTest, ResetWithOwnedHandleIsNoOp) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::File::INVALID_HANDLE);
        iocpp::File file{ native_handle };

        file.Reset(native_handle);

        EXPECT_EQ(file.NativeHandle(), native_handle);
        expect_open(native_handle);
    }

    TEST(FileTest, ResetCanBeRepeated) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::File::INVALID_HANDLE);
        iocpp::File file{ native_handle };

        file.Reset();
        file.Reset();

        EXPECT_FALSE(file.Valid());
        EXPECT_EQ(file.NativeHandle(), iocpp::File::INVALID_HANDLE);
        expect_closed(native_handle);
    }

} // namespace
