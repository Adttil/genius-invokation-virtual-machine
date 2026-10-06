[givm](../../reference.md) / [定义](../definition.md) / **defer_invoke**

# givm::defer_invoke

定义于头文件 `<givm/definition.hpp>`

```cpp
template<event_category Category, class... T> // 每个 T 均须为核心命令声明的 input_type
defer_program_input defer_invoke(effect<Category> entry, const T&... inputs);

template<event_category Category>
defer_program_input defer_invoke(effect<Category> entry, program_inputs inputs);
```

准备一次延迟效果需要的程序入口和全部参数。它只准备数据；实际登记和执行由 [`defer_program`](commands/defer_program.md) 完成。

## 参数

| | |
| --- | --- |
| `entry` | 与普通响应共用的目标程序入口 |
| `inputs` | 按目标程序动态命令的执行顺序提供的专用输入对象，或已准备的 [`program_inputs`](program_inputs.md) |

## 返回值

拥有目标入口和完整参数的 [`defer_program_input`](command_inputs/defer_program_input.md)。没有动态命令的目标程序使用 `defer_invoke(entry)`。

## 注意

动态延迟命令的输入可以直接放在普通输入之间，例如 `context.invoke(parent, input_a, defer_invoke(child, child_input), input_b)`。目标程序若也包含延迟命令，可以继续嵌套使用 `defer_invoke`。

固定延迟命令使用 [`fixed_defer_invoke`](fixed_defer_invoke.md)，写作 `defer_program{fixed_defer_invoke(child, child_input)}`。它保留编译校验需要的信息，固定形式不占父程序的动态输入位置。

逐项重载在返回前复制命令数组及嵌套参数，原数组此后可以销毁。接收 `program_inputs` 的重载按值取得已有快照，可以传入 `std::move(inputs)` 转移其所有权。

本函数不访问定义库，因此不核对目标程序要求。未定义 `NDEBUG` 时，返回值携带独立诊断信息，供动态调用的 [`invoke`](../executor/handle_context/invoke.md) 检查使用；发布构建不保留这份信息。

## 参阅

| | |
| --- | --- |
| [`defer_program`](commands/defer_program.md) | 登记延迟效果，包含完整示例 |
| [`pack_inputs`](pack_inputs.md) | 单独准备拥有型参数 |
| [`concat_inputs`](concat_inputs.md) | 合并运行时准备的输入片段 |

目标类别只允许 `normal` 和 `preview`；`immediate_effect` 在 C++ 编译时被拒绝。
