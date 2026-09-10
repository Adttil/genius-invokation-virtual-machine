[givm](../../reference.md) / [事件](../events.md) / **counted_entity_id**

# givm::counted_entity_id

定义于头文件 `<givm/executor/events.hpp>`

```cpp
using counted_entity_id = std::variant<hand_card_status_id, deck_card_status_id, support_id, summon_id,
                 combat_status_id, attachment_id>;
```

具有计数的实体标识，用于报告状态、支援和召唤物等对象的次数或层数变化。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::counted_entity_id entity{ givm::combat_status_id{} };
    std::println("计数对象是战斗状态: {}", std::holds_alternative<givm::combat_status_id>(entity));
}
```

输出

```text
计数对象是战斗状态: true
```
