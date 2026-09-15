[givm](../../reference.md) / [执行](../executor.md) / **compile_mode**

# givm::compile_mode

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class compile_mode : std::uint8_t
{
    normal,
    observed
};
```

选择编译后的定义库需要报告的执行现场。

## 枚举值

| | |
| --- | --- |
| `normal` | 仅在需要输入或对局结束时交回控制权 |
| `observed` | 除输入和终局外，还报告约定的领域观察现场 |

## 注意

作为 [`compile`](compile.md) 的最后一个参数显式提供；同一次编译中的游戏流程和所有定义响应使用同一模式。两种模式生成相同类型的 [`definition_library`](definition_library.md)，统一通过 [`executor::step`](executor/step.md) 推进。

模式决定交回控制权的时机，不改变游戏规则、响应顺序或输入要求。定义库编译完成后，其模式不再通过执行器切换。观察现场的具体边界见 [`execution_state`](execution_state.md)。
