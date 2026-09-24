[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_energy**

# givm::set_energy

定义于头文件 `<givm/definition.hpp>`

```cpp
struct set_energy;
```

角色充能的赋值命令，可用于清空充能或将其设置到指定数值。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定目标位置；默认采用动态输入 |
| `value` | `std::uint32_t` | 固定模式下要设置的充能值，默认零 |

## 注意

默认构造 `set_energy{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`energy_change`](../events/energy_change.md)。显式指定 `target` 时采用固定模式，不消费响应输入。

固定模式在命令执行时按相对位置定位角色，`target.selection` 必须为 `character_selection::character`；没有有效目标时跳过命令。允许目标为已战败但未离场的角色，不会因为角色战败而顺延到其他角色。动态输入必须指定实际存在的有效角色。

最终写入的 `energy` 为 `min(value, max_energy)`，上限取自命令执行时的角色状态。命令不改变 `energy_tag`，操作普通充能还是替代充能由定义自行决定。

命令只修改牌桌，不发送 [`changing_energy`](../events/changing_energy.md) 或 [`energy_changed`](../events/energy_changed.md)，也不产生专门的观察现场。技能使用不会自动增加充能，需要在技能效果程序中显式安排本命令或 [`modify_energy`](modify_energy.md)。

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
    std::string_view name() const { return "充能示例角色"; }
    int compile(givm::definition_compile_context&) const { return 0; }

    static givm::character_state query(const int&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .max_energy = 3, .health = 10 };
    }
};

int main()
{
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    const character_source source{};
    sources.add(source);
    const auto [library, ids] = compile(sources,
        std::tuple{
            givm::select_active_character_both{},
            givm::set_energy{ .target = {}, .value = 10 },
            givm::modify_energy{ .target = {}, .delta = -2 },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);

    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::character_view>("充能示例角色");
    const givm::linked_deck deck{ .characters = { definition } };
    load_deck(table, library, deck, deck);
    const givm::character_id target{ givm::player_id{ 0 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(target);
    execution.step(library, table, random);
    execution.view_in<givm::execution_state::remaining_active_character_selection>().select(
        givm::character_id{ givm::player_id{ 1 }, 0 });
    execution.step(library, table, random);
    std::println("充能上限: {}", table[target].state().max_energy);
    std::println("先设为 10 再减少 2: {}", table[target].state().energy);
}
```

输出

```text
充能上限: 3
先设为 10 再减少 2: 1
```

## 参阅

| | |
| --- | --- |
| [`energy_change`](../events/energy_change.md) | 充能赋值的动态输入 |
| [`modify_energy`](modify_energy.md) | 按增量修改充能的命令 |
