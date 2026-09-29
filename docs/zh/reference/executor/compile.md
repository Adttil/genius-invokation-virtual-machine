[givm](../../reference.md) / [执行](../executor.md) / **compile**

# givm::compile

定义于头文件 `<givm/executor.hpp>`

```cpp
std::expected<definition_compile_result, std::vector<compile_error>> compile(
    const definition_source_library& sources,
    const basic_definition_sources& basics,
    std::span<const any_command> initialization_program,
    std::span<const any_command> round_program,
    compile_mode mode
); // (1)

std::expected<definition_compile_result, std::vector<compile_error>> compile(
    const definition_source_library& sources,
    const basic_definition_sources& basics,
    const definition_selection& selection,
    std::span<const any_command> initialization_program,
    std::span<const any_command> round_program,
    compile_mode mode
); // (2)

template<class TInitializationSequence, class TRoundSequence>
    requires /* 两者均为命令序列，且至少一个不能隐式转换为 span<const any_command> */
auto compile(
    const definition_source_library& sources,
    const basic_definition_sources& basics,
    TInitializationSequence&& initialization_program,
    TRoundSequence&& round_program,
    compile_mode mode
); // (3)

template<class TInitializationSequence, class TRoundSequence>
    requires /* 两者均为命令序列，且至少一个不能隐式转换为 span<const any_command> */
auto compile(
    const definition_source_library& sources,
    const basic_definition_sources& basics,
    const definition_selection& selection,
    TInitializationSequence&& initialization_program,
    TRoundSequence&& round_program,
    compile_mode mode
); // (4)
```

准备一场对局要使用的实体定义和对局流程。初始化部分只进行一次，随后自动推进回合并反复执行回合部分，直到流程主动暂停或结束对局。

(1)、(3) 使用全部已登记定义及 `basics` 中的四个默认反应定义。(2)、(4) 从指定定义和 `basics` 中的四个默认反应定义出发，自动包含直接和间接按名称依赖的定义。其余定义不会编译。所有重载均不修改源库，编译期间的元数据查找和标签筛选也不会扩充这个集合。

(1)、(2) 接收 [`any_command`](../definition/any_command.md) 的连续序列。(3)、(4) 接收 tuple-like 对象或范围，并提供相同的编译行为。

模板重载要求两段序列均为 tuple-like 对象或输入范围，且至少一段不能隐式转换为 `span<const any_command>`。均可转换时直接使用非模板重载。

## 模板参数

|  |  |
| --- | --- |
| `TInitializationSequence` | 初始化命令序列，可为 tuple-like 对象或可遍历范围 |
| `TRoundSequence` | 每回合的命令序列，可为 tuple-like 对象或可遍历范围 |

## 参数

|  |  |
| --- | --- |
| `sources` | 已登记本场可用定义的源库 |
| `basics` | [`basic_definition_sources`](../definition/basic_definition_sources.md)，本场规则采用的四个默认反应源 |
| `selection` | 各类别首先选择的定义名称 |
| `initialization_program` | 对局开始时依次执行的命令 |
| `round_program` | 每回合依次执行的命令 |
| `mode` | [`compile_mode`](compile_mode.md)，决定是否提供额外观察现场 |

## 返回值

返回 `std::expected<definition_compile_result, std::vector<compile_error>>`。成功值为 [`definition_compile_result`](definition_compile_result.md)，其 `library` 和 `id_map` 对应同一个定义集合和 ID 分配结果。先检查返回的 `expected`，成功后可通过 `result->library`、`result->id_map` 访问，或以 `auto [library, id_map] = std::move(*result);` 取得两者。

失败时返回 [`compile_error`](compile_error.md) 列表，不发布部分编译的定义库。诊断包含发生阶段、定义源、程序与命令位置，以及具体错误类型；可交给 [`error_string`](error_string.md) 输出。能继续检查的错误会聚合，不通过验证异常中断。

某个定义源的 `compile` 产生诊断后，不再对该定义执行无参数的静态查询，以免继续使用未能成功编译的结果；其他定义仍继续编译和收集诊断。

## 自动回合推进

初始化流程完整结束后，进入第一个回合；每次回合流程完整结束后，进入下一回合。每次进入都按以下顺序处理：

1. 增加 `table.state().round_number`。
2. 观察模式先返回 `execution_state::round_started`；再次推进后继续以下步骤。
3. 若回合数超过 [`table_state::max_rounds`](../table/table_state.md)，以 `both_loss` 结束，不清空骰子，也不执行本回合程序。
4. 未超限则清空双方骰子，再从 `round_program` 的第一个命令开始执行。

回合数超限时通常为 `max_rounds + 1`。配置 `max_rounds = 0` 时，仍完整执行初始化，但不进入任何回合程序。初始化或回合流程中的命令若已结束对局，不再继续推进。响应产生的普通子程序执行完毕只回到原流程，不增加回合数。

回合程序应显式安排阶段顺序，例如 `start_dice_roll_phase{}`、`start_round{}`、`start_battle{}`、`begin_action{}`、`end_round{}`。其中 [`start_round`](../definition/commands/start_round.md) 只广播回合开始规则事件，应位于投骰和重投之后；它与上述自动回合推进是不同操作。

## 异常

|  |  |
| --- | --- |
| 定义源或用户范围抛出的异常 | 原样向调用方传播，不转换为验证诊断 |

## 注意

基础定义不必事先登记到 `sources`，它们及其依赖参与本次编译的选择与 ID 分配。需要在编译前取得 ID 时，调用 [`sources.make_issued_id_map(basics, ...)`](../definition/definition_source_library/make_issued_id_map.md)，并与本次编译使用相同的源库内容、基础定义配置及选择范围。

两段流程只能使用[核心给定的命令](../definition/commands.md)，也可用 [`any_command`](../definition/any_command.md) 保存。两段流程中的命令均不得消费响应输入；支持两种方式的命令必须提供固定参数。此限制在所有构建模式下检查，错误通过返回值报告。空回合流程也会自动推进回合，直至超过牌桌配置的上限而结束。

`mode` 必须显式指定。两种模式返回相同的 `definition_library` 类型，并通过同一个 视图的提交或 `resume` 推进；普通模式仍保留输入请求与终局，观察模式额外报告领域观察现场。模式同时应用于初始化、回合流程和定义源登记的所有响应程序。

编译返回后，初始化和回合流程的输入序列及其中的命令对象可以销毁，不影响定义库的使用。定义源编译所得配置数据随定义库保持有效；源名称、标签以及配置数据借用的对象仍须由调用方保证存活。

## 示例

```cpp
#include <utility>
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 2 } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
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
| [`executor::start`](executor/start.md) | 开始执行一场游戏 |
