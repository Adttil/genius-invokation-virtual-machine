[givm](../../reference.md) / [执行](../executor.md) / **fixed_program_input_error**

# givm::fixed_program_input_error

定义于头文件 `<givm/compile.hpp>`

```cpp
struct fixed_program_input_error
{
    program_input_error_reason reason;
};
```

编译固定延迟效果时发现目标入口或参数不符合程序要求。它作为 [`compile_error_reason`](compile_error_reason.md) 的候选，随编译错误列表返回。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `reason` | [`program_input_error_reason`](program_input_error_reason.md) | 入口无效、输入数量或类型不匹配、动态返回编号无效等具体原因 |

## 注意

该检查针对 [`defer_program{defer_invoke(...)}`](../definition/commands/defer_program.md) 已经提供的固定参数，包括嵌套延迟输入。未定义 `NDEBUG` 时执行，发布构建不保留诊断元数据与检查。

响应时提供的动态输入仍在 [`invoke`](handle_context/invoke.md) 中检查，失败时抛出 [`program_input_error`](program_input_error.md)。
