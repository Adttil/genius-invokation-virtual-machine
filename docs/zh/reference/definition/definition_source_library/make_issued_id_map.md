[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **make_issued_id_map**

# givm::definition_source_library::make_issued_id_map

定义于头文件 `<givm/definition.hpp>`

```cpp
issued_id_map make_issued_id_map() const; // (1)
issued_id_map make_issued_id_map(const definition_selection& selection) const; // (2)
```

为所需定义准备名称与 ID 的对应关系，便于在编写对局流程、准备牌组前取得定义 ID。

(1) 选择全部已登记定义。(2) 选择指定定义以及源库构造时选定的三个默认反应定义，并自动包含这些定义直接或间接声明的名称依赖和标签筛选依赖。即使 `selection` 为空，也保留默认反应定义及其依赖。

## 参数

|  |  |
| --- | --- |
| `selection` | 各类别首先选择的定义名称 |

## 返回值

与选定定义对应的 [`issued_id_map`](../issued_id_map.md)。同一源库内容及相同最终选择集合产生的映射可与随后编译的定义库配套。

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 选择了未登记的定义名称 |

## 注意

本函数不调用定义源的 `compile`。改变源库内容或选择集合后，应重新建立映射，不混用之前发放的 ID。

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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
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
