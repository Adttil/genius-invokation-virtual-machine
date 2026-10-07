[givm](../../../reference.md) / [牌桌](../../table.md) / [character_id](../character_id.md) / **operator_assign**

# givm::character_id::operator_assign

```cpp
constexpr character_id& operator=(const character_id&) noexcept = default;
constexpr character_id& operator=(character_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
