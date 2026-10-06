[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **end_segment**

# givm::end_segment

定义于头文件 `<givm/definition.hpp>`

```cpp
struct end_segment {};
```

结束当前程序段，使之后登记的后续工作属于下一段。

## 注意

本命令不执行已经登记的延迟程序，也不结束响应程序。程序继续执行下一条命令；普通响应调用方在程序返回后按登记顺序处理各段的后续工作。需要在当前位置完成这些工作时，使用 [`settle`](settle.md)。

没有记录的空段不产生额外效果。末段由普通响应或延迟程序的调用者收尾，[`add_normal_effect`](../../executor/definition_compile_context/add_effect.md) 不自动追加本命令。初始化和回合根程序须使用 `settle` 明确完成其登记的后续工作。

立即效果共享外层段，不能包含 `end_segment` 或 `settle`。在 Debug 和 Release 的库编译中都返回 `effect_command_not_allowed` 诊断，不进入运行期检查。

## 参阅

| | |
| --- | --- |
| [`defer_program`](defer_program.md) | 向当前段登记延迟程序 |
| [`return_response`](return_response.md) | 返回下一响应编号或结束响应链 |
