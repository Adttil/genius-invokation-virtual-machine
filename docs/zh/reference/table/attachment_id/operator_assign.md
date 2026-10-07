[givm](../../../reference.md) / [牌桌](../../table.md) / [attachment_id](../attachment_id.md) / **operator_assign**

# givm::attachment_id::operator_assign

```cpp
constexpr attachment_id& operator=(const attachment_id&) noexcept = default;
constexpr attachment_id& operator=(attachment_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
