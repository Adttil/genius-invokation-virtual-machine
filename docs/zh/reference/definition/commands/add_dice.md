[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_dice**

# givm::add_dice

定义于头文件 `<givm/definition.hpp>`

```cpp
struct add_dice_error;

struct add_dice
{
    using error_type = add_dice_error;

    using input_type = add_dice_input;

    relative_player player = static_cast<relative_player>(-1);
    dice_counts dice{};
};
```

向一位玩家增加指定种类和数量的元素骰的命令。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`add_dice_input`](../command_inputs/add_dice_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `add_dice_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 固定模式下获得元素骰的一方；默认采用动态输入 |
| `dice` | [`dice_counts`](../../enums/dice_counts.md) | 固定模式下本次增加的各类元素骰数量 |

## 编译检查

```cpp
struct add_dice_error;
```

`add_dice::error_type` 是 `givm::add_dice_error` 的别名。`add_dice_error` 是本命令的结构化编译错误，`add_dice_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |

### `add_dice_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 注意

默认构造 `add_dice{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`add_dice_input`](../command_inputs/add_dice_input.md)，包含玩家 ID 和本次增加的各类骰子数量。显式指定 `player` 为 `self` 或 `opponent` 时采用固定模式，不消费响应输入。本方的含义见 [`relative_player`](relative_player.md)；动态输入的玩家 ID 必须有效。

一次性增加全部指定骰子，然后全场广播一次 `dice_added`。事件记录本次增加量；响应读取牌桌时可以看到增加后的骰子。全部数量为零时无效果，也不广播。

命令本身不调用随机源；若需要随机种类或数量，由响应计算后提交动态输入。投骰和元素调和不属于此命令的产骰，也不广播 `dice_added`。

调用方应确保每类骰子增加后的数量在 0 到 255 范围内，与 [`dice_counts::operator+=`](../../enums/dice_counts/operator_add_assign.md) 的要求一致。

## 示例

```cpp
#include <utility>
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::dice_counts self_dice{};
    self_dice[givm::elemental_dice::pyro] = 2;
    self_dice[givm::elemental_dice::omni] = 1;
    givm::dice_counts opponent_dice{};
    opponent_dice[givm::elemental_dice::hydro] = 1;
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0,
        givm::genshin_impact::shield_3_3_0
    };
    givm::definition_source_library sources{};
    auto library_result = compile(sources, basics,
        std::tuple{
            givm::add_dice{ .player = givm::relative_player::self, .dice = self_dice },
            givm::add_dice{ .player = givm::relative_player::opponent, .dice = opponent_dice },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);

    givm::table table{ { .self_player = givm::player_id{ 1 } } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    const auto& self = table[givm::player_id{ 1 }].state().dice;
    const auto& opponent = table[givm::player_id{ 0 }].state().dice;
    std::println("我方火元素骰: {}", self[givm::elemental_dice::pyro]);
    std::println("我方万能元素骰: {}", self[givm::elemental_dice::omni]);
    std::println("对方水元素骰: {}", opponent[givm::elemental_dice::hydro]);
}
```

输出

```text
我方火元素骰: 2
我方万能元素骰: 1
对方水元素骰: 1
```

## 参阅

| | |
| --- | --- |
| [`dice_added`](../events/dice_added.md) | 元素骰增加完成后的通知 |
| [`remove_dice`](remove_dice.md) | 扣除指定元素骰的命令 |
| [`dice_counts`](../../enums/dice_counts.md) | 各类元素骰的数量 |
