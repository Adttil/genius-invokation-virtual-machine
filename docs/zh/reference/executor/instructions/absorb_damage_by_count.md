[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **absorb_damage_by_count**

# givm::absorb_damage_by_count

定义于头文件 `<givm/executor.hpp>`

```cpp
struct absorb_damage_by_count;
```

消耗当前响应实体计数以抵挡伤害的指令。它适用于护盾等需要随着吸收伤害而消耗计数的效果。

## 成员类型

| | |
| --- | --- |
| `context_type` | [`damage_effect`](../events/damage_effect.md)，表示伤害生效前的事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `maximum_count` | `std::uint32_t` | 本次最多消耗的计数，初始为 uint32_t 的最大值 |

## 注意

仅能用于 [`damage_effect`](../events/damage_effect.md) 的响应，当前响应实体必须具有 `state().count`。吸收量等于剩余伤害、实体计数和 `maximum_count` 三者的最小值；伤害与计数同时减少该值。是否属于己方护盾、是否忽略护盾等适用性判断，由实体的事件响应决定。

## 参阅

| | |
| --- | --- |
| [`damage_effect`](../events/damage_effect.md) | 扣除生命前的伤害结算事件 |
