[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **apply_element**

# givm::apply_element

定义于头文件 `<givm/definition.hpp>`

```cpp
struct apply_element;
```

元素附着命令。没有反应时更新角色附着；发生反应时提供反应前后两次响应时机。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`element_application_source_id`](../events/element_application_source_id.md) | 施加元素的来源 |
| `target` | [`character_id`](../../table/character_id.md) | 受到附着的有效角色 |
| `element` | [`element`](../../enums/element.md) | 施加的元素 |
| `cause` | [`element_application_cause`](../../enums/element_application_cause.md) | 附着来源的类别，初始为 effect |

## 注意

反应时先发出 [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md)，随后完成默认或由响应接管的附着处理，最后发出 [`after_elemental_reaction`](../events/after_elemental_reaction.md)。

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

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
    }
};

int main()
{
    character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::apply_element{ .source = givm::character_id{ .player_id = givm::player_id{ 0 }, .index = 0 }, .target = { .player_id = givm::player_id{ 1 }, .index = 0 }, .element = givm::element::hydro } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("目标附着水元素: {}", table[target].state().aura == givm::element_aura::hydro);
}
```

输出

```text
目标附着水元素: true
```

## 参阅

| | |
| --- | --- |
| [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md) | 元素反应生效前的事件 |
| [`after_elemental_reaction`](../events/after_elemental_reaction.md) | 元素反应处理完成后的通知 |
