[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_target_id**

# givm::card_target_id

定义于头文件 `<givm/definition.hpp>`

```cpp
using card_target_id = variant_entity_id<entity_category::null, entity_category::character, entity_category::support, entity_category::summon>;
```

卡牌效果的目标标识。没有目标时保存 `null` 空类别，其余情况分别表示角色、支援或召唤物。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::card_target_id target{};
    std::println("初始没有目标: {}", (not target));
    target = givm::character_id{ givm::player_id{ 1 }, 0  };
    std::println("选择角色目标: {}", target.holds<givm::entity_category::character>());
}
```

输出

```text
初始没有目标: true
选择角色目标: true
```
