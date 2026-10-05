[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **return_response_input**

# givm::return_response_input

定义于头文件 `<givm/definition.hpp>`

```cpp
struct return_response_input
{
    std::uint32_t index = std::numeric_limits<std::uint32_t>::max();
};
```

[`return_response`](../commands/return_response.md) 的动态输入，决定是否继续响应原事件及下一次的响应编号。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `index` | `std::uint32_t` | 下一次响应编号或 `return_response::null`，默认结束响应链 |

## 注意

不能将 `return_response::dynamic` 作为运行时返回值。未定义 `NDEBUG` 时，此错误在提交输入时通过 [`program_input_error`](../../executor/program_input_error.md) 报告。

仅当命令的 `index == return_response::dynamic` 时提供本输入；固定返回不占输入位置。
