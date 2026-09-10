[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **element_application_source_id**

# givm::element_application_source_id

定义于头文件 `<givm/executor/events.hpp>`

```cpp
using element_application_source_id = std::variant<hand_card_id, hand_card_status_id, deck_card_status_id, support_id,
                 summon_id, combat_status_id, character_id, skill_id, attachment_id>;
```

元素附着的来源标识。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::element_application_source_id source{ givm::summon_id{} };
    std::println("来源是召唤物: {}", std::holds_alternative<givm::summon_id>(source));
}
```

输出

```text
来源是召唤物: true
```
