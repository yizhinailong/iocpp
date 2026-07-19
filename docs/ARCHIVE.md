# iocpp 功能归档

<!-- archived-through: d9a74c039264a432a7a699a6563c047c051d041a -->

本文件只记录已经实现并可验证的功能。未完成工作统一保留在
[`TODO.md`](TODO.md) 中，不在这里预先列举。

最近一次归档基线：
`d9a74c039264a432a7a699a6563c047c051d041a`。

该 SHA 指向最后一个已纳入归档的功能提交，不要求包含归档文档自身的提交。
下次归档时，以该 SHA 为排他起点检查后续提交，完成归档后再更新顶部的
`archived-through` 标记和本段基线。

```bash
BASE=d9a74c039264a432a7a699a6563c047c051d041a
git log --oneline --reverse "$BASE"..HEAD
git diff --stat "$BASE"..HEAD
```

## 2026-07-19 初始功能快照

归档提交：`d9a74c039264a432a7a699a6563c047c051d041a`

### 构建与项目结构

- 使用 `mcpp` 管理 C++23 项目，产物为 `iocpp` 库而不是可执行文件。
- 使用 `.cppm` 模块接口，公开 `iocpp` 和 `iocpp.error` 模块。
- `iocpp` 模块重新导出 `iocpp.error`。
- 使用 `compat.gtest@1.15.2` 作为开发测试依赖。
- README 提供 `mcpp build` 和 `mcpp test` 命令。

### 时间与上下文接口

- 提供 `Clock`、`Duration` 和 `Timestamp` 时间类型别名。
- 提供 `IoContext` 接口，包含 `Now()` 和 `Sleep()`。
- 提供使用系统稳态时钟和当前线程休眠的 `ThreadContext`。
- 提供通过虚拟时间进行确定性测试的 `TestContext`。
- 提供轻量 `Io` 句柄，将 `Now()`、`Sleep()` 和 `SleepUntil()` 分发给上下文。
- `Io` 可以复制，副本共享同一个上下文；其查询和休眠接口支持在常量句柄上调用。
- `SleepUntil()` 仅在截止时间晚于当前时间时执行休眠。

### 错误与结果类型

- `iocpp.error` 提供最小错误模型 `ErrorCode`、`Operation` 和 `Error`。
- 当前只保留实际测试使用的 `ErrorCode::Unsupported` 和 `Operation::Sleep`。
- `Result<T>` 是 `std::expected<T, Error>` 的别名。
- `Result<T>` 支持普通值、`void`、move-only 值和 `std::unexpected<Error>`。
- 成功或失败通过 `Result<T>` 判断，不为 `Error` 提供语义含糊的布尔转换。
- 错误码和操作类型遵循按实际调用点增量添加的原则，不预先穷举。

### 测试覆盖

- 验证顶层模块重新导出错误模块。
- 验证虚拟时间推进、过期截止时间、上下文共享和常量句柄。
- 验证 `Error` 保存错误码与操作上下文。
- 验证 `Result<T>` 的成功值、失败值、布尔检查、解引用和错误访问异常。
- 验证 `Result<void>` 和 move-only 值。
- 归档时共有两个 GoogleTest 测试程序、11 个测试用例通过。
