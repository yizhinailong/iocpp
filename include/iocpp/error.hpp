#pragma once

namespace iocpp {
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
} // namespace iocpp
