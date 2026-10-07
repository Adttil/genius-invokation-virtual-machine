[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **id**

# givm::definition_library::definition_view::id

定义于头文件 `<givm/runtime.hpp>`

```cpp
definition_id<TDefinitionType> id() const;
```

取得该定义的 ID，可用来指定之后创建的同类实体采用相同定义。

## 返回值

本视图对应的定义 ID。

## 示例

```cpp
#include <utility>
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    static constexpr auto category = givm::definition_category::card;

    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    const auto card = ids.get_id<givm::definition_category::card>("恢复药剂");
    const auto definition = library[card];
    givm::linked_deck deck{ .cards = { definition.id(), definition.id() } };
    std::println("采用该定义的卡牌数量: {}", deck.cards.size());
}
```

输出

```text
采用该定义的卡牌数量: 2
```
