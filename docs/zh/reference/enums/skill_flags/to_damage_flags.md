[givm](../../../reference.md) / [skill_flags](../skill_flags.md) / **to_damage_flags**

# to_damage_flags

```cpp
damage_flags to_damage_flags() const noexcept;
```

所有函数均为 `constexpr`。将五种技能性质转换为对应伤害性质，并添加 `skill_damage`。不添加 `combat_damage`，也不会向后续命令隐式传播；定义在填充伤害时显式使用本结果。
