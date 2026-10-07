[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_entity_id](../variant_entity_id.md) / **player_id**

# givm::variant_entity_id::player_id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr givm::player_id player_id() const noexcept;
```

直接取得所属玩家的 ID，无须先提取具体类别。只有不包含 `entity_category::null` 的类型提供此成员；可空 ID 须先判断并提取非空 ID。
