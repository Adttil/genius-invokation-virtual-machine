[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **character_revived**

# givm::character_revived

```cpp
struct character_revived { const character_id target; };
```

已死亡角色通过 `healing_kind::revive` 实际恢复非零生命并重新存活后的通知。它进入当前段的混合通知，排在对应 `healed` 前。免于击倒时角色仍存活，不产生本事件。
