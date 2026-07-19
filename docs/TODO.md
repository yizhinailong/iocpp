# iocpp Linux 最小可用版本 TODO

本文档把 `iocpp` 的第一个最小可用版本拆成尽量独立、可测试、适合单独提交的任务。项目统一使用 C++23；首要目标是尽快形成一条可实际使用的 Linux 文件 I/O 与并发调用链；CI 矩阵、安装包和完整发布工程不阻塞首个可用版本。文中所有路径均相对于 iocpp 项目根目录。

项目当前处于独立仓库的初始化阶段，已有 C++ 源码目录、`mcpp.toml`、`README.md`、`LICENSE`、`.gitignore`、`docs/TODO.md`、`docs/ARCHIVE.md` 和适用于 iocpp 的 GitHub Issue Template。

已经实现的功能统一移至 [`ARCHIVE.md`](ARCHIVE.md)。本文档只保留尚未完成的工作。

## 1. MVP 目标

第一个版本只支持 Linux，使用阻塞系统调用和固定大小线程池，实现以下纵向功能链：

```text
Io
  -> Io::Threaded
      -> OpenFile/Close/ReadAt/WriteAt
      -> Async/Concurrent
      -> Io::Task<T> Wait/Cancel
      -> cancelable Sleep
```

最小可用版本完成标准：

- 能在 Linux x86_64 上使用 GCC 和 Clang 构建。
- 能创建、写入、读取和关闭普通文件。
- 能在线程池中执行有返回值或无返回值的任务。
- `Io::Async()` 在并发资源不足时能够同步回退。
- `Io::Concurrent()` 在并发资源不足时返回明确错误。
- 能取消尚未开始的任务。
- 能取消正在等待的 `Io::Sleep()`。
- 基础测试在普通 Debug 构建、AddressSanitizer 和 UndefinedBehaviorSanitizer 下通过。
- 提供可复制运行的 mcpp 构建命令和完整文件异步示例。
- 不要求支持网络、io_uring、epoll、目录遍历、进程或跨平台。

ThreadSanitizer、Valgrind、完整 CI 矩阵、发布打包和 mcpp package consumer 测试属于发布前强化，不阻塞首个可用版本。

## 2. 已确定的基础设计

- [x] 使用命名空间 `iocpp`，不向 `namespace std` 添加声明。
- [x] 公共业务类型统一放在 `iocpp::Io` 下，需要后端能力的操作统一由 `Io` 实例转发；顶层 `IoContext` 仅作为后端扩展接口。
- [ ] 第一版不依赖 Boost、Asio、liburing 或第三方线程池。
- [ ] 第一版使用固定数量的工作线程。
- [ ] 以 Zig `std.Io` 的 `async`、`concurrent` 和 Future 行为作为语义参考；C++ 入口对应为 `Io::Async()`、`Io::Concurrent()` 和 `Io::Task<T>`，与本文档冲突时以本文档的固定线程池和 C++ API 决策为准。
- [ ] 普通文件读写优先使用 `pread`/`pwrite`，避免共享文件 offset。
- [ ] 文件对象使用 move-only RAII 所有权模型。
- [x] `Io` 是不拥有 `IoContext` 实现的轻量句柄。
- [ ] `IoContext` 实现必须比引用它的 `Io` 和尚未结束的 `Io::Task<T>` 活得更久；`Io::File` 只拥有 native handle，不依赖后端生命周期。
- [ ] `Io::Task<T>::Wait()` 返回能够表示成功、取消和 callable 异常的结果。
- [ ] `Io::Task<T>` 是单消费者对象，`Wait()` 与 `Cancel()` 不允许被多个线程并发调用。
- [ ] callable 异常以 `std::exception_ptr` 原样保存在 task 结果中。
- [ ] `Io::Task<T>` 析构时若任务尚未结束，则请求取消并等待结束。
- [ ] 运行中的普通 `pread`/`pwrite` 第一版不保证可中断。
- [ ] 取消采用协作式语义，不使用 `pthread_cancel` 或强制终止线程。
- [ ] `Io::CancellationSource`、`Io::CancellationToken` 和取消回调封装标准库 `std::stop_source`、`std::stop_token` 与 `std::stop_callback`。

## 3. 项目骨架

### 3.1 最小可用版本骨架

- [ ] Debug 构建启用额外断言。

### 3.2 发布前工程配置

- [ ] 添加 `.clang-tidy` 基础配置。

## 4. 公共基础类型

### 4.1 错误模型

- [ ] 实现新功能时，只为实际出现的失败路径添加对应的 `Io::ErrorCode` 和 `Io::Operation`，不预先穷举。
- [ ] 首次接入 Linux 系统调用时保存原始 `errno`，并为实际使用的 errno 添加集中映射和表驱动测试。
- [ ] 在调用方需要错误文本时为 `Io::Error` 提供 `Message()`。

### 4.2 Buffer 类型约定

- [ ] 在 `src/iocpp.cppm` 中定义 `Io::WritableBuffer` 和 `Io::ReadOnlyBuffer`。
- [ ] 定义 `Io::WritableBuffer` 为 `std::span<std::byte>`。
- [ ] 定义 `Io::ReadOnlyBuffer` 为 `std::span<const std::byte>`。
- [ ] 提供从 `std::span<char>` 转换到字节 span 的辅助函数。
- [ ] 提供从 `std::string_view` 转换到只读字节 span 的辅助函数。
- [ ] 明确所有同步文件操作只在调用期间借用 buffer。
- [ ] 明确异步 lambda 捕获 buffer 时由调用者负责生命周期。
- [ ] 为零长度 buffer 编写测试。

## 5. Linux 文件句柄

### 5.1 Io::File 类型

- [x] 在内部模块 `src/iocpp/file.cppm` 中定义文件句柄，并仅通过 `Io::File` 暴露。
- [x] 实现 `Io::AdoptFile()`，集中接管已有 native handle。
- [x] 定义无效 native handle 为 `-1`。
- [x] 实现默认构造的空 `Io::File`。
- [x] 删除拷贝构造函数。
- [x] 删除拷贝赋值运算符。
- [x] 实现移动构造函数。
- [x] 实现移动赋值运算符。
- [x] 实现析构关闭句柄。
- [x] 实现 `Valid()`。
- [x] 实现 `NativeHandle()`。
- [x] 实现 `Release()`，转移 native handle 所有权。
- [x] 实现 `Reset()`，关闭旧句柄并接管新句柄。
- [x] 保证移动后的对象处于无效但可析构状态。
- [x] 调用 `close` 前移除对象中的旧 fd；Linux 上 `close` 返回 `EINTR` 时不重试，避免误关已复用的 fd。
- [x] 保证析构中的 close 错误不会抛异常。
- [ ] 提供 `Io::Close(File&)`，以便调用者获得关闭错误。
- [ ] 防止显式 close 后析构重复关闭。
- [x] 为移动构造和移动赋值编写测试。
- [x] 为 `Release()` 编写所有权测试。
- [x] 为重复 `Reset()` 安全性编写测试。

### 5.2 打开文件

- [ ] 定义 `Io::OpenFile()` 的路径参数为 `std::string_view`，同步调用期间借用路径内容。
- [ ] 定义 `Io::OpenMode::ReadOnly`。
- [ ] 定义 `Io::OpenMode::WriteOnly`。
- [ ] 定义 `Io::OpenMode::ReadWrite`。
- [ ] 定义 `Io::OpenOptions::Create`。
- [ ] 定义 `Io::OpenOptions::Truncate`。
- [ ] 定义 `Io::OpenOptions::Exclusive`。
- [ ] 定义创建权限默认值 `0644`。
- [ ] 将 `Io::OpenOptions` 映射为 Linux `O_*` 标志。
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

- [ ] 定义 `Io::ReadAt(File&, WritableBuffer, uint64_t offset)`。
- [ ] 定义 `Io::WriteAt(File&, ReadOnlyBuffer, uint64_t offset)`。
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
- [ ] 实现 `Io::ReadAtAll()` 辅助函数。
- [ ] 实现 `Io::WriteAtAll()` 辅助函数。
- [ ] 防止 `Io::WriteAtAll()` 在连续零进度时无限循环。
- [ ] 测试基础位置写入和读取。
- [ ] 测试非零 offset。
- [ ] 测试越过 EOF 读取。
- [ ] 测试稀疏文件写入。
- [ ] 测试短读处理。
- [ ] 测试并发位置读取不会共享 seek offset。
- [ ] 测试大于一页的数据。
- [ ] 测试文件描述符关闭后的错误映射。

## 6. Io 与后端分发

### 6.1 IoContext 接口

- [x] 在 `src/iocpp.cppm` 中定义 `IoContext` 抽象基类。
- [x] 为 `IoContext` 添加虚析构函数。
- [ ] 添加 `OpenFile` 虚函数。
- [ ] 添加 `Close` 虚函数。
- [ ] 添加 `ReadAt` 虚函数。
- [ ] 添加 `WriteAt` 虚函数。
- [ ] 添加任务提交的内部虚函数。
- [ ] 添加查询并发容量的内部接口。
- [ ] 添加可取消 sleep 的内部虚函数。
- [ ] 添加后端 shutdown 接口。
- [ ] 明确所有虚函数的线程安全要求。
- [ ] 明确后端 shutdown 后各函数的行为。
- [ ] 禁止复制 `IoContext` 实现。
- [ ] 允许 `IoContext` 实现在固定地址上构造和使用。

### 6.2 Io 统一入口

- [x] 在 `src/iocpp.cppm` 中定义 `Io` 接口。
- [x] 实现从 `IoContext&` 构造 `Io`。
- [x] 允许复制和移动 `Io`。
- [ ] 添加 `context()` 内部访问器。
- [ ] 添加 `OpenFile()` 转发函数。
- [ ] 添加 `Close()` 转发函数。
- [ ] 添加 `ReadAt()` 转发函数。
- [ ] 添加 `WriteAt()` 转发函数。
- [ ] 添加 `ReadAtAll()` 高层循环。
- [ ] 添加 `WriteAtAll()` 高层循环。
- [x] 保证 `Io` 不负责 `IoContext` 实现的生命周期。
- [ ] 在文档中给出正确和错误的生命周期示例。
- [x] 在测试代码中定义虚拟时间 `TestIoContext`，验证时间调用确实经过后端分发，不向生产模块导出测试工具。
- [x] 测试复制后的 `Io` 指向同一后端。

## 7. 取消基础设施

### 7.1 Io::CancellationSource/Io::CancellationToken

- [ ] 在 `src/iocpp.cppm` 中定义 cancellation 类型。
- [ ] 定义 `Io::CancellationSource`。
- [ ] 定义轻量、可复制的 `Io::CancellationToken`。
- [ ] 使用 `std::stop_source` 和 `std::stop_token` 保存和共享取消状态。
- [ ] 实现 `RequestCancel()`。
- [ ] 保证 `RequestCancel()` 幂等。
- [ ] 实现 `StopRequested()`。
- [ ] 实现纯取消点 `CheckCancelled()`。
- [ ] 规定空 token 永不取消。
- [ ] 规定 source 析构不会自动请求取消。
- [ ] 提供基于 `std::stop_callback` 的取消回调包装，不自行维护回调链表。
- [ ] 记录取消回调可能在注册线程或请求取消的线程中同步执行。
- [ ] 测试取消前后的 token 状态。
- [ ] 测试多次 `RequestCancel()`。
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

- [ ] 在 `src/iocpp.cppm` 中定义 `Io::Task<T>` 类型。
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
- [ ] 定义 `Io::TaskError`，区分取消、callable 异常和无效 task。
- [ ] 定义 `Io::TaskResult<T>`。
- [ ] 定义 `Io::TaskResult<void>`。
- [ ] 明确 `Io::TaskResult<T>` 与普通 `Io::Result<T>` 分离：前者保存 task 终态，后者保存操作和提交错误。

### 8.2 Io::Task<T> API

- [ ] 实现 move-only、单消费者 `Io::Task<T>`。
- [ ] 删除 task 拷贝构造和拷贝赋值。
- [ ] 实现 move 构造。
- [ ] 实现 move 赋值。
- [ ] 实现 `Valid()`。
- [ ] 实现 `Ready()`。
- [ ] 实现阻塞 `Wait()`，成功返回后取出结果并使 task 失效。
- [ ] 实现 `WaitFor()`，超时时不消费结果。
- [ ] 实现 `Cancel()`。
- [ ] 让 `Cancel()` 幂等地请求取消并等待任务结束，但不消费结果，之后仍允许调用一次 `Wait()`。
- [ ] 实现 `Token()`，允许查看同一取消状态。
- [ ] 禁止多次 `Wait()`；第二次调用或在已消费 task 上调用返回无效 task 错误。
- [ ] 允许多次 `Cancel()`，每次都等待任务进入终态。
- [ ] 规定 `Wait()`、`WaitFor()` 和 `Cancel()` 不支持多个线程并发调用同一 `Io::Task<T>`。
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
- [ ] 确保完成时唤醒正在执行 `Wait()` 或 `WaitFor()` 的单个消费者。
- [ ] 防止任务完成两次。
- [ ] 防止 packaged callable 执行两次。
- [ ] 测试返回 `int` 的任务。
- [ ] 测试返回 move-only 对象的任务。
- [ ] 测试返回 `void` 的任务。
- [ ] 测试 callable 抛异常。
- [ ] 测试执行前取消。
- [ ] 测试执行中请求取消但 callable 忽略请求。
- [ ] 测试完成与取消同时发生。
- [ ] 测试 `Cancel()` 后仍可调用一次 `Wait()` 取得终态。
- [ ] 测试多次 `Cancel()`。
- [ ] 测试第二次 `Wait()` 返回无效 task 错误。
- [ ] 测试 task 移动后等待。
- [ ] 测试未显式 wait 的 task 析构。

## 9. 固定线程池 Io::Threaded

### 9.1 生命周期

- [ ] 将现有 `Io::Threaded` 扩展为固定线程池后端。
- [ ] 创建 `src/threaded.cpp`。
- [ ] 定义 `Io::ThreadedOptions`。
- [ ] 添加 `worker_count`。
- [ ] 添加 `async_limit`。
- [ ] 添加 `concurrent_limit`。
- [ ] `worker_count == 0` 时构造函数抛出 `std::invalid_argument`。
- [ ] 默认 worker 数使用 `std::thread::hardware_concurrency()`。
- [ ] `hardware_concurrency()` 返回 0 时默认使用 1 个 worker。
- [ ] `async_limit` 和 `concurrent_limit` 默认等于 `worker_count`，并允许调用者显式设置更大的排队容量。
- [ ] `Io::Threaded` 构造时创建 worker。
- [ ] 部分 worker 创建失败时停止并 join 已创建线程，然后抛出 `std::system_error`。
- [ ] `Io::Threaded` 析构时开始 shutdown。
- [ ] shutdown 后停止接收线程池任务；`Io::Async()` 改为 eager 执行，`Io::Concurrent()` 返回错误。
- [ ] shutdown 时唤醒所有 worker。
- [ ] shutdown 时取消尚未开始的任务。
- [ ] shutdown 时等待正在运行的任务。
- [ ] join 所有 worker。
- [ ] 保证显式 `Shutdown()` 幂等。
- [ ] 防止 worker 线程调用 `Shutdown()` 后 join 自己。

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
- [ ] 测试 `Io::Threaded` 空闲时不消耗一个 CPU 核。

### 9.3 async 提交规则

- [ ] 在 `Io` 添加 `Async()` 模板函数。
- [ ] 支持无参数 lambda。
- [ ] 支持捕获 move-only 值的 lambda。
- [ ] 支持返回值 callable。
- [ ] 支持返回 void callable。
- [ ] 检测 callable 是否可接收 `Io::CancellationToken`。
- [ ] 若 callable 同时支持有 token 和无参数调用，优先传入任务 token。
- [ ] 若可接收 token，则执行时传入任务 token。
- [ ] 若不可接收 token，则以无参数形式调用。
- [ ] 在共享 outstanding 数量低于 `async_limit` 时提交线程池。
- [ ] 在线程池已 shutdown 时同步 eager 执行 callable。
- [ ] 分配 task state 失败时允许 `std::bad_alloc` 从 `Io::Async()` 传播，因为此时无法构造有效 task。
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

- [ ] 在 `Io` 添加 `Concurrent()` 模板函数。
- [ ] 返回 `Io::Result<Io::Task<T>>`。
- [ ] 共享 outstanding 数量低于 `concurrent_limit` 时提交。
- [ ] 达到 `concurrent_limit` 时返回明确错误；实现该路径时再添加对应的 `Io::ErrorCode`。
- [ ] concurrent 不允许同步 eager 执行。
- [ ] `Io::Threaded` shutdown 后 `Concurrent()` 返回明确错误。
- [ ] 分配 task state 或 work item 失败时返回明确错误。
- [ ] 提交成功后保证任务最终由 worker 执行或在 shutdown 时取消。
- [ ] 测试 concurrent 成功。
- [ ] 测试 concurrent 容量耗尽。
- [ ] 测试 concurrent 不在调用线程执行。
- [ ] 测试释放容量后可以再次提交。
- [ ] 测试 concurrent 与 async 同时争用容量。
- [ ] 测试 async 与 concurrent 使用同一个 outstanding 计数，但分别应用各自的 limit。

## 10. 可取消 sleep

- [x] 在 `src/iocpp.cppm` 中定义 `Io::Clock`、`Io::Duration` 和 `Io::Timestamp`。
- [ ] 创建 `src/time_linux.cpp`。
- [ ] 将 `Io::Sleep(Duration)` 扩展为接受 `Io::CancellationToken` 的可取消操作。
- [ ] 将 `Io::SleepUntil(Timestamp)` 扩展为接受 `Io::CancellationToken` 的可取消操作。
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

- [ ] 编写示例 `example/file_async.cpp`。
- [ ] 创建 `Io::Threaded`。
- [ ] 使用 `Io::Threaded` 创建 `Io`。
- [ ] 创建临时文件。
- [ ] 使用 `Io::WriteAtAll()` 写入已知字符串。
- [ ] 使用 `Io::Concurrent()` 启动位置读取任务。
- [ ] 在任务中调用 `Io::ReadAtAll()`。
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
- [ ] 测试多线程同时调用 `Io::Async()`。
- [ ] 测试多线程同时调用 `Io::Concurrent()`。
- [ ] 测试容量限制严格生效。
- [ ] 测试 `Io::Threaded` 析构时存在 queued task。
- [ ] 测试 `Io::Threaded` 析构时存在 running task。
- [ ] 测试 `Io::Task<T>` 比 `Io` 更早析构。
- [ ] 运行 ASan 测试套件。
- [ ] 运行 UBSan 测试套件。
- [ ] 修复最小可用版本范围内的所有 ASan 和 UBSan 报告。

### 12.1 发布前并发与内存强化

- [ ] 使用 Debug 断言测试可检测的 `IoContext` 生命周期误用。
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
- [ ] 写明 `IoContext` 实现的生命周期规则。
- [ ] 写明 buffer 生命周期规则。
- [ ] 写明 task 析构可能阻塞。
- [ ] 写明取消是协作式的。
- [ ] 写明普通文件 syscall 不保证中断。
- [ ] 记录 `Io::Async()` 与 `Io::Concurrent()` 的区别。
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

- [ ] `Io::OpenOptions::Append`；在定义它与位置写入 `pwrite` 的交互规则后再加入。
- [ ] 支持多个线程并发等待同一个 `Io::Task<T>` 的共享任务类型。
- [ ] `Io::Reader`/`Io::Writer` 缓冲接口。
- [ ] scatter/gather `preadv`/`pwritev`。
- [ ] 流式 `read`/`write`。
- [ ] TCP 和 Unix domain socket。
- [ ] DNS 解析。
- [ ] `Io::TaskGroup`。
- [ ] `Io::Select()`。
- [ ] `Io::Batch`。
- [ ] semaphore、event、condition 等高层同步原语。
- [ ] mmap。
- [ ] 目录遍历和文件系统修改操作。
- [ ] 子进程管理。
- [ ] 安全随机数接口。
- [ ] io_uring `IoContext` 实现。
- [ ] epoll `IoContext` 实现。
- [ ] kqueue `IoContext` 实现。
- [ ] Windows IOCP `IoContext` 实现。
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
2. `Io::Error` 与基于 `std::expected` 的 `Io::Result<T>`。
3. `Io`、`IoContext` 与 move-only `Io::File`。
4. 同步 `Io::OpenFile()`、`Io::Close()`、`Io::ReadAt()`、`Io::WriteAt()`。
   完成此步后形成第一个同步文件 I/O 可用里程碑。
5. 为 `IoContext` 与 `Io` 补齐文件操作分发。
6. 基于标准库 stop token 的 `Io::CancellationSource`/`Io::CancellationToken`。
7. 单消费者 `Io::Task<T>` 状态机，不接线程池，先用测试直接驱动。
8. 固定线程池和 FIFO 任务队列。
9. `Io::Async()` eager 回退。
10. `Io::Concurrent()` 容量保证。
11. 可取消 `Io::Sleep()`。
12. 文件异步完整示例、README 使用说明、ASan 和 UBSan。
    完成此步后形成完整的最小可用版本。
13. TSan、Valgrind、并发压力测试和 CI。
14. Doxygen、安装规则、外部 consumer 测试和 `v0.1.0` API 冻结。
