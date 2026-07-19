export module iocpp.error;

import std;

export namespace iocpp::detail {

    enum class Operation {
        Sleep
    };

    enum class ErrorCode {
        Unsupported
    };

    struct Error {
        ErrorCode code;
        Operation operation;
    };

    template <typename T>
    using Result = std::expected<T, Error>;

} // namespace iocpp::detail
