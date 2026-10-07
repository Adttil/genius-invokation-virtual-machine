[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **element_application_source_id**

# givm::element_application_source_id

定义于头文件 `<givm/definition.hpp>`

```cpp
using element_application_source_id = variant_entity_id<entity_category::hand_card, entity_category::deck_card, entity_category::hand_card_status, entity_category::deck_card_status, entity_category::support, entity_category::summon, entity_category::combat_status, entity_category::character, entity_category::skill, entity_category::attachment>;
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
    std::println("来源是召唤物: {}", source.template holds<givm::entity_category::summon>());
}
```

输出

```text
来源是召唤物: true
```
