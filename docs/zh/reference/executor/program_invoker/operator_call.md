[givm](../../../reference.md) / [执行](../../executor.md) / [program_invoker](../program_invoker.md) / **operator()**

# givm::program_invoker::operator()

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class... T> // 每个 T 均须为核心命令声明的 input_type
program_entry operator()(program_entry entry, T... inputs);

program_entry operator()(program_entry entry, std::span<const any_command_input> inputs);

template<class... T> // 每个 T 均须为核心命令声明的 input_type
program_entry operator()(substack_t, program_entry entry, T... inputs);

program_entry operator()(substack_t, program_entry entry, std::span<const any_command_input> inputs);
```

提交要执行的效果，以及本次效果需要的全部命令输入。定义源通过 [`handle_context::invoke`](../handle_context/invoke.md) 使用它；入口、输入和生命周期约定见该接口。

## 返回值

原样返回 `entry`。

## 注意

普通响应使用不带标记的重载；费用响应必须在入口前传 `substack_t{}`。每个动态命令对应一个由 `input_type` 指定的输入对象，固定模式不占输入位置。逐项提交按值接收对象，动态适配器可用 [`any_command_input`](../../definition/any_command_input.md) 的 span 提交同样的序列。

命令数组的内容在调用中复制。未定义 `NDEBUG` 时，在写入前检查输入数量、具体类型及顺序，不符时抛出 `std::invalid_argument`；数组长度可变。发布构建不检查，错误输入属于未定义行为。
