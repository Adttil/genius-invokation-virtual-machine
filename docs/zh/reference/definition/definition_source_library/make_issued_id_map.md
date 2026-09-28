[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **make_issued_id_map**

# givm::definition_source_library::make_issued_id_map

定义于头文件 `<givm/definition.hpp>`

```cpp
std::expected<issued_id_map, std::vector<source_preparation_error>>
make_issued_id_map(const basic_definition_sources& basics) const; // (1)

std::expected<issued_id_map, std::vector<source_preparation_error>>
make_issued_id_map(const basic_definition_sources& basics, const definition_selection& selection) const; // (2)
```

为所需定义准备名称与 ID 的对应关系，便于在编写对局流程、准备牌组前取得定义 ID。

(1) 选择全部已登记定义及 `basics` 中的四个默认反应定义。(2) 选择指定定义以及 `basics` 中的四个默认反应定义，并自动包含这些定义直接或间接声明的名称依赖。即使 `selection` 为空，也保留默认反应定义及其依赖。编译中的标签筛选仅查询这个集合，不会扩充选择范围。

## 参数

|  |  |
| --- | --- |
| `basics` | [`basic_definition_sources`](../basic_definition_sources.md)，本次映射采用的四个默认反应源 |
| `selection` | 各类别首先选择的定义名称 |

## 返回值

成功时返回含有 [`issued_id_map`](../issued_id_map.md) 的 `expected`。同一源库内容、相同 `basics` 及相同选择范围产生的映射可与随后编译的定义库配套。

选择了未知定义、基础定义与源库同类别名称冲突或名称依赖缺失时，返回 [`source_preparation_error`](../source_preparation_error.md) 列表；可通过 [`error_string`](../error_string.md) 输出。不为这些验证错误抛出异常，也不返回部分有效的映射。

## 注意

本函数不调用定义源的 `compile`，也不修改源库。四个基础源仅参与本次映射准备；它们所依赖的其他源须在源库或这四个源中。已有同类别同名项只有引用相同源对象且类型相同时才会去重。

改变源库内容、基础定义配置或选择集合后，应重新建立映射，不混用之前发放的 ID。随后调用 [`compile`](../../executor/compile.md) 时必须使用相同的 `basics`。

## 示例

```cpp
#include <utility>
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
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(potion, food)) return 1;
    const std::array<std::string_view, 1> names{ "恢复药剂" };
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::card_definition>()] = names;
    auto ids_result = sources.make_issued_id_map(basics, selection);
    if(not ids_result)
    {
        std::println("{}", error_string(ids_result.error()));
        return 1;
    }
    const auto ids = std::move(*ids_result);
    std::println("包含恢复药剂: {}", ids.has<givm::card_definition>("恢复药剂"));
    std::println("包含恢复料理: {}", ids.has<givm::card_definition>("恢复料理"));
}
```

输出

```text
包含恢复药剂: true
包含恢复料理: false
```
