[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_source_id**

# givm::damage_source_id

定义于头文件 `<givm/definition.hpp>`

```cpp
using damage_source_id = variant_entity_id<entity_category::hand_card, entity_category::deck_card, entity_category::hand_card_status, entity_category::deck_card_status, entity_category::support, entity_category::summon, entity_category::combat_status, entity_category::character, entity_category::skill, entity_category::attachment>;
```

伤害的来源标识。它保留造成伤害的具体实体种类，便于区分角色、技能和场上效果造成的伤害。

牌堆牌的自身舍弃效果可以使用 `deck_card_id` 作为来源；该牌此时已离场，结算期间仍可读取其定义与状态。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::damage_source_id source{ givm::character_id{ givm::player_id{ 0 }, 0  } };
    std::println("来源是角色: {}", source.template holds<givm::entity_category::character>());
}
```

输出

```text
来源是角色: true
```
