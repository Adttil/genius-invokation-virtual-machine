[givm](../../reference.md) / [执行](../executor.md) / **executor**

# givm::executor

定义于头文件 `<givm/executor.hpp>`

```cpp
class executor;
```

游戏对局的执行器。

它是所有牌桌（[`table`](../table/table.md)）以外状态的记录者，比如对局进行到了哪个阶段、结算进行到了哪一步、正在发生的事件等。

## 成员函数

| | |
| --- | --- |
| [`(构造函数)`](executor/constructor.md) | 构造一个`executor` |
| [`enter_entry`](executor/enter_entry.md) | 准备按照定义库的流程开始对局 |
| [`step`](executor/step.md) | 推进至编译模式要求报告的下一处现场 |
| [`view_in`](executor/view_in.md) | 取得指定种类的当前执行现场视图 |

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
    const auto [library, id_map] = compile(
        sources,
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 2 } }, givm::compile_mode::normal
    );
    givm::table table{};
    auto random = []() -> std::uint32_t { return 0; };

    givm::executor execution{};

    execution.enter_entry(library);
    const auto state = execution.step(library, table, random);

    std::println("终局时的回合数: {}", table.state().round_number);
    std::println("是否双败: {}", state == givm::execution_state::finished
        && execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}
```

输出

```text
终局时的回合数: 3
是否双败: true
```

## 参阅

| | |
| --- | --- |
| [`table`](../table/table.md) | 游戏对局的牌桌 |
| [`definition_library`](definition_library.md) | 编译后的定义库 |
