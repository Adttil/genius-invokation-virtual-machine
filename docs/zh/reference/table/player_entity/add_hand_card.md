[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **add_hand_card**

# givm::player_entity::add_hand_card

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr hand_card_entity<TStorage> add_hand_card(
    definition_id<card_definition> definition_id, const card_state& state
) const requires is_mutable;

constexpr hand_card_entity<TStorage> add_hand_card(card_data data) const
    requires is_mutable;
```

将一张新卡牌或已经取出的卡牌加入手牌。

指定定义与状态的重载创建新卡牌；接收 [`card_data`](../card_data.md) 的重载可接续卡牌原有的附带状态。

## 参数

|  |  |
| --- | --- |
| [`definition_id`](../../definition/definition_id.md) | 配套定义库中的卡牌定义 ID |
| `state` | 新卡牌的初始状态 |
| `data` | 从同一张牌桌取出的 [`card_data`](../card_data.md) |

## 返回值

新加入手牌的 [`hand_card_entity`](../hand_card_entity.md)。

## 注意

该操作不检查手牌上限，也不广播创建或抽牌事件。卡牌搬运的约定见[实体的身份与访问](../entity_access.md)。

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
    player.add_hand_card(definition, {});
    std::println("手牌张数: {}", player.hand_card_count());
}
```

输出

```text
手牌张数: 1
```
