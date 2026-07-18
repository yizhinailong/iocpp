import std;
import iocpp.error;

namespace {

    auto expect(
        bool condition,
        std::source_location location = std::source_location::current()
    ) -> void {
        if (condition) {
            return;
        }

        std::println(
            "expectation failed at {}:{}",
            location.file_name(),
            location.line()
        );
        std::abort();
    }

} // namespace

auto main() -> int {
    auto const error = iocpp::Error{
        .code = iocpp::ErrorCode::Unsupported,
        .operation = iocpp::Operation::Sleep
    };
    expect(error.code == iocpp::ErrorCode::Unsupported);
    expect(error.operation == iocpp::Operation::Sleep);

    iocpp::Result<int> success{ 42 };
    expect(success.has_value());
    expect(success.value() == 42);

    iocpp::Result<int> failure = std::unexpected<iocpp::Error>{ error };
    expect(!failure.has_value());
    expect(failure.error().code == iocpp::ErrorCode::Unsupported);
    expect(failure.error().operation == iocpp::Operation::Sleep);

    iocpp::Result<void> void_success{};
    expect(void_success.has_value());

    iocpp::Result<void> void_failure = std::unexpected<iocpp::Error>{ error };
    expect(!void_failure.has_value());

    iocpp::Result<std::unique_ptr<int>> move_only{ std::make_unique<int>(42) };
    expect(move_only.has_value());
    expect(**move_only == 42);

    auto moved_value = std::move(move_only).value();
    expect(*moved_value == 42);

    return 0;
}
