[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **start_round**

# givm::start_round

定义于头文件 `<givm/executor/instructions/start_round.hpp>`

```cpp
struct start_round;
```

新回合的开始指令，负责回合数更新和双方元素骰的清空。超过回合上限时，结束对局并判定双方失败。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `max_rounds` | `std::uint32_t` | 允许进行的最大回合数，初始为 14 |

## 成员函数

| | |
| --- | --- |
| [`execute`](start_round/execute.md) | 开始下一回合，更新回合数并清空双方的元素骰 |

## 注意

如果下一回合超过上限，本次不增加回合数，也不清空骰子。

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
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 2 } });
    givm::card_table table{ library };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("回合数: {}", table.state().round_number);
    std::println("达到上限后双败: {}", execution.status() == givm::game_result::both_loss);
}
```

输出

```text
回合数: 2
达到上限后双败: true
```
