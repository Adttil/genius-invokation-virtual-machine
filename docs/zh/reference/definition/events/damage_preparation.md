[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_preparation**

# givm::damage_preparation

定义于头文件 `<givm/definition.hpp>`

```cpp
struct damage_preparation;
```

伤害的属性修饰事件。附魔、伤害归属和目标调整在这里完成；之后的数值加成统一读取修饰后的属性。例如先处理冰附魔，再判断冰元素伤害加成，不受这两种效果的入场先后影响。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`damage_source_id`](damage_source_id.md) | 本次伤害的来源，可修改 |
| `target` | [`character_id`](../../table/character_id.md) | 受到本次伤害的角色，可修改 |
| `value` | `const std::uint32_t` | 进入本阶段时的伤害值；只读 |
| `multiplier_numerator` | `const std::uint16_t` | 初始伤害描述中的倍率分子，默认为 1；只读 |
| `multiplier_denominator` | `const std::uint16_t` | 初始伤害描述中的非零倍率分母，默认为 1；只读 |
| `type` | [`damage_type`](../../enums/damage_type.md) | 伤害种类，可用于附魔等元素转换 |
| `flags` | [`damage_flags`](../../enums/damage_flags.md) | 本段伤害的附加属性，可修改 |

## 时机

[`deal_damage`](../commands/deal_damage.md) 为每个具体目标先广播本事件，再按修饰后的目标、元素和目标此时的附着判定一次元素反应。有反应时先通过 [`elemental_reaction_will_occur`](elemental_reaction_will_occur.md) 确定替代标签，随后进入 [`damage_calculation`](damage_calculation.md)。数值加成与倍率在后者处理；减伤和护盾在 [`damage_effect`](damage_effect.md) 处理。

本事件中的多个属性修饰仍按[全场广播顺序](../events.md#全场广播)依次执行，后面的响应读取前面已经修改的属性。各阶段分别确定响应者；某阶段开始后新建的实体不会加入该阶段，可能参与尚未开始的后续阶段。

`flags` 描述本段伤害被视为什么伤害。修改 `normal_attack`、`elemental_skill` 或 `elemental_burst` 不会改变实际使用的技能，也不会改写技能使用通知中的 `skill_id`。

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
| [`deal_damage`](../commands/deal_damage.md) | 一组伤害的完整结算 |
