#include <cerrno>

#include <fcntl.h>

import std;
import iocpp;

auto main() -> int {
    auto const native_handle = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    if (native_handle == iocpp::Io::File::INVALID_HANDLE) {
        std::println("failed to open /dev/null (errno {})", errno);
        return 1;
    }

    iocpp::Io::Threaded threaded;
    iocpp::Io io{ threaded };
    auto source = io.AdoptFile(native_handle);
    iocpp::Io::File destination{ std::move(source) };

    std::println("source valid: {}", source.Valid());
    std::println("destination fd: {}", destination.NativeHandle());

    return source.Valid() || !destination.Valid() ? 1 : 0;
}
