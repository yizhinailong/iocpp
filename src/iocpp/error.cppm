export module iocpp.error;

import std;

export namespace iocpp {

    enum class Operation {
        Sleep
    };

    enum class ErrorCode {
        InvalidArgument,
        IoError,
        Unsupported
    };

    struct Error {
        ErrorCode code;
        Operation operation;
    };

    template <typename T>
    using Result = std::expected<T, Error>;

} // namespace iocpp
