[givm](../../../reference.md) / [牌桌](../../table.md) / [player_view](../player_view.md) / **reaction**

# givm::player_view::reaction

```cpp
definition_id<givm::definition_category::reaction> reaction(elemental_reaction slot) const noexcept;
```

读取本局该玩家的反应映射。`slot` 必须是 17 个有效槽位之一，不能为 `none`。角色死亡、附属移除和复苏不改变映射；通过 `reaction_id{ player.id(), slot }` 可以从牌桌取得相应反应实体。
