[givm](../../reference.md) / [牌桌](../table.md) / **action_cost_requirement**

# givm::action_cost_requirement

定义于头文件 `<givm/table.hpp>`

```cpp
struct action_cost_requirement;
```

一项行动的骰子、充能费用与行动速度。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `dice_requirement` | [`elemental_dice_requirement`](../enums/elemental_dice_requirement.md) | 这项行动需要的骰子 |
| `speed` | [`action_speed`](../enums/action_speed.md) | 行动速度 |
| `energy` | `std::uint32_t` | 从确认时锁定的原付费角色扣除的充能数量，默认为零 |
| `energy_tag` | [`optional_tag_id`](optional_tag_id.md) | 所需充能类型；默认空值表示普通充能，非空值表示对应标签的替代充能 |

## 注意

支付输入仅选择骰子。确认时锁定原付费角色，费用效果结束后从该角色按当前充能饱和扣除，费用效果切换出战不会改换付费对象。充能费用非零时，确认前的支付检查先要求角色状态的 `energy_tag` 与费用的 `energy_tag` 相等，再检查数量是否足够；充能费用为零时忽略 `energy_tag`。卡牌、技能、特技与切换使用同一费用结构。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::action_cost_requirement requirement{};
    requirement.dice_requirement.any = 1;
    requirement.speed = givm::action_speed::fast;
    std::println("任意骰需求: {}", requirement.dice_requirement.any);
    std::println("快速行动: {}", requirement.speed == givm::action_speed::fast);
}
```

输出

```text
任意骰需求: 1
快速行动: true
```
