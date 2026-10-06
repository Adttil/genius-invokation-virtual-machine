[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **add_immediate_effect**

# givm::definition_compile_context::add_immediate_effect

定义于头文件 `<givm/definition_source.hpp>`。

```cpp
immediate_effect add_immediate_effect(std::span<const any_command> commands);

template<class... TCommands>
immediate_effect add_immediate_effect(TCommands&&... commands);
```

登记立即效果，等价于 [`add_effect<event_category::immediate>`](add_effect.md)。接收连续命令、tuple 或其他命令范围，也可逐项传入零个或多个命令。

立即效果产生的记录归入外层段。编译时禁止 `end_segment` 和 `settle`，它也不能作为延迟调用的目标；效果本身可以用 `defer_program` 发布普通或预览效果。
