[givm](../../reference.md) / [指令](../instructions.md) / **begin_action**

# givm::begin_action

定义于头文件 `<givm/executor/instructions/begin_action.hpp>`

```cpp
struct begin_action;
```

开始本回合的行动阶段，让玩家依次选择行动，直到双方都宣布结束。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，可用于不限定事件的操作 |

## 成员函数

| | |
| --- | --- |
| [`execute`](begin_action/execute.md) | 开始本回合的行动阶段，让玩家依次选择行动，直到双方都宣布结束 |

## 注意

先发出 [`action_phase_started`](../events/action_phase_started.md)，每次选择行动前发出 [`before_action`](../events/before_action.md)。支持主动切换出战角色和宣布结束；当前行动方必须已有出战角色。主动切换可经过费用计算、支付、出战角色变更及行动权交接。双方均宣布结束后，本指令才结束行动阶段。

等待行动输入时，按 `top<action_argument, action_request, stage_t>()` 取得参数、请求和需保留的尾部状态。通过 [`action_request`](action_request.md) 指定行动，通过 [`action_argument`](action_argument.md) 提供要支付的骰子。宣布结束无需支付骰子；主动切换时，`action_index` 按当前存活非出战角色的遍历顺序选择目标。调用方负责保证提交的支付满足计算出的费用。

只请求计算切换费用时，执行器计算后再次等待输入，并清空请求。之后执行该行动需要重新填写请求，见 [`action_request_kind`](action_request_kind.md)。

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
        std::tuple{ givm::begin_action{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto definition = ids.get_id<givm::character_view>("character");
    const auto attacker = table[givm::player_id{ 0 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    const auto target = table[givm::player_id{ 1 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    table[givm::player_id{ 0 }].state().active_character = attacker;
    table[givm::player_id{ 1 }].state().active_character = target;
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    int declarations = 0;
    while(execution.status() == givm::game_result::no_result)
    {
        // 当前玩家宣布本回合结束。
        auto&& [request, preserved] = execution.stack().top<givm::action_request, givm::stage_t>();
        request = { .request_kind = givm::action_request_kind::do_action,
                    .action_kind = givm::action_kind::declare_round_end };
        ++declarations;
        while(execution.execute_next(table, random)) {}
    }
    std::println("双方结束声明次数: {}", declarations);
}
```

输出

```text
双方结束声明次数: 2
```

## 参阅

| | |
| --- | --- |
| [`action_phase_started`](../events/action_phase_started.md) | 行动阶段开始 |
| [`before_action`](../events/before_action.md) | 即将选择行动 |
| [`cost_of_switch`](../events/cost_of_switch.md) | 计算主动切换费用 |
| [`round_end_declared`](../events/round_end_declared.md) | 玩家宣布结束 |
