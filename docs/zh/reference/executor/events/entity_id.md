[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **entity_id**

# givm::entity_id

定义于头文件 `<givm/executor.hpp>`

```cpp
using entity_id = std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id,
                 support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>;
```

各类牌桌实体的标识。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::entity_id entity{ givm::character_id{} };
    std::println("离场对象是角色: {}", std::holds_alternative<givm::character_id>(entity));
}
```

输出

```text
离场对象是角色: true
```
