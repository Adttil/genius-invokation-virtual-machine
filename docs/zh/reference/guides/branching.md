[givm](../../reference.md) / [使用指南](../guides.md) / **分支模拟**

# 分支模拟

从同一个局面尝试不同选择时，需要让各分支拥有独立的牌桌、结算状态和随机源状态。这样，一个分支中的后续操作不会改变其他分支的局面。

复制 [`card_table`](../table/card_table.md) 和 [`executor`](../executor/executor.md)，并为分支准备相应随机源。各分支可以共享同一个 [`definition_library`](../definition/definition_library.md)，该定义库须在分支使用期间保持有效。

实体访问对象、view 和栈中的引用仍关联取得它们时的对象。建立分支后，应从分支牌桌和执行器重新取得访问对象。

## 示例

```cpp
#include <print>
#include <random>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    givm::card_table original_table{ library };
    givm::executor original{};
    original.enter_entry(library);
    std::mt19937 original_random{ 42 };

    givm::card_table branch_table{ original_table };
    givm::executor branch{ original };
    auto branch_random = original_random;

    while(branch.execute_next(branch_table, branch_random))
    {}

    std::println("原对局回合数: {}", original_table.state().round_number);
    std::println("分支回合数: {}", branch_table.state().round_number);
    std::println("原对局尚无结果: {}", original.status() == givm::game_result::no_result);
    std::println("分支已经终局: {}", branch.status() != givm::game_result::no_result);
}
```

输出

```text
原对局回合数: 0
分支回合数: 1
原对局尚无结果: true
分支已经终局: true
```
