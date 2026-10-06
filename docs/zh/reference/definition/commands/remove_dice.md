[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_dice**

# givm::remove_dice

定义于头文件 `<givm/definition.hpp>`

```cpp
struct remove_dice_error;

struct remove_dice
{
    using error_type = remove_dice_error;

    using input_type = remove_dice_input;

    relative_player player = static_cast<relative_player>(-1);
    dice_counts dice{};
};
```

从一位玩家的骰池中扣除指定种类和数量的元素骰的命令。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`remove_dice_input`](../command_inputs/remove_dice_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `remove_dice_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 固定模式下失去元素骰的一方；默认采用动态输入 |
| `dice` | [`dice_counts`](../../enums/dice_counts.md) | 固定模式下本次扣除的各类元素骰数量 |

## 编译检查

```cpp
struct remove_dice_error;
```

`remove_dice::error_type` 是 `givm::remove_dice_error` 的别名。`remove_dice_error` 是本命令的结构化编译错误，`remove_dice_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |

### `remove_dice_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 注意

默认构造 `remove_dice{}` 使用动态模式，由响应通过 [`invoke`](../../executor/handle_context/invoke.md) 提交一个 [`remove_dice_input`](../command_inputs/remove_dice_input.md)，包含玩家 ID 和本次扣除的各类骰子数量。显式指定 `player` 为 `self` 或 `opponent` 时采用固定模式，不消费响应输入。本方的含义见 [`relative_player`](relative_player.md)；动态输入的玩家 ID 必须有效。

调用方必须保证命令执行时玩家每类骰子的数量都足够扣除。命令精确扣除指定数量，不自动减少扣除量；需要“至多扣除”的效果时，由响应根据骰池计算实际数量后提交动态输入。

一次性扣除全部指定骰子，然后全场广播一次 [`dice_removed`](../events/dice_removed.md)。事件记录本次扣除量；响应读取牌桌时可以看到扣除后的骰池。全部数量为零时无效果，也不广播。

## 示例

```cpp
#include <utility>
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::dice_counts initial_dice{};
    initial_dice[givm::elemental_dice::pyro] = 3;
    initial_dice[givm::elemental_dice::omni] = 2;
    givm::dice_counts collected_dice{};
    collected_dice[givm::elemental_dice::pyro] = 1;
    collected_dice[givm::elemental_dice::omni] = 1;
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    auto library_result = compile(sources, basics,
        std::tuple{
            givm::add_dice{ .player = givm::relative_player::self, .dice = initial_dice },
            givm::remove_dice{ .player = givm::relative_player::self, .dice = collected_dice },
            givm::settle{}, givm::end_game{ .result = givm::game_result::both_loss }
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
    const auto& remaining = table[givm::player_id{ 1 }].state().dice;
    std::println("剩余火元素骰: {}", remaining[givm::elemental_dice::pyro]);
    std::println("剩余万能元素骰: {}", remaining[givm::elemental_dice::omni]);
}
```

输出

```text
剩余火元素骰: 2
剩余万能元素骰: 1
```

## 参阅

| | |
| --- | --- |
| [`dice_removed`](../events/dice_removed.md) | 元素骰移除或支付后的通知 |
| [`add_dice`](add_dice.md) | 增加指定元素骰的命令 |
