[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_calculation**

# givm::damage_calculation

定义于头文件 `<givm/definition.hpp>`

```cpp
struct damage_calculation;
```

伤害的数值计算事件。附魔和伤害归属已经确定，响应者可以据此调整基础伤害与倍率，并读取本次已判定的元素反应及其替代标签。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const damage_source_id` | 属性修饰后的伤害来源；只读 |
| `target` | `const character_id` | 属性修饰后的伤害目标；只读 |
| `value` | `std::uint32_t` | 应用伤害倍率前的伤害值 |
| `multiplier_numerator` | `std::uint16_t` | 伤害倍率的分子，继承初始伤害描述，默认为 1 |
| `multiplier_denominator` | `std::uint16_t` | 伤害倍率的非零分母，继承初始伤害描述，默认为 1 |
| `type` | `const damage_type` | 属性修饰后的伤害种类；只读 |
| `flags` | `const damage_flags` | 属性修饰后的伤害附加属性；只读 |
| `reaction` | `const elemental_reaction` | 属性修饰结束后判定的反应，默认为 none；只读 |
| `reacted_aura` | `const element_aura` | 判定本次反应时目标的完整附着，默认为 none；只读 |
| `replacement_reaction` | `const tag_id` | 已确定的替代反应标签，空值表示使用默认反应效果；只读 |

## 注意

[`damage_preparation`](damage_preparation.md) 结束后，按最终 `target`、`type` 和目标当时的附着判定反应；有反应时先完成 [`elemental_reaction_will_occur`](elemental_reaction_will_occur.md) 的标签选择，再开始本事件。之后即使响应效果改变目标的附着，本次伤害的 `reaction` 与 `reacted_aura` 也保持不变；后续伤害效果、扣血及元素反应处理沿用这次判定。

本事件结束后，只有 `replacement_reaction` 为空时才加入默认反应加伤，再统一应用倍率。非空标签同时取消该反应的默认派生伤害、实体生成和超载切人，但不改变原始 `reaction`，也不影响默认附着消耗。响应者可根据只读标签调整数值或在后续事件中完成替代效果。

同组各次伤害分别广播本事件；此前伤害的扣血和附着已经生效，但本组的伤害后响应尚未调用。

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
