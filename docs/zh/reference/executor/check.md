[givm](../../reference.md) / [执行](../executor.md) / **check**

# givm::check

定义于头文件 `<givm/executor.hpp>`

```cpp
inline std::vector<draw_cards::error_type> check(
    const draw_cards& command, const definition_compile_context& context, program_kind kind);
```

检查一条命令在当前编译集合和程序类别中是否合法。每个核心命令都有自己的非模板重载；上面列出 [`draw_cards`](../definition/commands/draw_cards.md) 的重载，其余命令以各自类型替换参数和结果中的 `draw_cards`。

## 参数

| | |
| --- | --- |
| `command` | 待编译的命令值 |
| `context` | 本次 [`definition_compile_context`](definition_compile_context.md)，用于核对定义 ID 数值是否落在对应类别的定义数量范围内等信息 |
| `kind` | [`program_kind`](program_kind.md)，表示命令位于初始化、回合还是响应程序 |

## 返回值

该命令自己的 `error_type` 列表，与 `std::vector<命令名_error>` 是同一类型；为空表示本次编译检查通过。可独立判断的多项错误会一并返回，例如非法玩家与超出定义范围的 ID。各命令的错误原因及附带字段见对应[命令页面](../definition/commands.md)。

动态模式的命令只检查当前程序能否消费响应输入，不检查未使用的固定字段。初始化与回合根程序不能消费响应输入，返回 `dynamic_input_in_root`；响应程序允许动态模式。

## 注意

通常无需手动调用：[`compile`](compile.md) 及 [`add_program`](definition_compile_context/add_program.md) 在编译每条命令时自动调用其 `check`，再把错误与源、程序及命令位置一起收集。需要单独检查时，可在定义源的编译函数中通过 ADL 调用 `check(command, context, kind)`。单独调用只返回结果，不登记程序或把诊断写入编译上下文。

检查使用编译期间已知的信息，不读取对局牌桌；它不判断运行时目标是否存在、当前资源是否足够或未来响应输入是否正确。相对玩家允许用于根程序，但调用方仍须在运行前正确设置牌桌的本方。

命令没有编译期参数错误时，其 `error_type` 是空枚举，对应 `check` 总是返回空列表。接口仅支持核心给定的命令，不能据此扩展新的命令种类。

## 参阅

| | |
| --- | --- |
| [`compile_error`](compile_error.md) | 包含位置与原因的整库编译诊断 |
| [`error_string`](error_string.md) | 将编译诊断转换为文本 |
| [单命令错误的 `error_string`](../definition/error_string.md) | 将单独检查得到的原因转换为文本 |
