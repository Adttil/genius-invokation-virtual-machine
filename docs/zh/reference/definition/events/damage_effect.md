[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_effect**

# givm::damage_effect

定义于头文件 `<givm/definition.hpp>`

```cpp
struct damage_effect;
```

扣除生命前的伤害结算事件。护盾和其他减伤效果可以在这里减少最终伤害。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const damage_source_id` | 本次伤害的来源；只读 |
| `target` | `const character_id` | 将扣除生命的角色；只读 |
| `value` | `std::uint32_t` | 将用于扣除生命的伤害值，可由响应者减少 |
| `type` | `const damage_type` | 伤害种类；只读 |
| `flags` | `const damage_flags` | 伤害附加属性；只读 |
| `reaction` | `const elemental_reaction` | 属性修饰后已判定的反应，默认为 none；只读 |
| `replacement_reaction` | `const tag_id` | 已确定的替代反应标签，空值表示使用默认反应效果；只读 |

## 时机

本事件发生在 [`damage_calculation`](damage_calculation.md) 的加伤、默认反应加成及倍率计算之后，扣除生命之前。`value` 已经包含这些数值计算的结果；护盾与减伤在此调整最终用于扣血的数额。

`reaction` 沿用属性修饰结束时的判定，便于响应判断本次伤害是否引起某种反应；`replacement_reaction` 沿用数值计算前确定的标签。替代反应仍保留原始反应种类，此后目标的附着即使发生变化，也不重新判定本次反应。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::damage_effect event{ .source = givm::character_id{}, .target = {}, .value = 3, .type = givm::damage_type::physical, .flags = {} };
    // 抵挡 2 点伤害。
    event.value -= 2;
    std::println("剩余伤害: {}", event.value);
}
```

输出

```text
剩余伤害: 1
```

## 参阅

| | |
| --- | --- |
| [`modify_combat_status_state`](../commands/modify_combat_status_state.md) | 按增量修改出战状态的层数和本回合次数 |
| [`deal_damage`](../commands/deal_damage.md) | 伤害结算命令 |
