[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_view](../deck_card_view.md) / **begin**

# givm::deck_card_view::begin

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto begin(this const auto& self);
```

取得这个牌库卡牌的单实体范围起点。

## 参数

|  |  |
| --- | --- |
| `self` | 当前实体的只读视图 |

## 返回值

单实体范围的起始迭代器。实体有效时，解引用取得该实体的只读 [`deck_card_view`](../deck_card_view.md)；实体无效时与 `end()` 相等。

## 注意

范围依赖访问对象本身的存活。遍历有效实体时执行一次循环；遍历已移除实体时不执行循环。

## 示例

```cpp
#include <utility>
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct example_source
{
    static constexpr auto category = givm::definition_category::card;
    struct definition_type {};
    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    example_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    const auto definition = ids.get_id<givm::definition_category::card>("示例");
    givm::table table{};
    load_deck(table, library, givm::linked_deck{ .cards = { definition } }, {});
    const givm::deck_card_view view = table[givm::deck_card_id{ givm::player_id{ 0 }, 0 }];
    for(const auto& item : view)
    {
        std::println("范围中的实体: {}", item.id() == view.id());
    }
}
```

输出

```text
范围中的实体: true
```
