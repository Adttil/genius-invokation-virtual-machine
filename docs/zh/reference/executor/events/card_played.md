[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **card_played**

# givm::card_played

定义于头文件 `<givm/executor.hpp>`

```cpp
struct card_played;
```

打出手牌完成后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |
| `definition_id` | `const definition_id<card_definition>` | 对应的牌定义标识；只读 |
| `target` | `const card_target_id` | 卡牌效果最终采用的目标；只读 |
| `speed` | [`const action_speed`](../../enums/action_speed.md) | 行动速度；只读 |
| `effect_cancelled` | `const bool` | 本次卡牌效果是否已被取消；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::card_played event{ .card = {}, .definition_id = {}, .target = givm::character_id{}, .speed = givm::action_speed::combat, .effect_cancelled = false };
    std::println("目标是角色: {}", std::holds_alternative<givm::character_id>(event.target));
    std::println("快速行动: {}", event.speed == givm::action_speed::fast);
}
```

输出

```text
目标是角色: true
快速行动: false
```
