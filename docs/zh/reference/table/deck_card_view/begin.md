[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_view](../deck_card_view.md) / **begin**

# givm::deck_card_view::begin

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const deck_card_view* begin() const;
```

取得这个牌库卡牌的单实体范围起点。

## 返回值

指向该 [`deck_card_view`](../deck_card_view.md) 访问对象自身的只读指针。若实体无效，它等于 `end()`。

## 注意

范围依赖访问对象本身的存活。遍历有效实体时执行一次循环；遍历已移除实体时不执行循环。


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
