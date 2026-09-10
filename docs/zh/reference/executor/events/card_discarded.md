[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **card_discarded**

# givm::card_discarded

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct card_discarded;
```

手牌或牌堆中的牌被弃置后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const card_id` | 本次事件对应的牌标识；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::card_discarded event{ .card = givm::deck_card_id{} };
    std::println("从牌堆弃置: {}", std::holds_alternative<givm::deck_card_id>(event.card));
}
```

输出

```text
从牌堆弃置: true
```
