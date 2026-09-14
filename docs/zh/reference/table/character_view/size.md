[givm](../../../reference.md) / [牌桌](../../table.md) / [character_view](../character_view.md) / **size**

# givm::character_view::size

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr size_t size() const;
```

将这个角色作为单实体范围时，取得其中的元素数。

## 返回值

实体有效时返回 1，否则返回 0。


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
    using definition_category = givm::character_view;
    struct definition_type {};
    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::program_entry<givm::character_initialization> handle(
        const definition_type&, const givm::character_view&,
        givm::character_initialization& event, const givm::table&, givm::random_fn&)
    {
        event.state = { .max_health = 10, .health = 10 };
        return givm::program_entry<givm::character_initialization>::null();
    }
};

int main()
{
    example_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } }, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{});
    const auto definition = ids.get_id<givm::character_view>("示例");
    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { definition } });

    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(library, table, random);
    const givm::character_view view = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    std::println("单实体范围大小: {}", view.size());
}
```

输出

```text
单实体范围大小: 1
```
