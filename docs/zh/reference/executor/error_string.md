[givm](../../reference.md) / [执行](../executor.md) / **error_string**

# givm::error_string

定义于头文件 `<givm/executor.hpp>`

```cpp
inline std::string error_string(const compile_error& error);
inline std::string error_string(const std::vector<compile_error>& errors);
```

把定义库编译的结构化诊断转换为文本。

## 参数

| | |
| --- | --- |
| `error`、`errors` | [`compile`](compile.md) 返回的一项诊断或诊断列表 |

## 返回值

按原列表顺序格式化的文本，包含错误原因与可用的源、程序、命令位置信息。各诊断之间换行，末尾不追加换行；空列表得到空字符串。

单项重载返回该诊断的文本，不追加换行。单独调用 [`check`](check.md) 得到的命令错误可使用定义模块的 [`error_string`](../definition/error_string.md)；它只格式化原因，不包含尚未关联的程序位置信息。

## 注意

本函数不排序或去重。按错误类型处理时直接访问 [`compile_error`](compile_error.md) 的成员；仅输出日志时可使用 `error_string(result.error())`。

## 参阅

| | |
| --- | --- |
| [定义模块的 `error_string`](../definition/error_string.md) | 格式化源准备、牌组链接与单命令诊断 |
