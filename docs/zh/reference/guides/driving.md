[givm](../../reference.md) / [使用指南](../guides.md) / **驱动与输入**

# 驱动与输入

外层程序在执行器需要输入或观察时接管控制，处理后再继续推进。这使同一套对局逻辑可以由交互界面、决策程序或测试代码驱动。

## 推进与暂停

调用 [`enter_entry`](../executor/executor/enter_entry.md) 准备开始，然后重复调用 [`execute_next`](../executor/executor/execute_next.md)。一次调用完整执行一次指令；同一个结算可能包含多次这样的调用。

返回 `false` 表示应停止自动推进。此时先检查 [`status`](../executor/executor/status.md)：已经终局时读取结果；尚未终局时，再识别当前指令，处理它约定的输入或观察。

将 [`position`](../executor/executor/position.md) 的结果交给 [`definition_library::instruction`](../definition/definition_library/instruction.md)，可以取得下一条要执行的指令。通过其 `is<T>()` 判断类型，必要时通过 `as<T>()` 读取该指令的公开参数。

## 提交输入

每条需要输入的指令说明输入何时可访问、输入对象类型以及可写字段。外层程序只应访问约定的输入槽后缀，并保留其他值。提交前应检查选择是否满足游戏规则。

下面的流程会暂停等待玩家 0 换牌。示例选择保留全部手牌，再继续到对局结束。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };

    execution.enter_entry(library);
    while(true)
    {
        while(execution.execute_next(table, random))
        {}
        if(execution.status() != givm::game_result::no_result) break;

        const auto instruction = library.instruction(execution.position());
        if(instruction.is<givm::replace_cards>())
        {
            // 仅访问换牌暂停时约定的后缀，保留尾部进度值。
            auto [input, stage] = execution.stack().top<givm::selector, givm::stage_t>();
            input.selected.reset();
            std::println("选择换掉的手牌数: {}", input.selected.count());
        }
        else
        {
            return 1;
        }
    }
    std::println("是否双方告负: {}", execution.status() == givm::game_result::both_loss);
}
```

输出

```text
选择换掉的手牌数: 0
是否双方告负: true
```

输入 view 只在对应暂停仍然有效时使用。继续执行后，应根据下一次暂停重新取得 view。其他输入类型参阅[具体指令](../instructions.md)。
