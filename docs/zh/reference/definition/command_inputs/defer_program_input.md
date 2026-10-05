[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **defer_program_input**

# givm::defer_program_input

定义于头文件 `<givm/definition.hpp>`

```cpp
struct defer_program_input
{
    program_entry entry;
    program_inputs inputs;
};
```

为一次延迟效果保存目标程序及其全部参数。通常通过 [`defer_invoke`](../defer_invoke.md) 准备，再交给动态 [`defer_program`](../commands/defer_program.md)，或直接作为该命令的固定参数。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `entry` | [`program_entry`](../program_entry.md) | 目标程序入口 |
| `inputs` | [`program_inputs`](../program_inputs.md) | 目标程序的完整参数；默认构造表示无参数 |

## 注意

参数按目标程序的命令顺序提供；仅动态命令占输入位置。可以嵌套使用 `defer_invoke`，为目标程序中的其他延迟命令继续准备参数。

本对象拥有已准备的参数和数组内容。原数组在 `defer_invoke` 完成后即可销毁；本对象在 [`invoke`](../../executor/handle_context/invoke.md) 或 [`add_program`](../../executor/definition_compile_context/add_program.md) 完成复制后也可销毁，无须存活到延迟程序执行。

目标入口须来自配套定义库，参数须符合其协议。未定义 `NDEBUG` 时，动态调用在提交时检查，固定命令在编译时检查，嵌套参数也参与检查；发布构建不保留相应诊断。
