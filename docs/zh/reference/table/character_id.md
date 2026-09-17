[givm](../../reference.md) / [牌桌](../table.md) / **character_id**

# givm::character_id

定义于头文件 `<givm/table.hpp>`

```cpp
struct character_id;
```

角色在一张牌桌中的身份。使用此 ID 可以通过 [`table::operator[]`](table/operator_subscript.md) 再次取得相应实体。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player_id` | [`player_id`](player_id.md) | 所属玩家的 ID |
| `index` | `size_t` | 角色区中从 0 起算的位置；不表示筛除已移除角色后的排列位置 |

## 非成员函数

```cpp
friend constexpr bool operator==(character_id, character_id) = default;
```

比较各成员是否相等；比较不检查实体是否尚未移除。

## 注意

通过 [`load_deck`](table/load_deck.md) 初次装载角色时，角色位置与 `linked_deck::characters` 中的下标对应。规则中新入场的角色应通过牌桌的角色视图取得 ID。

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。


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

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .health = 10 };
    }
};

int main()
{
    example_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } }, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::normal);
    const auto definition = ids.get_id<givm::character_view>("示例");
    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { definition } });

    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    const givm::character_view view = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    const givm::character_id id = view.id();
    std::println("通过 ID 取得定义: {}", library[table[id].definition_id()].name());
}
```

输出

```text
通过 ID 取得定义: 示例
```
