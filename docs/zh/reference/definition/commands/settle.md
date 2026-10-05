[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **settle**

# givm::settle

定义于头文件 `<givm/definition.hpp>`

```cpp
struct settle {};
```

完成当前调用已经登记的后续工作，然后继续执行下一条命令。

## 注意

先结束当前段，再按登记顺序执行各段的后续工作。每个延迟程序及其派生工作完整结束后，才继续下一项。延迟程序需要玩家输入时，执行器按相应命令暂停；恢复并完成后，才继续 `settle` 之后的程序。

普通响应和延迟程序的调用者在程序返回时自动完成末段及后续工作；初始化、回合等根程序需要显式使用本命令。根程序中的 [`end_segment`](end_segment.md) 只保存段边界，不能代替 `settle`。结束对局不会隐式执行尚未处理的延迟程序。

即时响应共享外层当前段，禁止执行本命令。未定义 `NDEBUG` 时，违规执行抛出 [`command_input_error`](../../executor/command_input_error.md)，原因为 `settlement_in_inline_response`；发布构建不进行此检查。

## 参阅

| | |
| --- | --- |
| [`end_segment`](end_segment.md) | 结束当前段而不立即执行后续工作 |
| [`defer_program`](defer_program.md) | 登记稍后执行的程序 |
