[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_active_character**

# givm::set_active_character

定义于头文件 `<givm/definition.hpp>`

```cpp
struct set_active_character;
```

直接设置出战角色的命令。它用于开局设置或规则强制的切换，不经过主动切换的费用计算。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | [`character_id`](../../table/character_id.md) | 固定目标；默认采用动态输入 |

## 输入

通过构造命令选择目标的提供方式：

- 默认构造 `set_active_character{}` 使用动态输入，消费响应通过 `invoke` 提交的一个 [`active_character_changed`](../events/active_character_changed.md)，以其中的 `current` 为目标。
- 显式指定 `target` 时，使用该固定目标，不消费响应输入。

两种方式的目标都必须是牌桌上的有效角色。命令不读取外层响应事件或响应者；动态目标的提交示例见 [`handle_context::invoke`](../../executor/handle_context/invoke.md#示例)。

## 注意

执行时，若该玩家当前出战角色具有 `control_immunity` 附属，则忽略此次设置，不产生变更通知或观察现场。没有出战角色时仍可设置初始角色。此限制针对本命令；玩家在行动选择中[主动切换](../../executor/execution_view/action_selection/switch_active_character.md)不受免控保护限制。

设置后发出 [`active_character_changed`](../events/active_character_changed.md)，响应者能够读取新的出战角色。

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，写入新出战角色前先返回 `execution_state::active_character_changed`。相应[视图](../../executor/execution_view/active_character_changed.md)提供目标角色，牌桌上仍保留原出战角色；随后推进才写入目标并处理变更响应。未被免控阻止且目标本就是该方的出战角色时，不产生此观察现场，规则事件仍照常处理。

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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 1 }, 0 } }, givm::set_active_character{ .target = { .player_id = givm::player_id{ 1 }, .index = 1 } } },
        std::tuple{}, givm::compile_mode::observed);
    givm::table table{ { .max_rounds = 0 } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition, definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id original{ givm::player_id{ 1 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 1 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    execution.step(library, table, random);
    const auto view = execution.view_in<givm::execution_state::active_character_changed>();
    std::println("本次将设置为目标角色: {}", view.character() == target);
    std::println("牌桌仍保留原出战角色: {}", table[view.character().player_id].state().active_character == original);
    execution.step(library, table, random);
    std::println("出战角色设置成功: {}", table[givm::player_id{ 1 }].state().active_character == target);
}
```

输出

```text
本次将设置为目标角色: true
牌桌仍保留原出战角色: true
出战角色设置成功: true
```

## 参阅

| | |
| --- | --- |
| [`active_character_changed`](../events/active_character_changed.md) | 出战角色设置完成后的通知 |
