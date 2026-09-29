[givm](../../reference.md) / [执行](../executor.md) / **error_string**

# givm::error_string

定义于头文件 `<givm/executor.hpp>`

```cpp
inline std::string error_string(const compile_error& error);
inline std::string error_string(const std::vector<compile_error>& errors);
inline std::string error_string(const program_input_error& error);
inline std::string error_string(const command_input_error& error);
inline std::string error_string(const command_input_error_reason& error);
inline std::string error_string(const history_access_error& error);
inline std::string error_string(const execution_view_error& error);
template<class Reason>
inline std::string error_string(const view_input_error<Reason>& error);
```

把定义库编译的结构化诊断或对局运行时的调试异常转换为文本。

## 参数

| | |
| --- | --- |
| `error`、`errors` | [`compile`](compile.md) 返回的一项诊断或诊断列表 |
| `error` | [`program_input_error`](program_input_error.md)、[`command_input_error`](command_input_error.md)、[`history_access_error`](history_access_error.md) 异常对象，或命令输入错误的具体原因 |

[`execution_view_error`](execution_view_error.md)、[`view_input_error`](view_input_error.md) 及其具体原因同样可以格式化。

## 返回值

按原列表顺序格式化的文本，包含错误原因与可用的源、程序、命令位置信息。各诊断之间换行，末尾不追加换行；空列表得到空字符串。

单项重载返回该诊断的文本，不追加换行。从 `compile_error::reason` 中取得的具体命令错误也可使用定义模块的 [`error_string`](../definition/error_string.md)；它只格式化原因，不包含编译诊断的程序位置信息。

异常重载返回与 `what()` 相同的文本。程序输入错误的各个具体原因也有独立重载，不包含异常对象附带的定义及程序位置。

## 注意

本函数不排序或去重。按错误类型处理时直接访问 [`compile_error`](compile_error.md) 的成员；仅输出日志时可使用 `error_string(result.error())`。

## 参阅

| | |
| --- | --- |
| [定义模块的 `error_string`](../definition/error_string.md) | 格式化源准备、牌组链接与单命令诊断 |
