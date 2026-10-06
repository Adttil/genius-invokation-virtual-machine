[givm](../../../../reference.md) / [执行](../../../executor.md) / **health_reduced::reaction**

# reaction

```cpp
reaction_id reaction() const;
```

本击判定的反应映射玩家和槽位；没有反应时其布尔值为 false。以 `reaction().slot` 判断基础反应，以 `table[reaction()].definition_id()` 查看所选定义。
