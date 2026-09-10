[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **damage_source_id**

# givm::damage_source_id

定义于头文件 `<givm/executor/events.hpp>`

```cpp
using damage_source_id = std::variant<hand_card_id, hand_card_status_id, deck_card_status_id, support_id,
                 summon_id, combat_status_id, character_id, skill_id, attachment_id>;
```

伤害的来源标识。它保留造成伤害的具体实体种类，便于区分角色、技能和场上效果造成的伤害。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::damage_source_id source{ givm::character_id{ .player_id = givm::player_id{ 0 }, .index = 0 } };
    std::println("来源是角色: {}", std::holds_alternative<givm::character_id>(source));
}
```

输出

```text
来源是角色: true
```
