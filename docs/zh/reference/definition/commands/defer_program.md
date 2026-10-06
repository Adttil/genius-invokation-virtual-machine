[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **defer_program**

# givm::defer_program

定义于头文件 `<givm/definition.hpp>`

```cpp
struct defer_program
{
    using error_type = defer_program_error;
    using input_type = defer_program_input;

    fixed_defer_program_input input{};
};
```

登记一段稍后执行的程序，使它在当前所属域的下一次结算中按登记顺序执行。结算可由显式 `settle` 发起，也可由非内联调用者在程序返回后发起。

## 成员类型

| | |
| --- | --- |
| `error_type` | 本命令的编译检查错误 `defer_program_error` |
| `input_type` | 目标入口及其参数的输入 [`defer_program_input`](../command_inputs/defer_program_input.md) |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `input` | [`fixed_defer_program_input`](../fixed_defer_program_input.md) | 固定目标入口及参数；其中入口为空时由动态输入整体指定 |

## 注意

`defer_program{}` 消费一个动态输入，通过 [`defer_invoke`](../defer_invoke.md) 同时提供目标入口和参数，例如 `context.invoke(parent, defer_invoke(child, input_a, input_b))`。

固定形式为 `defer_program{fixed_defer_invoke(child, input_a, input_b)}`。它不消费父程序的输入；编译时检查参数的数量、类型、顺序和嵌套关系，再复制目标入口和全部参数字节，之后不再依赖命令对象。不需要参数时使用 `fixed_defer_invoke(child)`。固定和动态形式均整体指定入口与参数。

目标程序与普通响应使用同一个 [`add_normal_effect`](../../executor/definition_compile_context/add_effect.md)、入口和命令输入协议。没有延迟专用的程序类型。执行目标程序时完成其末段及派生结算，然后继续其他后续工作；其返回编号被忽略。

延迟保存的是已选入口和已准备的参数，不会重新调用登记者的 `handle`。[`defer_invoke`](../defer_invoke.md) 先拥有参数和数组内容；动态提交在 [`invoke`](../../executor/handle_context/invoke.md) 时复制这份快照，固定形式则在编译时复制。目标执行时保留登记该命令时的本方归属。

普通响应返回后自动处理所登记的程序；即时响应向外层当前段登记，自己返回时不处理它们。根程序须显式使用 [`settle`](settle.md)。目标程序仍可继续登记其他延迟程序。

## 编译检查

`defer_program_error` 包含 `reason cause`，其中 `reason::dynamic_input_in_root` 表示初始化或回合根程序采用了动态形式。根程序仅能使用固定形式；目标程序本身可以需要参数，只要已经通过 `defer_invoke` 完整提供。

未定义 `NDEBUG` 时，还检查固定目标入口及参数，包括嵌套延迟参数。不匹配时返回 [`fixed_program_input_error`](../../executor/compile_error_reason.md)，而不是等到延迟程序执行时才报告。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>
#include <utility>

#include <givm/givm.hpp>

struct delayed_dice_source
{
    using definition_category = givm::card_definition;
    struct definition_type
    {
        givm::normal_effect first;
        givm::normal_effect delayed;
    };

    std::string_view name() const { return "延迟产骰"; }

    definition_type compile(givm::definition_compile_context& context) const
    {
        return {
            context.add_normal_effect(std::tuple{
                givm::defer_program{}, givm::return_response{ .index = 1 }
            }),
            context.add_normal_effect(std::tuple{
                givm::add_dice{}, givm::return_response{ .index = 7 }
            })
        };
    }

    static givm::normal_effect handle(
        const definition_type& data, givm::round_started&,
        givm::handle_context<givm::deck_card_view>& context,
        std::uint32_t response_index = 0)
    {
        if(response_index == 1)
        {
            std::println("再次响应时的骰子数: {}", context.entity().player().state().dice.total());
            return {};
        }
        givm::dice_counts dice{};
        dice[givm::elemental_dice::pyro] = 1;
        return context.invoke(data.first,
            givm::defer_invoke(data.delayed,
                givm::add_dice_input{ context.entity().player().id(), dice }));
    }
};

int main()
{
    const delayed_dice_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto result = compile(sources, basics, std::tuple{},
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        givm::compile_mode::normal);
    if(not result)
    {
        std::println("{}", error_string(result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*result);
    givm::table table{};
    load_deck(table, library,
        givm::linked_deck{ .cards = { ids.get_id<givm::card_definition>(source.name()) } }, {});
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    std::println("最终骰子数: {}", table[givm::player_id{ 0 }].state().dice.total());
}
```

输出

```text
再次响应时的骰子数: 1
最终骰子数: 1
```

延迟程序返回的 7 被忽略；父响应返回的 1 在延迟产骰完整结束后才用于下一次 `handle`。
