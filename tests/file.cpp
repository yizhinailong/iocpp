#include <cerrno>

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

import std;
import iocpp;

namespace {

    static_assert(!std::is_copy_constructible_v<iocpp::Io::File>);
    static_assert(!std::is_copy_assignable_v<iocpp::Io::File>);
    static_assert(std::is_nothrow_move_constructible_v<iocpp::Io::File>);
    static_assert(std::is_nothrow_move_assignable_v<iocpp::Io::File>);
    static_assert(std::is_nothrow_destructible_v<iocpp::Io::File>);

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

    struct StubIoContext final : iocpp::IoContext {
        auto Now() const -> iocpp::Io::Timestamp override {
            return {};
        }

        auto Sleep(iocpp::Io::Duration) -> void override {}
    };

    class FileTest : public ::testing::Test {
    protected:
        auto adoptFile(int native_handle) -> iocpp::Io::File {
            return m_io.AdoptFile(native_handle);
        }

        StubIoContext m_context;
        iocpp::Io m_io{ m_context };
    };

    TEST_F(FileTest, DefaultConstructsEmpty) {
        iocpp::Io::File file;

        EXPECT_FALSE(file.Valid());
        EXPECT_EQ(file.NativeHandle(), iocpp::Io::File::INVALID_HANDLE);
    }

    TEST_F(FileTest, AdoptsNativeHandle) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::Io::File::INVALID_HANDLE);

        auto file = this->adoptFile(native_handle);

        EXPECT_TRUE(file.Valid());
        EXPECT_EQ(file.NativeHandle(), native_handle);
        expect_open(native_handle);
    }

    TEST_F(FileTest, DestructorClosesNativeHandle) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::Io::File::INVALID_HANDLE);

        {
            auto file = this->adoptFile(native_handle);
        }

        expect_closed(native_handle);
    }

    TEST_F(FileTest, MoveConstructionTransfersOwnership) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::Io::File::INVALID_HANDLE);
        auto source = this->adoptFile(native_handle);

        iocpp::Io::File destination{ std::move(source) };

        EXPECT_FALSE(source.Valid());
        EXPECT_EQ(source.NativeHandle(), iocpp::Io::File::INVALID_HANDLE);
        EXPECT_TRUE(destination.Valid());
        EXPECT_EQ(destination.NativeHandle(), native_handle);
        expect_open(native_handle);
    }

    TEST_F(FileTest, MoveAssignmentClosesPreviousHandle) {
        auto const source_handle = open_null();
        ASSERT_NE(source_handle, iocpp::Io::File::INVALID_HANDLE);
        auto const destination_handle = open_null();
        ASSERT_NE(destination_handle, iocpp::Io::File::INVALID_HANDLE);
        auto source = this->adoptFile(source_handle);
        auto destination = this->adoptFile(destination_handle);

        destination = std::move(source);

        EXPECT_FALSE(source.Valid());
        EXPECT_EQ(destination.NativeHandle(), source_handle);
        expect_closed(destination_handle);
        expect_open(source_handle);
    }

    TEST_F(FileTest, ReleaseRelinquishesOwnership) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::Io::File::INVALID_HANDLE);

        {
            auto file = this->adoptFile(native_handle);

            EXPECT_EQ(file.Release(), native_handle);
            EXPECT_FALSE(file.Valid());
        }

        expect_open(native_handle);
        EXPECT_EQ(::close(native_handle), 0);
    }

    TEST_F(FileTest, ResetClosesPreviousAndAdoptsReplacement) {
        auto const previous_handle = open_null();
        ASSERT_NE(previous_handle, iocpp::Io::File::INVALID_HANDLE);
        auto const replacement_handle = open_null();
        ASSERT_NE(replacement_handle, iocpp::Io::File::INVALID_HANDLE);

        {
            auto file = this->adoptFile(previous_handle);

            file.Reset(replacement_handle);

            expect_closed(previous_handle);
            expect_open(replacement_handle);
            EXPECT_EQ(file.NativeHandle(), replacement_handle);
        }

        expect_closed(replacement_handle);
    }

    TEST_F(FileTest, ResetWithOwnedHandleIsNoOp) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::Io::File::INVALID_HANDLE);
        auto file = this->adoptFile(native_handle);

        file.Reset(native_handle);

        EXPECT_EQ(file.NativeHandle(), native_handle);
        expect_open(native_handle);
    }

    TEST_F(FileTest, ResetCanBeRepeated) {
        auto const native_handle = open_null();
        ASSERT_NE(native_handle, iocpp::Io::File::INVALID_HANDLE);
        auto file = this->adoptFile(native_handle);

        file.Reset();
        file.Reset();

        EXPECT_FALSE(file.Valid());
        EXPECT_EQ(file.NativeHandle(), iocpp::Io::File::INVALID_HANDLE);
        expect_closed(native_handle);
    }

} // namespace
