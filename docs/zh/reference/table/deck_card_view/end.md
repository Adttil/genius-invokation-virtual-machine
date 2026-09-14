[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_view](../deck_card_view.md) / **end**

# givm::deck_card_view::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const deck_card_view* end() const;
```

取得这个牌库卡牌的单实体范围终点。

## 返回值

实体有效时为单实体范围的尾后指针，否则与 `begin()` 相等。


## 示例

```cpp
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct example_source
{
    using definition_category = givm::card_definition;
    struct definition_type {};
    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    example_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{});
    const auto definition = ids.get_id<givm::card_definition>("示例");
    givm::card_table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .cards = { definition } });
    const givm::deck_card_view view = table[givm::deck_card_id{ givm::player_id{ 0 }, 0 }];
    std::println("范围长度: {}", view.end() - view.begin());
}
```

输出

```text
范围长度: 1
```
