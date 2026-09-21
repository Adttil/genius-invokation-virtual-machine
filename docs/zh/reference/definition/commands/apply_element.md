[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **apply_element**

# givm::apply_element

定义于头文件 `<givm/definition.hpp>`

```cpp
struct apply_element;
```

元素附着命令。没有反应时更新角色附着；发生反应时提供反应前后两次响应时机。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`element_application_source_id`](../events/element_application_source_id.md) | 施加元素的来源 |
| `target` | [`character_id`](../../table/character_id.md) | 受到附着的有效角色 |
| `element` | [`element`](../../enums/element.md) | 施加的元素 |
| `cause` | [`element_application_cause`](../../enums/element_application_cause.md) | 附着来源的类别，初始为 effect |

## 注意

反应时先发出 [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md)，随后完成默认或由响应接管的反应处理。

超导、感电会对目标以外的同方存活角色分别造成 1 点穿透伤害；扩散会分别造成 1 点所扩散元素的伤害。这些伤害及其继续引发的反应先完成扣血与元素附着，然后才进行 [`after_elemental_reaction`](../events/after_elemental_reaction.md) 和派生伤害的完成通知。独立附着没有主目标伤害，不额外对原目标扣除反应加伤。

派生伤害同样在每次扣血后立即处理击倒并判定终局；若对局已结束，剩余伤害及完成通知不再进行。

生成反应实体等其他默认反应后果尚未全部实现。

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
