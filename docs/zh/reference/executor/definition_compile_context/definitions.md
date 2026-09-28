[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **definitions**

# givm::definition_compile_context::definitions

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TCategory>
auto definitions() const noexcept;
```

遍历本次编译集合中指定类别的全部定义，用于按名称、标签和能力组合筛选效果对象。

## 模板参数

| | |
| --- | --- |
| `TCategory` | 要遍历的定义类别 |

## 返回值

由 [`definition_view<TCategory>`](definition_view.md) 组成的只读范围，满足 `std::ranges::sized_range`，按定义 ID 顺序排列。没有该类别的定义时返回空范围。

范围仅包含本次选定的定义及其名称依赖、基础定义构成的集合，不包含源库中未被选中的定义。

## 注意

不需要声明依赖，遍历和筛选不会扩充编译集合。范围与元素视图仅在本次编译期间有效；要保存到编译后的配置中，应提取 ID 或复制所需数据。

## 示例

```cpp
#include <utility>
#include <array>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct food_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "示例料理"; }
    auto tags() const { return std::array<std::string_view, 1>{ "料理" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
    static givm::program_entry handle(const int&, const givm::hand_card_view&,
        givm::card_effect&, givm::handle_context&)
    { return {}; }
};

struct search_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "料理检索"; }

    int compile(givm::definition_compile_context& context) const
    {
        const auto cards = context.definitions<givm::card_definition>();
        std::println("当前卡牌定义数: {}", std::ranges::size(cards));
        for(const auto card : cards)
        {
            if(card.has_tag("料理") && card.can_handle<givm::card_effect, givm::hand_card_view>())
            {
                std::println("可打出的料理牌: {}", context[card.id()].name());
                std::println("自定义初始状态查询: {}", card.has_query<givm::card_initial_state>());
            }
        }
        std::println("存在未选择的牌: {}",
            context.find_definition<givm::card_definition>("未选择的牌").has_value());
        return 0;
    }
};

int main()
{
    const food_source food{};
    const search_source search{};
    auto sources = givm::make_definition_source_library(food, search);
    if(not sources) return 1;
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    auto library_result = compile(*sources, basics,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
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
当前卡牌定义数: 2
可打出的料理牌: 示例料理
自定义初始状态查询: false
存在未选择的牌: false
```
