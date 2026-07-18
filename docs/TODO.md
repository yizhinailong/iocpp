# iocpp Linux 最小可用版本 TODO

本文档把 `iocpp` 的第一个最小可用版本拆成尽量独立、可测试、适合单独提交的任务。项目统一使用 C++23；首要目标是尽快形成一条可实际使用的 Linux 文件 I/O 与并发调用链；CI 矩阵、安装包和完整发布工程不阻塞首个可用版本。文中所有路径均相对于 iocpp 项目根目录。

项目当前处于独立仓库的初始化阶段，已有 C++ 源码目录、`mcpp.toml`、`README.md`、`LICENSE`、`.gitignore`、`docs/TODO.md` 和适用于 iocpp 的 GitHub Issue Template。

## 1. MVP 目标

第一个版本只支持 Linux，使用阻塞系统调用和固定大小线程池，实现以下纵向功能链：

```text
io_context
  -> threaded_backend
      -> open/close/pread/pwrite
      -> async/concurrent
      -> task wait/cancel
      -> cancelable sleep
```

最小可用版本完成标准：

- 能在 Linux x86_64 上使用 GCC 和 Clang 构建。
- 能创建、写入、读取和关闭普通文件。
- 能在线程池中执行有返回值或无返回值的任务。
- `async` 在并发资源不足时能够同步回退。
- `concurrent` 在并发资源不足时返回明确错误。
- 能取消尚未开始的任务。
- 能取消正在等待的 `sleep_for`。
- 基础测试在普通 Debug 构建、AddressSanitizer 和 UndefinedBehaviorSanitizer 下通过。
- 提供可复制运行的 mcpp 构建命令和完整文件异步示例。
- 不要求支持网络、io_uring、epoll、目录遍历、进程或跨平台。

ThreadSanitizer、Valgrind、完整 CI 矩阵、发布打包和 mcpp package consumer 测试属于发布前强化，不阻塞首个可用版本。

## 2. 已确定的基础设计

- [ ] 使用命名空间 `iocpp`，不向 `namespace std` 添加声明。
- [ ] 第一版不依赖 Boost、Asio、liburing 或第三方线程池。
- [x] 项目使用 C++23，以 `std::expected<T, Error>` 作为结果类型，并通过 `Result<T>` 别名暴露。
- [ ] 第一版使用固定数量的工作线程。
- [ ] 以 Zig `std.Io` 的 `async`、`concurrent` 和 Future 行为作为语义参考；与本文档冲突时以本文档的固定线程池和 C++ API 决策为准。
- [ ] 普通文件读写优先使用 `pread`/`pwrite`，避免共享文件 offset。
- [ ] 文件对象使用 move-only RAII 所有权模型。
- [ ] `io_context` 是不拥有 backend 的轻量句柄。
- [ ] backend 必须比引用它的 `io_context` 和尚未结束的 `task` 活得更久；`file` 只拥有 native handle，不依赖 backend 生命周期。
- [ ] `task<T>::wait()` 返回能够表示成功、取消和 callable 异常的结果。
- [ ] `task<T>` 是单消费者对象，`wait()` 与 `cancel()` 不允许被多个线程并发调用。
- [ ] callable 异常以 `std::exception_ptr` 原样保存在 task 结果中。
- [ ] `task<T>` 析构时若任务尚未结束，则请求取消并等待结束。
- [ ] 运行中的普通 `pread`/`pwrite` 第一版不保证可中断。
- [ ] 取消采用协作式语义，不使用 `pthread_cancel` 或强制终止线程。
- [ ] `cancellation_source`、`cancellation_token` 和取消回调封装标准库 `std::stop_source`、`std::stop_token` 与 `std::stop_callback`。

## 3. 项目骨架

### 3.1 最小可用版本骨架

- [x] 使用 `mcpp.toml` 配置 C++23 构建。
- [ ] Debug 构建启用额外断言。

### 3.2 发布前工程配置

- [ ] 添加 `.clang-tidy` 基础配置。

## 4. 公共基础类型

### 4.1 错误模型

- [ ] 在 `src/iocpp/error.cppm` 中定义错误模型。
- [ ] 定义 `enum class errc`。
- [ ] 添加 `invalid_argument`。
- [ ] 添加 `bad_file_descriptor`。
- [ ] 添加 `file_not_found`。
- [ ] 添加 `permission_denied`。
- [ ] 添加 `already_exists`。
- [ ] 添加 `would_block`。
- [ ] 添加 `interrupted`。
- [ ] 添加 `io_error`。
- [ ] 添加 `no_space_left`。
- [ ] 添加 `file_too_large`。
- [ ] 添加 `resource_unavailable`。
- [ ] 添加 `concurrency_unavailable`。
- [ ] 添加 `cancelled`。
- [ ] 添加 `unknown`。
- [ ] 定义 `enum class operation`，记录失败操作类型。
- [ ] 为 `operation` 添加 `open_file`。
- [ ] 为 `operation` 添加 `close_file`。
- [ ] 为 `operation` 添加 `read_at`。
- [ ] 为 `operation` 添加 `write_at`。
- [ ] 为 `operation` 添加 `submit_task`。
- [ ] 为 `operation` 添加 `sleep`。
- [ ] 定义 `iocpp::error`，至少保存 `errc`、`operation` 和原始 `errno`。
- [ ] 为 `iocpp::error` 提供 `message()`。
- [ ] 为 `iocpp::error` 提供 `operator bool` 或等价检查接口。
- [ ] 实现从 Linux `errno` 到 `errc` 的集中映射函数。
- [ ] 保证未知 errno 不会被静默丢弃。
- [ ] 为 errno 映射编写表驱动单元测试。
- [ ] 明确 `iocpp::error` 只表示系统调用、参数、资源和任务提交错误，不承载 callable 抛出的 C++ 异常。

### 4.2 `Result<T>`（基于 `std::expected`）

- [ ] 在 `src/iocpp/error.cppm` 中定义结果类型。
- [ ] 定义 `template<typename T> using Result = std::expected<T, Error>`。
- [x] 验证 `Result<int>` 的成功值构造。
- [x] 验证通过 `std::unexpected<Error>` 构造失败结果。
- [x] 验证 `Result<void>` 的成功和失败结果。
- [x] 验证 `Result<std::unique_ptr<int>>` 等 move-only 值类型。
- [ ] 验证 `has_value()` 和显式 `operator bool()`。
- [ ] 验证 `value()`、`error()`、`operator*` 和 `operator->`。
- [ ] 验证错误访问抛出 `std::bad_expected_access<Error>`。

### 4.3 Buffer 类型约定

- [ ] 在 `src/iocpp.cppm` 中定义 buffer 类型。
- [ ] 定义可写 buffer 为 `std::span<std::byte>`。
- [ ] 定义只读 buffer 为 `std::span<const std::byte>`。
- [ ] 提供从 `std::span<char>` 转换到字节 span 的辅助函数。
- [ ] 提供从 `std::string_view` 转换到只读字节 span 的辅助函数。
- [ ] 明确所有同步文件操作只在调用期间借用 buffer。
- [ ] 明确异步 lambda 捕获 buffer 时由调用者负责生命周期。
- [ ] 为零长度 buffer 编写测试。

## 5. Linux 文件句柄

### 5.1 file 类型

- [ ] 在 `src/iocpp.cppm` 中定义 file 类型。
- [ ] 创建 `src/file_linux.cpp`。
- [ ] 定义无效 native handle 为 `-1`。
- [ ] 实现默认构造的空 `file`。
- [ ] 删除拷贝构造函数。
- [ ] 删除拷贝赋值运算符。
- [ ] 实现移动构造函数。
- [ ] 实现移动赋值运算符。
- [ ] 实现析构关闭句柄。
- [ ] 实现 `valid()`。
- [ ] 实现 `native_handle()`。
- [ ] 实现 `release()`，转移 native handle 所有权。
- [ ] 实现 `reset()`，关闭旧句柄并接管新句柄。
- [ ] 保证移动后的对象处于无效但可析构状态。
- [ ] 调用 `close` 前先使对象中的 fd 失效；Linux 上 `close` 返回 `EINTR` 时不重试，避免误关已复用的 fd。
- [ ] 明确析构中的 close 错误不会抛异常。
- [ ] 提供显式 `close()` 以便调用者获得关闭错误。
- [ ] 防止显式 close 后析构重复关闭。
- [ ] 为 move 构造和 move 赋值编写测试。
- [ ] 为 `release()` 编写所有权测试。
- [ ] 为重复 close 安全性编写测试。

### 5.2 打开文件

- [ ] 定义路径参数为 `std::string_view`，同步调用期间借用路径内容。
- [ ] 定义 `open_mode::read_only`。
- [ ] 定义 `open_mode::write_only`。
- [ ] 定义 `open_mode::read_write`。
- [ ] 定义 `open_options::create`。
- [ ] 定义 `open_options::truncate`。
- [ ] 定义 `open_options::exclusive`。
- [ ] 定义创建权限默认值 `0644`。
- [ ] 将 `open_options` 映射为 Linux `O_*` 标志。
- [ ] 默认添加 `O_CLOEXEC`。
- [ ] 使用 `openat(AT_FDCWD, ...)` 实现打开文件。
- [ ] 拒绝互相冲突的选项组合。
- [ ] 保留原始 errno。
- [ ] 测试打开不存在文件。
- [ ] 测试创建新文件。
- [ ] 测试 exclusive 创建已存在文件。
- [ ] 测试 truncate。
- [ ] 测试只读文件写入失败。
- [ ] 测试包含空字节的路径被拒绝。
- [ ] 测试空路径行为。

### 5.3 位置读写

- [ ] 定义 `read_at(file&, writable_buffer, uint64_t offset)`。
- [ ] 定义 `write_at(file&, readonly_buffer, uint64_t offset)`。
- [ ] 检查无效文件句柄。
- [ ] 检查 offset 是否能安全转换为 `off_t`。
- [ ] 使用 `pread` 实现单 buffer 位置读取。
- [ ] 使用 `pwrite` 实现单 buffer 位置写入。
- [ ] 对 `EINTR` 自动重试。
- [ ] 保留短读语义。
- [ ] 保留短写语义。
- [ ] 将 EOF 表示为成功读取 0 字节。
- [ ] 不把 0 字节读取当作错误。
- [ ] 处理零长度 buffer，不执行 syscall。
- [ ] 实现 `read_at_all` 辅助函数。
- [ ] 实现 `write_at_all` 辅助函数。
- [ ] 防止 `write_at_all` 在连续零进度时无限循环。
- [ ] 测试基础位置写入和读取。
- [ ] 测试非零 offset。
- [ ] 测试越过 EOF 读取。
- [ ] 测试稀疏文件写入。
- [ ] 测试短读处理。
- [ ] 测试并发位置读取不会共享 seek offset。
- [ ] 测试大于一页的数据。
- [ ] 测试文件描述符关闭后的错误映射。

## 6. backend 与 io_context

### 6.1 backend 接口

- [ ] 在 `src/iocpp.cppm` 中定义 io_backend 接口。
- [ ] 定义 `io_backend` 抽象基类。
- [ ] 为 `io_backend` 添加虚析构函数。
- [ ] 添加 `open_file` 虚函数。
- [ ] 添加 `read_at` 虚函数。
- [ ] 添加 `write_at` 虚函数。
- [ ] 添加任务提交的内部虚函数。
- [ ] 添加查询并发容量的内部接口。
- [ ] 添加可取消 sleep 的内部虚函数。
- [ ] 添加 backend shutdown 接口。
- [ ] 明确所有虚函数的线程安全要求。
- [ ] 明确 backend shutdown 后各函数的行为。
- [ ] 禁止复制 backend。
- [ ] 允许 backend 在固定地址上构造和使用。

### 6.2 io_context

- [ ] 在 `src/iocpp.cppm` 中定义 io_context 接口。
- [ ] 实现从 `io_backend&` 构造 `io_context`。
- [ ] 允许复制和移动 `io_context`。
- [ ] 添加 `backend()` 内部访问器。
- [ ] 添加 `open_file` 转发函数。
- [ ] 添加 `read_at` 转发函数。
- [ ] 添加 `write_at` 转发函数。
- [ ] 添加 `read_at_all` 高层循环。
- [ ] 添加 `write_at_all` 高层循环。
- [ ] 保证 `io_context` 不负责 backend 生命周期。
- [ ] 在文档中给出正确和错误的生命周期示例。
- [ ] 编写 fake backend，验证调用确实经过 backend 分发。
- [ ] 测试复制后的 `io_context` 指向同一 backend。

## 7. 取消基础设施

### 7.1 cancellation_source/token

- [ ] 在 `src/iocpp.cppm` 中定义 cancellation 类型。
- [ ] 定义 `cancellation_source`。
- [ ] 定义轻量、可复制的 `cancellation_token`。
- [ ] 使用 `std::stop_source` 和 `std::stop_token` 保存和共享取消状态。
- [ ] 实现 `request_cancel()`。
- [ ] 保证 `request_cancel()` 幂等。
- [ ] 实现 `stop_requested()`。
- [ ] 实现纯取消点 `check_cancelled()`。
- [ ] 规定空 token 永不取消。
- [ ] 规定 source 析构不会自动请求取消。
- [ ] 提供基于 `std::stop_callback` 的取消回调包装，不自行维护回调链表。
- [ ] 记录取消回调可能在注册线程或请求取消的线程中同步执行。
- [ ] 测试取消前后的 token 状态。
- [ ] 测试多次 request_cancel。
- [ ] 测试多个 token 观察同一 source。
- [ ] 测试取消前后注册回调的行为。

### 7.2 取消语义文档

- [ ] 记录取消是请求而不是强制终止。
- [ ] 记录 callable 可以忽略取消请求并正常完成。
- [ ] 记录 queued task 可以在执行前被取消。
- [ ] 记录普通文件 syscall 第一版不保证中断。
- [ ] 记录取消任务后仍必须等待其资源释放。
- [ ] 将 queued -> running 状态转换定义为取消竞争的线性化点：转换前取消得到 `cancelled`，转换后取消仅作为协作请求，最终结果由 callable 决定。
- [ ] 记录运行中的 callable 可以忽略取消并正常返回成功结果。

## 8. task 状态机

### 8.1 类型设计

- [ ] 在 `src/iocpp.cppm` 中定义 task 类型。
- [ ] 创建内部 `task_state_base`。
- [ ] 创建模板 `task_state<T>`。
- [ ] 创建 `task_state<void>` 特化。
- [ ] 定义状态 `queued`。
- [ ] 定义状态 `running`。
- [ ] 定义状态 `completed`。
- [ ] 定义状态 `cancelled`。
- [ ] 使用 mutex 保护非原子复合状态。
- [ ] 使用 condition variable 实现等待。
- [ ] 在 task state 中保存 cancellation source。
- [ ] 在 task state 中保存成功结果。
- [ ] 在 task state 中保存 `std::exception_ptr`。
- [ ] 定义 `task_error`，区分取消、callable 异常和无效 task。
- [ ] 定义 `task_result<T>`。
- [ ] 定义 `task_result<void>`。
- [ ] 明确 `task_result<T>` 与普通 `Result<T>` 分离：前者保存 task 终态，后者保存操作和提交错误。

### 8.2 task<T> API

- [ ] 实现 move-only、单消费者 `task<T>`。
- [ ] 删除 task 拷贝构造和拷贝赋值。
- [ ] 实现 move 构造。
- [ ] 实现 move 赋值。
- [ ] 实现 `valid()`。
- [ ] 实现 `ready()`。
- [ ] 实现阻塞 `wait()`，成功返回后取出结果并使 task 失效。
- [ ] 实现 `wait_for()`，超时时不消费结果。
- [ ] 实现 `cancel()`。
- [ ] 让 `cancel()` 幂等地请求取消并等待任务结束，但不消费结果，之后仍允许调用一次 `wait()`。
- [ ] 实现 `token()`，允许查看同一取消状态。
- [ ] 禁止多次 `wait()`；第二次调用或在已消费 task 上调用返回无效 task 错误。
- [ ] 允许多次 `cancel()`，每次都等待任务进入终态。
- [ ] 规定 `wait()`、`wait_for()` 和 `cancel()` 不支持多个线程并发调用同一 task。
- [ ] 处理空 task 上调用成员函数的行为。
- [ ] 实现析构时取消并等待。
- [ ] 防止 task 在自己的 callable 内析构导致自等待死锁。
- [ ] 添加 Debug 断言检测自等待。

### 8.3 task 执行和竞态

- [ ] 实现 queued -> running 转换。
- [ ] 实现 running -> completed 转换。
- [ ] 实现 queued -> cancelled 转换。
- [ ] 定义取消与 queued -> running 竞争的线性化点。
- [ ] 确保结果写入发生在完成通知之前。
- [ ] 确保等待线程看到完整结果。
- [ ] 捕获 callable 抛出的所有异常。
- [ ] 保证 worker 不会因用户异常退出。
- [ ] 确保完成时唤醒正在执行 `wait()` 或 `wait_for()` 的单个消费者。
- [ ] 防止任务完成两次。
- [ ] 防止 packaged callable 执行两次。
- [ ] 测试返回 `int` 的任务。
- [ ] 测试返回 move-only 对象的任务。
- [ ] 测试返回 `void` 的任务。
- [ ] 测试 callable 抛异常。
- [ ] 测试执行前取消。
- [ ] 测试执行中请求取消但 callable 忽略请求。
- [ ] 测试完成与取消同时发生。
- [ ] 测试 `cancel()` 后仍可调用一次 `wait()` 取得终态。
- [ ] 测试多次 `cancel()`。
- [ ] 测试第二次 `wait()` 返回无效 task 错误。
- [ ] 测试 task 移动后等待。
- [ ] 测试未显式 wait 的 task 析构。

## 9. 固定线程池 threaded_backend

### 9.1 生命周期

- [ ] 在 `src/iocpp.cppm` 中定义 threaded_backend。
- [ ] 创建 `src/threaded_backend.cpp`。
- [ ] 定义 `threaded_backend_options`。
- [ ] 添加 `worker_count`。
- [ ] 添加 `async_limit`。
- [ ] 添加 `concurrent_limit`。
- [ ] `worker_count == 0` 时构造函数抛出 `std::invalid_argument`。
- [ ] 默认 worker 数使用 `std::thread::hardware_concurrency()`。
- [ ] `hardware_concurrency()` 返回 0 时默认使用 1 个 worker。
- [ ] `async_limit` 和 `concurrent_limit` 默认等于 `worker_count`，并允许调用者显式设置更大的排队容量。
- [ ] backend 构造时创建 worker。
- [ ] 部分 worker 创建失败时停止并 join 已创建线程，然后抛出 `std::system_error`。
- [ ] backend 析构时开始 shutdown。
- [ ] shutdown 后停止接收线程池任务；`async` 改为 eager 执行，`concurrent` 返回错误。
- [ ] shutdown 时唤醒所有 worker。
- [ ] shutdown 时取消尚未开始的任务。
- [ ] shutdown 时等待正在运行的任务。
- [ ] join 所有 worker。
- [ ] 保证显式 shutdown 幂等。
- [ ] 防止 worker 线程调用 shutdown 后 join 自己。

### 9.2 任务队列

- [ ] 定义内部 move-only work item。
- [ ] 使用 mutex 保护任务队列。
- [ ] 使用 condition variable 唤醒 worker。
- [ ] 使用 FIFO 作为第一版调度顺序。
- [ ] 记录 queued task 数量。
- [ ] 记录 running task 数量。
- [ ] 定义并记录共享的 outstanding task 数量，即 `queued + running`。
- [ ] eager 同步执行的任务不计入线程池 outstanding 数量。
- [ ] 提供仅测试使用的统计快照。
- [ ] worker 在无任务时阻塞而不是忙等。
- [ ] worker 被虚假唤醒时重新检查条件。
- [ ] worker 取出任务后在队列锁外执行 callable。
- [ ] callable 完成后更新计数并通知容量等待者。
- [ ] 防止计数上溢和下溢。
- [ ] 测试 FIFO 行为。
- [ ] 测试多个 producer 并发提交。
- [ ] 测试 backend 空闲时不消耗一个 CPU 核。

### 9.3 async 提交规则

- [ ] 在 `io_context` 添加 `async` 模板函数。
- [ ] 支持无参数 lambda。
- [ ] 支持捕获 move-only 值的 lambda。
- [ ] 支持返回值 callable。
- [ ] 支持返回 void callable。
- [ ] 检测 callable 是否可接收 `cancellation_token`。
- [ ] 若 callable 同时支持有 token 和无参数调用，优先传入任务 token。
- [ ] 若可接收 token，则执行时传入任务 token。
- [ ] 若不可接收 token，则以无参数形式调用。
- [ ] 在共享 outstanding 数量低于 `async_limit` 时提交线程池。
- [ ] 在线程池已 shutdown 时同步 eager 执行 callable。
- [ ] 分配 task state 失败时允许 `std::bad_alloc` 从 `async` 传播，因为此时无法构造有效 task。
- [ ] 达到 `async_limit` 时在调用线程 eager 执行。
- [ ] eager 执行也必须返回有效且已完成的 task。
- [ ] eager 执行不增加 queued、running 或 outstanding 计数。
- [ ] eager callable 抛异常时捕获到 task state。
- [ ] eager 执行与 worker 执行使用同一套 callable 调用和异常捕获逻辑。
- [ ] 测试正常异步提交。
- [ ] 测试达到限制后的 eager 回退。
- [ ] 测试 eager 返回值。
- [ ] 测试 eager callable 异常。

### 9.4 concurrent 提交规则

- [ ] 在 `io_context` 添加 `concurrent` 模板函数。
- [ ] 返回 `result<task<T>>`。
- [ ] 共享 outstanding 数量低于 `concurrent_limit` 时提交。
- [ ] 达到 `concurrent_limit` 时返回 `concurrency_unavailable`。
- [ ] concurrent 不允许同步 eager 执行。
- [ ] backend shutdown 后 concurrent 返回 `concurrency_unavailable`。
- [ ] 分配 task state 或 work item 失败时返回 `resource_unavailable`。
- [ ] 提交成功后保证任务最终由 worker 执行或在 shutdown 时取消。
- [ ] 测试 concurrent 成功。
- [ ] 测试 concurrent 容量耗尽。
- [ ] 测试 concurrent 不在调用线程执行。
- [ ] 测试释放容量后可以再次提交。
- [ ] 测试 concurrent 与 async 同时争用容量。
- [ ] 测试 async 与 concurrent 使用同一个 outstanding 计数，但分别应用各自的 limit。

## 10. 可取消 sleep

- [ ] 在 `src/iocpp.cppm` 中定义时间相关类型。
- [ ] 创建 `src/time_linux.cpp`。
- [ ] 定义 `sleep_for(io_context, duration, cancellation_token)`。
- [ ] 定义 `sleep_until(io_context, steady_clock::time_point, cancellation_token)`。
- [ ] 使用单调时钟语义。
- [ ] 正确处理零时长。
- [ ] 正确处理负时长。
- [ ] 等待前检查取消状态。
- [ ] 使用 condition variable 和取消回调唤醒 sleep。
- [ ] 超时与取消同时发生时允许返回正常到期或取消，但不得丢失唤醒或发生死锁。
- [ ] 返回结果区分正常到期和取消。
- [ ] 不为每次 sleep 创建额外线程。
- [ ] 测试短时间 sleep。
- [ ] 测试零时长 sleep。
- [ ] 测试 sleep 前取消。
- [ ] 测试 sleep 过程中取消。
- [ ] 测试多个任务同时 sleep。
- [ ] 测试取消延迟在合理范围内。

## 11. 第一条完整调用链

- [ ] 编写示例 `examples/file_async.cpp`。
- [ ] 创建 `threaded_backend`。
- [ ] 从 backend 创建 `io_context`。
- [ ] 创建临时文件。
- [ ] 使用 `write_at_all` 写入已知字符串。
- [ ] 使用 `concurrent` 启动位置读取任务。
- [ ] 在任务中调用 `read_at_all`。
- [ ] 等待 task 完成。
- [ ] 校验读取内容。
- [ ] 显式关闭文件并检查结果。
- [ ] 删除临时文件。
- [ ] 给示例添加 `mcpp test` smoke test。

## 12. 最小可用版本集成测试

- [ ] 创建统一临时目录测试辅助类。
- [ ] 保证测试失败时也能清理临时文件。
- [ ] 测试两个文件任务并发读取。
- [ ] 测试一个任务写入后另一个任务读取。
- [ ] 测试 1000 个短任务。
- [ ] 测试多线程同时调用 `async`。
- [ ] 测试多线程同时调用 `concurrent`。
- [ ] 测试容量限制严格生效。
- [ ] 测试 backend 析构时存在 queued task。
- [ ] 测试 backend 析构时存在 running task。
- [ ] 测试 task 比 io_context 更早析构。
- [ ] 运行 ASan 测试套件。
- [ ] 运行 UBSan 测试套件。
- [ ] 修复最小可用版本范围内的所有 ASan 和 UBSan 报告。

### 12.1 发布前并发与内存强化

- [ ] 使用 Debug 断言测试可检测的 backend 生命周期误用。
- [ ] 运行 TSan 测试套件。
- [ ] 修复所有 TSan 报告。
- [ ] 使用 `valgrind --leak-check=full` 做一次补充检查。
- [ ] 连续运行并发测试至少 100 次。

## 13. API 与文档

- [ ] 将 README 从一句话占位内容扩展为完整项目介绍。
- [ ] 写明项目目标是借鉴 Zig `std.Io` 的能力注入和并发语义。
- [ ] 写明当前仅支持 Linux。
- [ ] 写明当前使用阻塞 syscall 和线程池。
- [ ] 写明当前不支持 io_uring。
- [ ] 写明 backend 生命周期规则。
- [ ] 写明 buffer 生命周期规则。
- [ ] 写明 task 析构可能阻塞。
- [ ] 写明取消是协作式的。
- [ ] 写明普通文件 syscall 不保证中断。
- [ ] 记录 `async` 与 `concurrent` 的区别。
- [ ] 添加构建命令。
- [ ] 添加运行测试命令。
- [ ] 添加 sanitizer 构建命令。
- [ ] 添加最小文件 I/O 示例。
- [ ] 添加异步任务示例。
- [ ] 确保 `iocpp` 模块可以被最小 consumer 独立导入。

### 13.1 发布前文档完善

- [ ] 为所有公共类和函数添加 Doxygen 注释。

## 14. 发布前 CI、安装与 API 冻结

本节不阻塞首个最小可用版本，在完整纵向功能链和基础测试完成后实施。

- [ ] 添加 GCC Debug CI 配置。
- [ ] 添加 GCC Release CI 配置。
- [ ] 添加 Clang Debug CI 配置。
- [ ] 添加 Clang Release CI 配置。
- [ ] 添加 ASan + UBSan CI 配置。
- [ ] 添加 TSan CI 配置。
- [ ] 在 CI 中运行 clang-format 检查。
- [ ] 在 CI 中运行 clang-tidy。
- [ ] 在 CI 中运行全部 `mcpp test` 测试。
- [ ] 确认公共模块接口不依赖未导出的私有声明。
- [ ] 确认发布后的 mcpp package 可以被外部项目声明为依赖。
- [ ] 完善 `mcpp.toml` 发布元数据。
- [ ] 使用 `mcpp publish --dry-run` 验证发布内容。
- [ ] 使用 `mcpp pack` 验证发布归档。
- [ ] 添加最小外部 consumer 测试。
- [ ] 检查 GitHub、Conan 和 vcpkg 中的 `iocpp` 名称冲突。
- [ ] 确认许可证和第三方声明。
- [ ] 标记 `v0.1.0` 前冻结第一版公共 API。

## 15. 明确推迟到 MVP 之后

- [ ] `open_options::append`；在定义它与位置写入 `pwrite` 的交互规则后再加入。
- [ ] 支持多个线程并发等待同一个 `task<T>` 的共享任务类型。
- [ ] `reader`/`writer` 缓冲接口。
- [ ] scatter/gather `preadv`/`pwritev`。
- [ ] 流式 `read`/`write`。
- [ ] TCP 和 Unix domain socket。
- [ ] DNS 解析。
- [ ] `task_group`。
- [ ] `select`。
- [ ] `io_batch`。
- [ ] semaphore、event、condition 等高层同步原语。
- [ ] mmap。
- [ ] 目录遍历和文件系统修改操作。
- [ ] 子进程管理。
- [ ] 安全随机数接口。
- [ ] io_uring backend。
- [ ] epoll backend。
- [ ] kqueue backend。
- [ ] Windows IOCP backend。
- [ ] 无异常构建模式。
- [ ] 自定义 allocator 或 `std::pmr` 支持。
- [ ] coroutine `co_await` 支持。
- [ ] work stealing。
- [ ] 动态增长线程池。
- [ ] NUMA 和 CPU affinity 优化。
- [ ] 性能 benchmark 与其他 I/O 库对比。

## 16. 推荐实施顺序

按以下顺序推进，每完成一项都保持 mcpp 配置、构建和已有测试可运行：

1. 项目骨架和编译选项。
2. `Error` 与基于 `std::expected` 的 `Result<T>`。
3. move-only `file`。
4. 同步 `open_file`、`read_at`、`write_at`。
   完成此步后形成第一个同步文件 I/O 可用里程碑。
5. `io_backend` 与 `io_context` 转发。
6. 基于标准库 stop token 的 cancellation source/token。
7. 单消费者 `task<T>` 状态机，不接线程池，先用测试直接驱动。
8. 固定线程池和 FIFO 任务队列。
9. `async` eager 回退。
10. `concurrent` 容量保证。
11. 可取消 `sleep_for`。
12. 文件异步完整示例、README 使用说明、ASan 和 UBSan。
    完成此步后形成完整的最小可用版本。
13. TSan、Valgrind、并发压力测试和 CI。
14. Doxygen、安装规则、外部 consumer 测试和 `v0.1.0` API 冻结。
