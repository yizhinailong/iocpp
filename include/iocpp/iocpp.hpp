#pragma once

#include <print>

namespace iocpp {
    auto version() -> void {
        std::println("0.1.0");
    }
} // namespace iocpp
