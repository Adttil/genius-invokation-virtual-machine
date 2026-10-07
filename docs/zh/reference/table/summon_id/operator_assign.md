[givm](../../../reference.md) / [牌桌](../../table.md) / [summon_id](../summon_id.md) / **operator_assign**

# givm::summon_id::operator_assign

```cpp
constexpr summon_id& operator=(const summon_id&) noexcept = default;
constexpr summon_id& operator=(summon_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
