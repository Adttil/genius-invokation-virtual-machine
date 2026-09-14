[givm](../../../reference.md) / [牌桌](../../table.md) / [character_view](../character_view.md) / **operator bool**

# givm::character_view::operator bool

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr explicit operator bool() const;
```

判断这个角色是否仍然有效。

## 返回值

实体尚未移除时为 `true`，已经移除时为 `false`。

## 注意

访问对象本身必须仍可安全访问；本函数不能用来检查已经悬空的对象。


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
    if(view)
    {
        std::println("有效实体的定义: {}", library[view.definition_id()].name());
    }
}
```

输出

```text
有效实体的定义: 示例
```
