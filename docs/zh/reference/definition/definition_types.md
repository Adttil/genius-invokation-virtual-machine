[givm](../../reference.md) / [定义](../definition.md) / **definition_types**

# givm::definition_types

定义于头文件 `<givm/definition.hpp>`

```cpp
using definition_types = type_list<
    card_definition,
    status_definition,
    support_view,
    summon_view,
    combat_status_view,
    character_view,
    skill_view,
    attachment_view
>;
```

定义系统支持的全部实体定义类别。编写通用的定义管理操作，或按类别填写对局选择时，可以通过这份类型列表枚举与定位各类别。

## 注意

成员操作继承自 [`type_list`](../utils/type_list.md)。卡牌与卡牌状态各自包含手牌和牌堆两种实体形态，其他类别对应各自的只读 view。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view source_name;

    std::string_view name() const { return source_name; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{ "恢复药剂" };
    const card_source food{ "恢复料理" };
    givm::definition_source_library sources{};
    sources.add(potion, food);
    const std::array<std::string_view, 1> names{ "恢复药剂" };
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::card_definition>()] = names;
    const auto ids = sources.make_issued_id_map(selection);
    std::println("包含恢复药剂: {}", ids.has<givm::card_definition>("恢复药剂"));
    std::println("包含恢复料理: {}", ids.has<givm::card_definition>("恢复料理"));
}
```

输出

```text
包含恢复药剂: true
包含恢复料理: false
```
