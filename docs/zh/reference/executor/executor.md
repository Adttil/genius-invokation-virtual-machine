[givm](../../reference.md) / [执行](../executor.md) / **executor**

# givm::executor

定义于头文件 `<givm/executor.hpp>`

```cpp
class executor;
```

游戏对局的执行器。

它是所有牌桌（[`card_table`](../table/card_table.md)）以外状态的记录者，比如对局进行到了哪个阶段、结算进行到了哪一步、正在发生的事件等。

## 成员函数

| | |
| --- | --- |
| [`(构造函数)`](executor/constructor.md) | 构造一个`executor` |
| [`enter_entry`](executor/enter_entry.md) | 开始执行一场游戏 |
| [`position`](executor/position.md) | 取得当前执行位置 |
| [`execute_next`](executor/execute_next.md) | 执行当前位置的指令 |
| [`status`](executor/status.md) | 取得对局结果 |
| [`stack`](executor/stack.md) | 访问执行栈 |
| [`clear`](executor/clear.md) | 清空执行栈 |

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    // 编译一个最大回合数为 2 的定义库
    const auto [library, id_map] = sources.compile(
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 2 } }
    );
    givm::card_table table{ library };
    auto random = []() -> std::uint32_t { return 0; };

    givm::executor execution{};

    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}

    std::println("总回合数: {}", table.state().round_number);
    std::println("是否双败: {}", execution.status() == givm::game_result::both_loss);
}
```

输出

```text
总回合数: 2
是否双败: true
```

## 参阅

| | |
| --- | --- |
| [`card_table`](../table/card_table.md) | 保存牌桌状态 |
| [`definition_library`](../definition/definition_library.md) | 提供实体定义和规则程序 |
