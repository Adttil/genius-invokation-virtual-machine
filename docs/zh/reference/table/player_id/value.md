[givm](../../../reference.md) / [牌桌](../../table.md) / [player_id](../player_id.md) / **value**

# givm::player_id::value

```cpp
constexpr std::uint64_t value() const noexcept;
```

只读取得内部保存的完整编码值。玩家没有父级，其编码值等于构造时传入的玩家索引。
