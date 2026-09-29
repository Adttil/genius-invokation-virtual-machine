[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **shield_id**

# givm::definition_compile_context::shield_id

定义于头文件 `<givm/definition_source.hpp>`

```cpp
definition_id<combat_status_view> shield_id() const noexcept;
```

取得本次编译采用的护盾定义 ID，供定义源编译效果程序时使用。

## 返回值

本次 [`basic_definition_sources`](../../definition/basic_definition_sources.md) 的 `shield` 所对应的有效 `definition_id<combat_status_view>`。

## 注意

无须通过名称依赖声明绑定具体版本，也无须提前登记该基础源。即使只编译部分定义，本次配置的基础定义也始终存在。普通按名称查询的依赖仍须提前声明。

返回值与最终 [`definition_library::shield_id`](../definition_library/shield_id.md) 相同；不得与其他编译所得定义库的 ID 混用。
