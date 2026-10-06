[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **add_normal_effect**

# givm::definition_compile_context::add_normal_effect

定义于头文件 `<givm/definition_source.hpp>`。

```cpp
normal_effect add_normal_effect(std::span<const any_command> commands);

template<class... TCommands>
normal_effect add_normal_effect(TCommands&&... commands);
```

登记普通效果，等价于 [`add_effect<event_category::normal>`](add_effect.md)。接收连续命令、tuple 或其他命令范围，也可逐项传入零个或多个命令。

普通效果允许显式分段及结算，返回前自动完成末段结算并退出自己的结算域。它也可以通过 [`defer_invoke`](../../definition/defer_invoke.md) 用于延迟调用。
