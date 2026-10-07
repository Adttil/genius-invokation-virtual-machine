[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_definition_id](../variant_definition_id.md) / **constructor**

# givm::variant_definition_id::constructor

```cpp
constexpr variant_definition_id() noexcept;
constexpr variant_definition_id(std::nullptr_t) noexcept; // 仅可空类型
constexpr explicit variant_definition_id(std::uint64_t word);
```

可空类型默认保存空值，非空类型默认构造保持平凡。允许的单类别 ID 可隐式转换为该类型，另一个类别集合为本集合子集的多类别 ID 也可隐式转换。整数构造读取完整编码字，Debug 验证类别及编码前提。
