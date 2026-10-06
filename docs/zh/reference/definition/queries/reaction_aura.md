[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **reaction_aura**

# givm::reaction_aura

```cpp
struct reaction_aura {
    const elemental_reaction slot;
    const element_aura reacted_aura;
    const element incoming_element;
};
```

查询反应定义在给定原附着和输入元素下的新附着，返回 `element_aura`。默认按基础元素规则计算。结果预填到 `elemental_reaction_will_occur::new_aura`，普通响应可以继续修改，无须使用 optional。
