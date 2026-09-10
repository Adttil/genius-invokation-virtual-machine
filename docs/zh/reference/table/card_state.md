[givm](../../reference.md) / [牌桌](../table.md) / **card_state**

# givm::card_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct card_state {};
```

一张卡牌自身的对局状态。

该类型不含成员对象；卡牌携带的效果由附着在卡牌上的状态表达。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

template<class Category>
struct example_source
{
    using definition_category = Category;
    struct definition_type {};

    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    givm::definition_source_library sources{};
    const example_source<givm::card_definition> card_source{};
    sources.add(card_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    givm::card_state initial{};
    const auto card = player.add_hand_card(definition, initial);
    std::println("手牌创建成功: {}", card.is_valid());
}
```

输出

```text
手牌创建成功: true
```
