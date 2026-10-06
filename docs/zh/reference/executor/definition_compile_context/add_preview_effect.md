[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **add_preview_effect**

# givm::definition_compile_context::add_preview_effect

定义于头文件 `<givm/definition_source.hpp>`。

```cpp
preview_effect add_preview_effect(std::span<const any_command> commands);

template<class... TCommands>
preview_effect add_preview_effect(TCommands&&... commands);
```

登记预览响应提交的后续效果，等价于 [`add_effect<event_category::preview>`](add_effect.md)。接收连续命令、tuple 或其他命令范围，也可逐项传入零个或多个命令。

报价时只保留入口及参数；确认操作后，效果在独立结算域执行，返回前自动完成尾部结算。返回编号被忽略，它也可以用于延迟调用。
