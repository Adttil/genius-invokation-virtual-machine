[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **return_response**

# givm::return_response

定义于头文件 `<givm/definition.hpp>`

```cpp
struct return_response
{
    using error_type = return_response_error;
    using input_type = return_response_input;

    static constexpr std::uint32_t null = std::numeric_limits<std::uint32_t>::max();
    static constexpr std::uint32_t dynamic = null - 1;

    std::uint32_t index = dynamic;
};
```

结束当前响应程序，并指定同一实体是否继续响应原事件。

## 成员类型

| | |
| --- | --- |
| `error_type` | 本命令的编译检查错误 `return_response_error` |
| `input_type` | 动态返回编号的输入 [`return_response_input`](../command_inputs/return_response_input.md) |

## 静态成员常量

| | |
| --- | --- |
| `null` | 结束该实体本次响应链的特殊值 |
| `dynamic` | 从动态输入取得返回编号的特殊值 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `index` | `std::uint32_t` | 下一次响应编号、`null` 或 `dynamic`；默认使用动态输入 |

## 注意

普通编号的范围为 0 至 `dynamic - 1`，不要求逐次递增。普通响应的程序及其后续结算全部完成后，再以该编号调用同一实体对原事件的 `handle`。返回 `null` 时继续其他响应者。即时响应只结束本次程序，不封闭外层当前段。

[`add_normal_effect`](../../executor/definition_compile_context/add_effect.md) 只处理到第一条本命令，其后命令不再检查、编译或要求输入。有效序列未显式返回时，自动补上 `return_response{ .index = return_response::null }`。

本命令不自动执行 [`end_segment`](end_segment.md)；普通响应和延迟程序的调用者负责末段收尾及后续工作。费用响应只调用一次，实际执行其缓存程序时忽略返回编号。[延迟程序](defer_program.md) 同样忽略返回编号，不延长登记它的父响应链。

动态输入可以返回普通编号或 `null`，不得返回 `dynamic`。未定义 `NDEBUG` 时，错误通过 [`program_input_error`](../../executor/program_input_error.md) 的 `invalid_response_index` 原因报告。

## 编译检查

`return_response_error` 包含 `reason cause`，其中 `reason::return_in_root` 表示在初始化或回合根程序中使用了本命令。根程序没有响应调用方，不能使用此命令返回。

## 参阅

| | |
| --- | --- |
| [定义源协议](../source_protocol.md) | 响应实体、事件与编号的调用约定 |
| [`defer_program`](defer_program.md) | 在后续结算中执行程序 |
