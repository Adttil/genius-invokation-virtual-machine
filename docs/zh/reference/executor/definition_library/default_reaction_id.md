[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **default_reaction_id**

# givm::definition_library::default_reaction_id

```cpp
definition_id<reaction_view> default_reaction_id(elemental_reaction slot) const noexcept;
```

取得编译配置指定的默认反应定义 ID。`slot` 必须为 17 个有效反应槽位之一，不能传入 `none`。此值不包含某方牌组带来的替换；运行时通过 `table[reaction_id{player, slot}].definition_id()` 取得实际定义。

反应生成的实体由反应定义声明名称依赖，并通过 `resolve_id` 按名称取得；库不再提供五个特殊生成实体 ID。
