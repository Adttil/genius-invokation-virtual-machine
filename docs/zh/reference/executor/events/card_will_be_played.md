[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **card_will_be_played**

# givm::card_will_be_played

定义于头文件 `<givm/executor.hpp>`

```cpp
struct card_will_be_played;
```

手牌效果生效前的事件。响应者可以调整目标、行动速度，或取消这张牌的效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |
| `definition_id` | `const definition_id<card_definition>` | 对应的牌定义标识；只读 |
| `target` | [`card_target_id`](card_target_id.md) | 卡牌效果的目标，可在生效前调整 |
| `speed` | [`action_speed`](../../enums/action_speed.md) | 行动速度 |
| `effect_cancelled` | `bool` | 是否取消卡牌效果，初始为 false |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::card_will_be_played event{ .card = {}, .definition_id = {}, .target = givm::character_id{}, .speed = givm::action_speed::combat, .effect_cancelled = false };
    event.speed = givm::action_speed::fast;
    std::println("目标是角色: {}", std::holds_alternative<givm::character_id>(event.target));
    std::println("快速行动: {}", event.speed == givm::action_speed::fast);
}
```

输出

```text
目标是角色: true
快速行动: true
```
