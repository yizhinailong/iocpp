import std;
import iocpp;

auto main() -> int {
    iocpp::ThreadContext thread_context;
    iocpp::Io io{ thread_context };

    auto const timestamp = io.Now().time_since_epoch();
    std::println(
        "io.Now(): {} ns",
        std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp).count()
    );

    io.Sleep(std::chrono::milliseconds(100));

    return 0;
}
