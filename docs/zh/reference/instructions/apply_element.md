[givm](../../reference.md) / [指令](../instructions.md) / **apply_element**

# givm::apply_element

定义于头文件 `<givm/executor/instructions/apply_element.hpp>`

```cpp
struct apply_element;
```

向一个角色施加元素附着。没有反应时更新角色附着；发生反应时提供反应前后两次响应时机。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，可用于不限定事件的操作 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`element_application_source_id`](../events/element_application_source_id.md) | 施加元素的来源 |
| `target` | [`character_id`](../table/character_id.md) | 受到附着的有效角色 |
| `element` | [`element`](../enums/element.md) | 施加的元素 |
| `cause` | [`element_application_cause`](../enums/element_application_cause.md) | 附着来源的类别，初始为 effect |

## 成员函数

| | |
| --- | --- |
| [`execute`](apply_element/execute.md) | 向一个角色施加元素附着 |

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
        std::tuple{ givm::apply_element{ .source = givm::character_id{ .player_id = givm::player_id{ 0 }, .index = 0 }, .target = { .player_id = givm::player_id{ 1 }, .index = 0 }, .element = givm::element::hydro } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto definition = ids.get_id<givm::character_view>("character");
    const auto attacker = table[givm::player_id{ 0 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    const auto target = table[givm::player_id{ 1 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
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
| [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md) | 元素反应前 |
| [`after_elemental_reaction`](../events/after_elemental_reaction.md) | 元素反应后 |
