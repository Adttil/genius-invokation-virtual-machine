[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **set_element_aura**

# givm::set_element_aura

定义于头文件 `<givm/executor/instructions/set_element_aura.hpp>`

```cpp
struct set_element_aura;
```

直接设置角色元素附着的指令，适用于需要明确指定附着状态的效果。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | [`character_id`](../../table/character_id.md) | 要设置附着的有效角色 |
| `aura` | [`element_aura`](../../enums/element_aura.md) | 新的元素附着 |

## 注意

本指令只设置附着，不触发元素反应。需要施加元素并判断反应时，使用 [`apply_element`](apply_element.md)。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct character_source
{
    using definition_category = givm::character_view;
    struct definition_type {};
    std::string_view name() const { return "character"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::program_entry<givm::character_initialization> handle(
        const definition_type&, const givm::character_view&,
        givm::character_initialization& event, const givm::card_table&, givm::random_fn&)
    {
        event.state = { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
        return givm::program_entry<givm::character_initialization>::null();
    }
};

int main()
{
    character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } }, givm::initialize_characters{ .player = givm::player_id{ 1 } }, givm::set_element_aura{ .target = { .player_id = givm::player_id{ 1 }, .index = 0 }, .aura = givm::element_aura::hydro } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{};
    const auto definition = ids.get_id<givm::character_view>("character");
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { definition } });
    table.load_deck(givm::player_id{ 1 }, givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(library, table, random);
    std::println("目标附着水元素: {}", table[target].state().aura == givm::element_aura::hydro);
}
```

输出

```text
目标附着水元素: true
```
