[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_id](../skill_id.md) / **operator_assign**

# givm::skill_id::operator_assign

```cpp
constexpr skill_id& operator=(const skill_id&) noexcept = default;
constexpr skill_id& operator=(skill_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
