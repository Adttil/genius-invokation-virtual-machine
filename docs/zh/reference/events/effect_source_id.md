[givm](../../reference.md) / [事件](../events.md) / **effect_source_id**

# givm::effect_source_id

定义于头文件 `<givm/executor/events.hpp>`

```cpp
using effect_source_id = std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id,
                 support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>;
```

治疗等效果的来源标识，能够表示牌、角色、技能以及其他场上实体。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::effect_source_id source{ givm::support_id{} };
    std::println("来源是支援: {}", std::holds_alternative<givm::support_id>(source));
}
```

输出

```text
来源是支援: true
```
