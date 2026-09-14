[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **id**

# givm::definition_library::definition_view::id

定义于头文件 `<givm/executor.hpp>`

```cpp
definition_id<TDefinitionType> id() const;
```

取得该定义的 ID，可用来指定之后创建的同类实体采用相同定义。

## 返回值

本视图对应的定义 ID。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    const auto definition = library[card];
    givm::linked_deck deck{ .cards = { definition.id(), definition.id() } };
    std::println("采用该定义的卡牌数量: {}", deck.cards.size());
}
```

输出

```text
采用该定义的卡牌数量: 2
```
