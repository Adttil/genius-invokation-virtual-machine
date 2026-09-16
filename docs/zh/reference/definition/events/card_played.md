[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_played**

# givm::card_played

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_played;
```

打出手牌完成后的通知。在本牌原效果完成后发出；原效果被反制时也会发出。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |
| `definition_id` | `const definition_id<card_definition>` | 对应的牌定义标识；只读 |
| `targets` | `const std::array<card_target_id, 2>` | 本次采用的两个目标位置；只读 |
| `speed` | [`const action_speed`](../../enums/action_speed.md) | 行动速度；只读 |

## 注意

通知只说明这次出牌已经完成，不提供反制结果字段。牌已离开手牌，但在牌桌 [`clean_up`](../../table/table/clean_up.md) 前仍可通过其 ID 读取原牌信息；普通手牌遍历及广播跳过该牌。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::card_played event{ .card = {}, .definition_id = {}, .targets = { givm::character_id{}, {} }, .speed = givm::action_speed::combat };
    std::println("首个目标是角色: {}", std::holds_alternative<givm::character_id>(event.targets[0]));
    std::println("快速行动: {}", event.speed == givm::action_speed::fast);
}
```

输出

```text
首个目标是角色: true
快速行动: false
```
