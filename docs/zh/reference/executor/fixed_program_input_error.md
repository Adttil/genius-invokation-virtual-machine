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

该检查针对 [`defer_program{fixed_defer_invoke(...)}`](../definition/commands/defer_program.md) 已经提供的固定参数，包括嵌套延迟输入。Debug 和 Release 编译均执行，失败时作为整库编译的结构化诊断返回，不通过异常报告。

参数描述在编译时使用，不写入固定程序或 Release 定义库。Debug 定义库额外保留程序输入要求，供后续动态调用检查使用。

响应时提供的动态输入仅在 Debug 的 [`invoke`](handle_context/invoke.md) 中检查，失败时抛出 [`program_input_error`](program_input_error.md)。
