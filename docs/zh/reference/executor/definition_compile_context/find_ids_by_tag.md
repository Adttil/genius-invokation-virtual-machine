[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **find_ids_by_tag**

# givm::definition_compile_context::find_ids_by_tag

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class TCategory>
std::vector<definition_id<TCategory>> find_ids_by_tag(std::string_view filter) const;
```

从本次编译集合中取得满足标签条件的一组定义，适合编写从当前规则提供的某类卡牌中检索或生成卡牌的效果。

## 模板参数

|  |  |
| --- | --- |
| `TCategory` | 要筛选的定义类别 |

## 参数

|  |  |
| --- | --- |
| `filter` | 标签筛选表达式；以 `&` 连接条件，`!标签` 表示排除该标签 |

## 返回值

匹配条件的定义 ID 序列，按配套映射的 ID 分配顺序排列。无匹配时返回空序列。

## 注意

不需要预先声明筛选表达式。查询只查看选定定义及其按名称依赖形成的闭包，不会把源库中其他匹配定义加入编译结果。

条件两侧允许空白，各条件不得为空，不支持括号或 `|`。未知的正向标签使结果为空；未知的排除标签不限制结果。例如未定义 `料理` 标签时，`治疗 & !料理` 与 `治疗` 的结果相同。

## 示例

```cpp
#include <utility>
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct potion_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

struct search_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "治疗检索"; }
    auto compile(givm::definition_compile_context& context) const
    {
        auto cards = context.find_ids_by_tag<givm::card_definition>("治疗 & !料理");
        std::println("可检索的治疗牌数量: {}", cards.size());
        return cards;
    }
};

int main()
{
    const potion_source potion{};
    const search_source search{};
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0,
        givm::genshin_impact::shield_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(potion, search)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
}
```

输出

```text
可检索的治疗牌数量: 1
```
