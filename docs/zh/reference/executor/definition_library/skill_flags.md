[givm](../../../reference.md) / [definition_library](../definition_library.md) / **skill_flags**

# skill_flags

```cpp
givm::skill_flags skill_flags(definition_id<skill_view> id) const noexcept;
```

按照技能定义的 `normal_attack`、`elemental_skill`、`elemental_burst` 标签取得固有分类，可同时具有多类。结果不包含由本次行动决定的重击或下落攻击。
