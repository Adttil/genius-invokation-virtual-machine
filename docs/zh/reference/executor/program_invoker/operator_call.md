[givm](../../../reference.md) / [执行](../../executor.md) / [program_invoker](../program_invoker.md) / **operator()**

# givm::program_invoker::operator()

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class... T> // 每个 T 均须为核心命令声明的 input_type
normal_effect operator()(normal_effect entry, T&&... inputs);

normal_effect operator()(normal_effect entry, const program_inputs& inputs);

template<class... T> // 每个 T 均须为核心命令声明的 input_type
normal_effect operator()(substack_t, normal_effect entry, T&&... inputs);

normal_effect operator()(substack_t, normal_effect entry, const program_inputs& inputs);
```

提交要执行的效果，以及本次效果需要的全部命令输入。定义源通过 [`handle_context::invoke`](../handle_context/invoke.md) 使用它；入口、输入和生命周期约定见该接口。

## 返回值

原样返回 `entry`。

## 注意

普通响应使用不带标记的重载；费用响应必须在入口前传 `substack_t{}`。每个动态命令对应一个由 `input_type` 指定的输入对象，固定模式不占输入位置。动态适配器可提交通过 [`pack_inputs`](../../definition/pack_inputs.md) 和 [`concat_inputs`](../../definition/concat_inputs.md) 准备的 [`program_inputs`](../../definition/program_inputs.md)。

命令数组的内容在调用中复制，数组长度可变。未定义 `NDEBUG` 时，在写入前检查入口、普通与费用提交方式、重复提交、输入数量、具体类型及顺序，失败时抛出 [`program_input_error`](../program_input_error.md)。发布构建不保留检查及对应诊断元数据，错误输入属于未定义行为。

检查失败不会写入本次输入，但不保证整个响应或本次推进回滚；捕获异常后不应在原现场继续推进。尾调用与借用对象生命周期仍由定义源保证，见 [`invoke`](../handle_context/invoke.md)。
