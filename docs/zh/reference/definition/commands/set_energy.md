[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_energy**

# givm::set_energy

定义于头文件 `<givm/definition.hpp>`

```cpp
struct set_energy;
```

角色充能的赋值命令，可用于清空充能或将其设置到指定数值。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`set_energy_input`](../command_inputs/set_energy_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `set_energy_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定目标位置；默认采用动态输入 |
| `value` | `std::uint32_t` | 固定模式下要设置的充能值，默认零 |

## 编译检查

```cpp
struct set_energy_error;
```

`set_energy::error_type` 是 `givm::set_energy_error` 的别名。`set_energy_error` 是本命令的结构化编译错误，`set_energy_error::reason` 是原因枚举。[编译检查 `check`](../../executor/check.md) 使用本次定义集合与程序种类检查以下条件；[`compile`](../../executor/compile.md) 自动收集这些错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_target_player` | `target.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_target_selection` | `target.selection` 不是 `character_selection::character`；此处只允许单个角色 |

### `set_energy_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 注意

默认构造 `set_energy{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`set_energy_input`](../command_inputs/set_energy_input.md)。显式指定 `target` 时采用固定模式，不消费响应输入。

固定模式在命令执行时按相对位置定位角色，`target.selection` 必须为 `character_selection::character`；没有有效目标时跳过命令。允许目标为已战败但未离场的角色，不会因为角色战败而顺延到其他角色。动态输入必须指定实际存在的有效角色。

最终写入的 `energy` 为 `min(value, max_energy)`，上限取自命令执行时的角色状态。命令不改变 `energy_tag`，操作普通充能还是替代充能由定义自行决定。

命令只修改牌桌，不发送 [`changing_energy`](../events/changing_energy.md) 或 [`energy_changed`](../events/energy_changed.md)，也不产生专门的观察现场。技能使用不会自动增加充能，需要在技能效果程序中显式安排本命令或 [`modify_energy`](modify_energy.md)。

## 示例

```cpp
#include <utility>
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
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    const character_source source{};
    if(not sources.add(source)) return 1;
    auto library_result = compile(sources, basics,
        std::tuple{
            givm::select_active_character_both{},
            givm::set_energy{ .target = {}, .value = 10 },
            givm::modify_energy{ .target = {}, .delta = -2 },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);

    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::character_view>("充能示例角色");
    const givm::linked_deck deck{ .characters = { definition } };
    load_deck(table, library, deck, deck);
    const givm::character_id target{ givm::player_id{ 0 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.start(library, table);
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
| [`set_energy_input`](../command_inputs/set_energy_input.md) | 充能赋值的动态输入 |
| [`modify_energy`](modify_energy.md) | 按增量修改充能的命令 |
