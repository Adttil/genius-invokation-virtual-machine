[givm](../../reference.md) / [执行](../executor.md) / **program_input_error_reason**

# givm::program_input_error_reason

定义于头文件 `<givm/runtime.hpp>`

```cpp
using program_input_error_reason = std::variant<program_input_count_mismatch,
    program_input_type_mismatch, invalid_effect,
    repeated_program_invocation, invalid_response_index>;
```

[`program_input_error`](program_input_error.md) 的具体原因。以下结构体、枚举均位于 `givm` 命名空间。

## 类

### `program_input_count_mismatch`

```cpp
struct program_input_count_mismatch;
```

程序需要的输入对象数量与提交数量不同。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `expected` | `std::size_t` | 该程序需要的输入对象数量 |
| `actual` | `std::size_t` | 本次提交的输入对象数量 |

### `program_input_type_mismatch`

```cpp
struct program_input_type_mismatch;
```

某个输入对象的类型不符合对应命令的要求。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `input_index` | `std::size_t` | 出错对象在本次输入列表中的索引，从零开始 |
| `command_index` | `std::size_t` | 对应命令在该响应程序中的索引，从零开始；固定参数命令也计入 |
| `command` | `std::string` | 对应命令名称 |
| `expected` | `std::string` | 所需输入类型的名称 |
| `actual` | `std::string` | 实际输入类型的名称，或无法取得 variant 值的说明 |

### `repeated_program_invocation`

```cpp
struct repeated_program_invocation;
```

同一次响应已经成功提交过程序，却再次调用 `invoke` 的错误。该类型没有成员。

### `invalid_response_index`

```cpp
struct invalid_response_index;
```

动态返回输入错误地使用了 `return_response::dynamic`。它是命令的取值方式标记，不能作为运行时返回编号。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `index` | `std::uint32_t` | 本次提供的错误返回值 |

## 枚举

### `invalid_effect`

```cpp
enum class invalid_effect;
```

不能在当前定义库中使用的程序入口。

| | |
| --- | --- |
| `null_entry` | 向 `invoke` 传入空入口 |
| `different_library` | 入口来自另一份独立编译的定义库 |
| `unknown_entry` | 当前定义库不能识别该入口 |

定义库复制保留入口的对应关系，因此原库的入口可以在其副本中使用。独立重新编译的库即使内容相同，也不能混用入口。

## 参阅

| | |
| --- | --- |
| [`program_input_error`](program_input_error.md) | 带定义、程序位置的输入协议异常 |
