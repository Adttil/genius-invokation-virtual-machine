[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **end_round**

# givm::end_round

定义于头文件 `<givm/executor/instructions/end_round.hpp>`

```cpp
struct end_round;
```

回合结束指令，负责本回合的收尾与下一回合的先手准备。它应放在双方的结束声明已经结算完毕之后。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 注意

将行动玩家从最后宣布结束的一方切换为另一方，清除已有人宣布结束的标记，然后发出 [`round_ended`](../events/round_ended.md)。回合结束抽牌等其他效果可在本指令之后另行安排。

以 [`step`](../executor/step.md) 推进时，执行上述操作前先返回 `execution_state::round_ending`。此时 `active_player` 仍是最后宣布结束的一方，结束声明标记尚未清除。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::end_round{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{};
    table.state().active_player = givm::player_id{ 1 };
    table.state().first_ended = true;
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(library, table, random);
    std::println("下一回合由玩家 0 先手: {}", table.state().active_player == givm::player_id{ 0 });
    std::println("结束声明标记已清除: {}", !table.state().first_ended);
}
```

输出

```text
下一回合由玩家 0 先手: true
结束声明标记已清除: true
```

## 参阅

| | |
| --- | --- |
| [`round_ended`](../events/round_ended.md) | 本回合结束的通知 |
