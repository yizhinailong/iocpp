# iocpp

zig std.Io 使用 C++ 实现

公共操作统一从 `iocpp::Io` 进入：类型位于 `Io::File`、`Io::Error`、`Io::Threaded` 等名字下，需要后端能力的操作通过 `Io` 实例调用。`IoContext` 是供后端实现继承的抽象接口，不是业务操作入口。

## 构建与测试

项目使用 [mcpp](https://github.com/mcpp-community/mcpp) 和 C++23 模块：

```bash
mcpp build
mcpp test
```

## 示例

```bash
cd example
mcpp run file-example
```

## 开发文档

- [未完成工作](docs/TODO.md)
- [已实现功能归档](docs/ARCHIVE.md)
