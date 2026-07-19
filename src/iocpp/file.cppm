module;

#include <unistd.h>

export module iocpp.file;

import std;

export namespace iocpp {

    class File {
    public:
        static constexpr int INVALID_HANDLE{ -1 };

        File() noexcept = default;

        explicit File(int native_handle) noexcept
            : m_native_handle(native_handle) {}

        File(File const&) = delete;

        auto operator=(File const&) -> File& = delete;

        File(File&& other) noexcept
            : m_native_handle(other.Release()) {}

        auto operator=(File&& other) noexcept -> File& {
            if (this != &other) {
                this->Reset(other.Release());
            }

            return *this;
        }

        ~File() noexcept {
            this->Reset();
        }

        [[nodiscard]]
        auto Valid() const noexcept -> bool {
            return m_native_handle != INVALID_HANDLE;
        }

        [[nodiscard]]
        auto NativeHandle() const noexcept -> int {
            return m_native_handle;
        }

        [[nodiscard]]
        auto Release() noexcept -> int {
            return std::exchange(m_native_handle, INVALID_HANDLE);
        }

        auto Reset(int native_handle = INVALID_HANDLE) noexcept -> void {
            if (m_native_handle == native_handle) {
                return;
            }

            auto const previous_handle = this->Release();
            if (previous_handle != INVALID_HANDLE) {
                static_cast<void>(::close(previous_handle));
            }

            m_native_handle = native_handle;
        }

    private:
        int m_native_handle{ INVALID_HANDLE };
    };

} // namespace iocpp
