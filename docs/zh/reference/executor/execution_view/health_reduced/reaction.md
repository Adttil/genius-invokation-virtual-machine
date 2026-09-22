[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view](../../execution_view.md) / [health_reduced](../health_reduced.md) / **reaction**

# givm::execution_view<execution_state::health_reduced>::reaction

定义于头文件 `<givm/executor.hpp>`

```cpp
elemental_reaction reaction() const noexcept;
```

取得本次伤害引起的元素反应种类。它在伤害属性修饰结束后、数值计算之前确定，并用于后续反应加伤与元素处理。

## 返回值

本次伤害的 [`elemental_reaction`](../../../enums/elemental_reaction.md)；没有反应时为 `elemental_reaction::none`。此值不会根据观察时的牌桌附着重新判定。

## 注意

到达本现场时已经扣血，但该次元素附着与反应派生效果尚未进行。若继续推进时判定终局，则不再执行这些后续效果。
