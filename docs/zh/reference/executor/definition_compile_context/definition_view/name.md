[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **name**

# givm::definition_compile_context::definition_view::name

定义于头文件 `<givm/definition_source.hpp>`

```cpp
std::string_view name() const noexcept;
```

取得这项定义的名称。

## 返回值

定义源提供的完整名称。字符串视图不拥有字符存储，长期保存时须遵守[定义源生命周期](../../../definition/source_protocol.md)约定。
