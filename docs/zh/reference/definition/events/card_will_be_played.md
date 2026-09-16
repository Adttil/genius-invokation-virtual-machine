[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_will_be_played**

# givm::card_will_be_played

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_will_be_played;
```

手牌效果生效前的事件。响应者可以反制这张牌，取消其原效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |
| `definition_id` | `const definition_id<card_definition>` | 对应的牌定义标识；只读 |
| `targets` | `const std::array<card_target_id, 2>` | 本次采用的两个目标位置；只读 |
| `speed` | [`const action_speed`](../../enums/action_speed.md) | 费用计算确定的行动速度；只读 |
| `effect_cancelled` | `bool` | 是否取消卡牌效果，初始为 false |

## 注意

此时牌已经离手，费用效果与骰子支付均已完成。反制只取消本牌的 [`card_effect`](card_effect.md)，不退还费用，也不撤销离手；之后仍发出 [`card_played`](card_played.md)。目标与行动速度在确认出牌时确定，响应不能改写。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::card_will_be_played event{ .card = {}, .definition_id = {}, .targets = { givm::character_id{}, {} }, .speed = givm::action_speed::combat };
    event.effect_cancelled = true;
    std::println("首个目标是角色: {}", std::holds_alternative<givm::character_id>(event.targets[0]));
    std::println("原效果被反制: {}", event.effect_cancelled);
}
```

输出

```text
首个目标是角色: true
原效果被反制: true
```
