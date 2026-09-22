[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **has_technique**

# has_technique

定义于头文件 `<givm/executor.hpp>`

```cpp
bool has_technique() const noexcept;
```

当前出战角色装备了支持 `technique_effect` 响应的特技时返回 true。受控不改变此结果，使用前可单独调用 `is_controlled`。
