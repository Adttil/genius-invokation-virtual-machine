[givm](../../reference.md) / [执行](../executor.md) / **compile**

# givm::compile

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TInitializationSequence, class TRoundSequence>
auto compile(
    const definition_source_library& sources,
    TInitializationSequence&& initialization_program,
    TRoundSequence&& round_program,
    compile_mode mode
); // (1)

template<class TInitializationSequence, class TRoundSequence>
auto compile(
    const definition_source_library& sources,
    const definition_selection& selection,
    TInitializationSequence&& initialization_program,
    TRoundSequence&& round_program,
    compile_mode mode
); // (2)
```

准备一场对局要使用的实体定义和对局流程。初始化部分只进行一次，随后自动推进回合并反复执行回合部分，直到流程主动暂停或结束对局。

(1) 使用全部已登记定义。(2) 从指定定义和源库构造时选定的四个默认反应定义出发，自动包含直接和间接依赖的定义。其余定义不会编译。

## 模板参数

|  |  |
| --- | --- |
| `TInitializationSequence` | 初始化命令序列，可为 tuple-like 对象或可遍历范围 |
| `TRoundSequence` | 每回合的命令序列，可为 tuple-like 对象或可遍历范围 |

## 参数

|  |  |
| --- | --- |
| `sources` | 已登记本场可用定义的源库 |
| `selection` | 各类别首先选择的定义名称 |
| `initialization_program` | 对局开始时依次执行的命令 |
| `round_program` | 每回合依次执行的命令 |
| `mode` | [`compile_mode`](compile_mode.md)，决定是否提供额外观察现场 |

## 返回值

返回一个类型未指定的对象，包含以下公开成员，按表中顺序支持结构化绑定：

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `library` | [`definition_library`](definition_library.md) | 编译后的实体定义与对局流程 |
| `id_map` | [`issued_id_map`](../definition/issued_id_map.md) | 同一次编译产生的定义及标签名称映射 |

两者对应同一个定义集合和 ID 分配结果。调用方可以通过 `auto` 保存结果，或使用 `auto [library, id_map] = compile(...);` 分别取得两者。

## 自动回合推进

初始化流程完整结束后，进入第一个回合；每次回合流程完整结束后，进入下一回合。每次进入都按以下顺序处理：

1. 增加 `table.state().round_number`。
2. 观察模式先返回 `execution_state::round_started`；再次推进后继续以下步骤。
3. 若回合数超过 [`game_parameters::max_rounds`](../table/game_parameters.md)，以 `both_loss` 结束，不清空骰子，也不执行本回合程序。
4. 未超限则清空双方骰子，再从 `round_program` 的第一个命令开始执行。

回合数超限时通常为 `max_rounds + 1`。配置 `max_rounds = 0` 时，仍完整执行初始化，但不进入任何回合程序。初始化或回合流程中的命令若已结束对局，不再继续推进。响应产生的普通子程序执行完毕只回到原流程，不增加回合数。

回合程序应显式安排阶段顺序，例如 `start_dice_roll_phase{}`、`start_round{}`、`start_battle{}`、`begin_action{}`、`end_round{}`。其中 [`start_round`](../definition/commands/start_round.md) 只广播回合开始规则事件，应位于投骰和重投之后；它与上述自动回合推进是不同操作。

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 选择了未知定义，定义源编译时查询了未声明的依赖，或动态定义源声明支持某项响应或查询却缺少对应实现；未定义 `NDEBUG` 时，初始化或回合流程需要响应输入也会抛出 |

## 注意

两段流程只能使用[核心给定的命令](../definition/commands.md)，也可用 [`any_command`](../definition/any_command.md) 保存。两段流程中的命令均不得消费响应输入；支持两种方式的命令必须提供固定参数。空回合流程也会自动推进回合，直至超过牌桌配置的上限而结束。定义源的编译操作抛出的异常继续向调用者传播。

`mode` 必须显式指定。两种模式返回相同的 `definition_library` 类型，并通过同一个 `executor::step` 推进；普通模式仍保留输入请求与终局，观察模式额外报告领域观察现场。模式同时应用于初始化、回合流程和定义源登记的所有响应程序。

编译返回后，初始化和回合流程的输入序列及其中的命令对象可以销毁，不影响定义库的使用。定义源编译所得配置数据随定义库保持有效；源名称、标签以及配置数据借用的对象仍须由调用方保证存活。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal
    );
    givm::table table{ { .max_rounds = 2 } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("终局时的回合数: {}", table.state().round_number);
    std::println("以双败结束: {}", execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}
```

输出

```text
终局时的回合数: 3
以双败结束: true
```

## 参阅

|  |  |
| --- | --- |
| [`definition_source_library::make_issued_id_map`](../definition/definition_source_library/make_issued_id_map.md) | 在编译前取得配套 ID |
| [`executor::enter_entry`](executor/enter_entry.md) | 开始执行一场游戏 |
