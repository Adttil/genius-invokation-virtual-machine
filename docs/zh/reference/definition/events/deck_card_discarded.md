[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **deck_card_discarded**

# givm::deck_card_discarded

定义于头文件 `<givm/definition.hpp>`

```cpp
struct deck_card_discarded;
```

[舍弃牌堆牌命令](../commands/discard_deck_card.md)产生的通知，也是该命令采用动态参数时的输入。通知先发给被舍弃牌自身；其响应程序完成后再向全场广播。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const deck_card_id` | 被舍弃的牌堆牌 ID；只读 |

## 注意

响应时牌及其附属状态已经离场，仍可按 ID 查询保留的信息。该牌自身仅接收前面的自身通知，不参加后面的全场广播。抽牌、转移到手牌和超过手牌上限不产生此事件。

## 示例

```cpp
#include <print>
#include <givm/definition.hpp>

int main()
{
    const givm::deck_card_id card{ givm::player_id{ 1 }, 1 };
    const givm::deck_card_discarded event{ .card = card };
    std::println("通知指定牌堆牌: {}", event.card == card);
}
```

输出

```text
通知指定牌堆牌: true
```
