[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_calculation**

# givm::damage_calculation

伤害属性确定后、扣血前的即时计算。`value`、`multiplier_numerator`、`multiplier_denominator` 和 `cancel_reaction_bonus` 可修改，来源、目标、类型、属性、`reaction_id reaction` 和原附着 `reacted_aura` 只读。

普通响应一次广播内完成加值和倍率修饰；除非 `cancel_reaction_bonus` 为 true，随后告知所选反应定义完成反应加伤。最后统一应用倍率、向上取整，再进入 `damage_effect`。不会先为后续伤害提前推进附着；每条命令读取当前牌桌。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::damage_calculation event{ .source = givm::character_id{}, .target = {}, .value = 3, .type = givm::damage_type::physical, .flags = {} };
    // 增加 1 点伤害，并把倍率调整为 2 倍。
    ++event.value;
    event.multiplier_numerator = 2;
    std::println("基础伤害: {}", event.value);
    std::println("伤害倍率: {}/{}", event.multiplier_numerator, event.multiplier_denominator);
}
```

输出

```text
基础伤害: 4
伤害倍率: 2/1
```

## 参阅

| | |
| --- | --- |
| [`damage_preparation`](damage_preparation.md) | 伤害属性修饰 |
| [`damage_effect`](damage_effect.md) | 最终数值的减伤与护盾处理 |
| [`deal_damage`](../commands/deal_damage.md) | 伤害结算命令 |
