[givm](../../../reference.md) / [执行](../../executor.md) / [program_invoker](../program_invoker.md) / **operator()**

# givm::program_invoker::operator()

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class... T>
requires ((std::is_trivially_copyable_v<T> && ...)
    && (not std::is_convertible_v<T, std::span<const unsigned char>> && ...))
program_entry operator()(program_entry entry, T... inputs);

program_entry operator()(program_entry entry, std::span<const unsigned char> inputs);

template<class... T>
requires ((std::is_trivially_copyable_v<T> && ...)
    && (not std::is_convertible_v<T, std::span<const unsigned char>> && ...))
program_entry operator()(substack_t, program_entry entry, T... inputs);

program_entry operator()(substack_t, program_entry entry, std::span<const unsigned char> inputs);
```

提交要执行的效果，以及本次效果需要的全部初始输入，并原样返回 `entry`。定义源通过 [`handle_context::invoke`](../handle_context/invoke.md) 使用它；入口、输入和生命周期约定见该接口。

普通响应使用不带标记的重载；费用响应必须在入口前传 `substack_t{}`，否则行为未定义。逐项调用按值接收初始事件；不需要输入的程序不传输入参数。字节 span 重载接收为固定入口准备好的完整不透明输入段。每个程序要求的输入数量、类型和顺序在编译时确定，响应时只提供本次调用的输入值。输入必须与入口匹配。debug 构建在写入前只检查输入总字节长度，不符时抛出 `std::invalid_argument`；不检查同长度输入的类型或顺序，不要求源提供类型元信息。发布构建不检查，错误输入属于未定义行为。
