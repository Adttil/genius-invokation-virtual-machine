[givm](../../reference.md) / [指令](../instructions.md) / **end_round**

# givm::end_round

定义于头文件 `<givm/executor/instructions/end_round.hpp>`

```cpp
struct end_round;
```

关闭本回合并准备下一回合的先手。它应放在双方的结束声明已经结算完毕之后。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，可用于不限定事件的操作 |

## 成员函数

| | |
| --- | --- |
| [`execute`](end_round/execute.md) | 关闭本回合并准备下一回合的先手 |

## 注意

将行动玩家从最后宣布结束的一方切换为另一方，清除已有人宣布结束的标记，然后发出 [`round_ended`](../events/round_ended.md)。回合结束抽牌等其他效果可在本指令之后另行安排。

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
    givm::card_table table{ library };
    table.state().active_player = givm::player_id{ 1 };
    table.state().first_ended = true;
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
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
| [`round_ended`](../events/round_ended.md) | 本回合已关闭 |
