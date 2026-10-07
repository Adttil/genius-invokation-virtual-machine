[givm](../../../reference.md) / [牌桌](../../table.md) / [support_id](../support_id.md) / **operator_assign**

# givm::support_id::operator_assign

```cpp
constexpr support_id& operator=(const support_id&) noexcept = default;
constexpr support_id& operator=(support_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
