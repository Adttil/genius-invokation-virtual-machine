[givm](../../reference.md) / [执行](../executor.md) / **executor**

# givm::executor

定义于头文件 `<givm/executor.hpp>`

```cpp
class executor;
```

规则程序的执行器，保存当前执行位置和继续执行所需的临时状态。

[`card_table`](../table/card_table.md) 保存牌桌状态。复制牌桌和执行器，并为各分支准备相应的随机源状态，可建立独立推进的模拟分支。

默认构造后，先调用 [`enter_entry`](executor/enter_entry.md) 准备从定义库的规则程序入口开始执行，再使用 [`execute_next`](executor/execute_next.md) 逐条执行指令。通过 [`position`](executor/position.md) 取得的执行位置，可从定义库中找到下一条要执行的指令；通过 [`status`](executor/status.md) 查询对局结果。

## 成员函数

| | |
| --- | --- |
| [(构造函数)](executor/constructor.md) | 创建执行器，或复制、移动执行现场 |
| [(析构函数)](executor/destructor.md) | 销毁执行器 |
| [`operator=`](executor/operator_assign.md) | 复制或移动赋值执行现场 |

### 执行

| | |
| --- | --- |
| [`enter_entry`](executor/enter_entry.md) | 建立从规则程序入口开始的执行现场 |
| [`execute_next`](executor/execute_next.md) | 完整执行当前位置的一次指令 |
| [`clear`](executor/clear.md) | 清空执行栈 |

### 状态访问

| | |
| --- | --- |
| [`position`](executor/position.md) | 取得当前执行位置 |
| [`status`](executor/status.md) | 取得对局结果 |
| [`stack`](executor/stack.md) | 访问执行栈 |

## 示例

```cpp
#include <givm/givm.hpp>

#include <cstdint>
#include <iostream>
#include <tuple>

int main()
{
    using namespace givm;

    definition_source_library sources{};
    // 初始化程序执行一次，回合程序反复执行。
    const auto [library, id_map] = sources.compile(
        std::tuple{shuffle_deck{.player = player_id{0}}},
        std::tuple{start_round{.max_rounds = 2}}
    );

    card_table table{library};
    executor execution{};
    execution.enter_entry(library);

    auto random = []() -> std::uint32_t { return 0; };
    while(execution.status() == game_result::no_result
          && execution.execute_next(table, random))
    {}

    std::cout << "round: " << table.state().round_number << '\n';
    std::cout << std::boolalpha
              << "both players lost: " << (execution.status() == game_result::both_loss) << '\n';
}
```

输出

```text
round: 2
both players lost: true
```

## 参阅

| | |
| --- | --- |
| [`card_table`](../table/card_table.md) | 保存牌桌状态 |
| [`definition_library`](../definition/definition_library.md) | 提供实体定义和规则程序 |
