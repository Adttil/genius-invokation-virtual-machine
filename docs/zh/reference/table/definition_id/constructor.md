[givm](../../../reference.md) / [牌桌](../../table.md) / [definition_id](../definition_id.md) / **(构造函数)**

# givm::definition_id::definition_id

```cpp
constexpr definition_id() noexcept = default;
constexpr explicit definition_id(std::uint64_t index);
```

默认构造保持平凡，不提供空值。默认初始化后，必须先赋值才能读取。值初始化会将存储清零；它不表示“没有定义”。

整数构造保存索引，前提是高位预留部分为零。Debug 在违反位宽要求时抛出 `std::invalid_argument`；是否在配套定义库中存在该索引由调用方保证。
