[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **equipment_type**

# givm::definition_library::definition_view::equipment_type

定义于头文件 `<givm/executor.hpp>`

```cpp
givm::equipment_type equipment_type() const noexcept
    requires std::same_as<TDefinitionType, attachment_view>;
```

取得该附着实体定义的装备类别。仅适用于附着实体定义视图。

## 返回值

与 `library.equipment_type(id())` 相同，见 [`definition_library::equipment_type`](../equipment_type.md)。
