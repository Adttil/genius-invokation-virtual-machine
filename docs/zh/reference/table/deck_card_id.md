[givm](../../reference.md) / [牌桌](../table.md) / **deck_card_id**

# givm::deck_card_id

定义于头文件 `<givm/table.hpp>`

```cpp
class deck_card_id;
```

牌库卡牌在一张牌桌中的身份。使用此 ID 可以通过 [`table::operator[]`](table/operator_subscript.md) 再次取得相应实体。

## 成员函数

| 名称 | 说明 |
| --- | --- |
| [`(构造函数)`](deck_card_id/constructor.md) | 默认构造保持平凡，未初始化的 ID 须先赋值 |
| [`operator=`](deck_card_id/operator_assign.md) | 复制或移动同类 ID |
| [`value`](deck_card_id/value.md) | 取得不含类别标志的完整编码字，供读取或保存 |
| [`index`](deck_card_id/index.md) | 取得当前实体的索引 |
| [`player_id`](deck_card_id/player_id.md) | 取得所属玩家的 ID |
| [`operator==`](deck_card_id/operator_equal.md) | 比较同类 ID 的身份，不检查实体是否在场，也不区分属于哪张牌桌 |

## 非成员函数

```cpp
friend constexpr bool operator==(deck_card_id, deck_card_id) = default;
```

比较编码的身份是否相等；比较不检查实体是否尚未移除。

## 注意

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。

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
    const givm::deck_card_id id = view.id();
    std::println("通过 ID 取得定义: {}", library[table[id].definition_id()].name());
}
```

输出

```text
通过 ID 取得定义: 示例
```
