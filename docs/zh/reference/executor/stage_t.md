[givm](../../reference.md) / [执行](../executor.md) / **stage_t**

# givm::stage_t

定义于头文件 `<givm/executor.hpp>`

```cpp
using stage_t = std::uint8_t;
```

指令用来记录自身结算进度的数值类型。

## 注意

各值的含义由使用它的指令决定。外部调用方仅在某个输入槽约定要求该类型时使用它，不应据此推断其他结算状态的布局。

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
    execution.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };
    const bool continued = execution.execute_next(table, random);
    std::println("等待替换手牌: {}", !continued);
    {
        auto&& [selection, stage] = execution.stack().top<givm::selector, givm::stage_t>();
        // stage 是输入槽后需要保留的尾部状态，无需读取或修改。
        std::println("输入玩家: {}", selection.player.index);
        selection.selected.reset(); // 空选择表示保留全部手牌。
    }
    std::println("提交选择后继续执行: {}", execution.execute_next(table, random));
    std::println("下一条是回合开始: {}",
        library.instruction(execution.position()).is<givm::start_round>());
}
```

输出

```text
等待替换手牌: true
输入玩家: 0
提交选择后继续执行: true
下一条是回合开始: true
```

## 参阅

|  |  |
| --- | --- |
| [`execution_context::current_stage`](execution_context/current_stage.md) | 访问本条指令的进度 |
