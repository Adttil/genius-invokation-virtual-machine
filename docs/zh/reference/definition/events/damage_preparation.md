[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_preparation**

# givm::damage_preparation

每次伤害开始时的即时属性修饰。`source`、`target`、`type`、`flags` 可以修改；基础 `value`、倍率分子和分母只读。属性确定后读取目标原附着、判断反应，再进入 `damage_calculation`。各次伤害依命令顺序执行，不预先准备整组伤害。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::damage_preparation event{
        .source = givm::character_id{}, .target = {}, .value = 2,
        .type = givm::damage_type::physical,
        .flags = givm::damage_flags{ givm::damage_flag_bits::skill_damage }
            | givm::damage_flag_bits::normal_attack
    };
    event.type = givm::damage_type::cryo;
    std::println("附魔后是冰伤害: {}", event.type == givm::damage_type::cryo);
    std::println("仍是普通攻击伤害: {}", event.flags.contains(givm::damage_flag_bits::normal_attack));
}
```

输出

```text
附魔后是冰伤害: true
仍是普通攻击伤害: true
```

## 参阅

| | |
| --- | --- |
| [`damage_calculation`](damage_calculation.md) | 属性确定后的伤害数值计算 |
| [`deal_damage`](../commands/deal_damage.md) | 单次或范围伤害 |
