[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **resolve_ids_by_tag**

# givm::definition_compile_context::resolve_ids_by_tag

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TCategory>
std::vector<definition_id<TCategory>> resolve_ids_by_tag(std::string_view filter) const;
```

取得满足已声明标签条件的一组定义，适合编写从某类卡牌中检索或生成卡牌的效果。

## 模板参数

|  |  |
| --- | --- |
| `TCategory` | 要筛选的定义类别 |

## 参数

|  |  |
| --- | --- |
| `filter` | 本源在对应 `*_dependencies_by_tag()` 中声明的完整表达式 |

## 返回值

匹配条件的定义 ID 序列，按配套映射的 ID 分配顺序排列。

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 完整表达式未在本源的对应依赖声明中出现 |

## 注意

筛选使用 [`query_by_tag`](../../definition/issued_id_map/query_by_tag.md) 的语法。声明与查询的字符串须完全一致，不能只在逻辑上等价。

## 示例

```cpp
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
    auto card_dependencies_by_tag() const
    { return std::array<std::string_view, 1>{ "治疗 & !料理" }; }

    auto compile(givm::definition_compile_context& context) const
    {
        auto cards = context.resolve_ids_by_tag<givm::card_definition>("治疗 & !料理");
        std::println("可检索的治疗牌数量: {}", cards.size());
        return cards;
    }
};

int main()
{
    const potion_source potion{};
    const search_source search{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(potion, search);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
}
```

输出

```text
可检索的治疗牌数量: 1
```
