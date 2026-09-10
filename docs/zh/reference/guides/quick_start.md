[givm](../../reference.md) / [使用指南](../guides.md) / **开始使用**

# 开始使用

创建一场对局时，先准备规则，再建立使用这些规则的牌桌和执行器。下面的示例采用只推进回合数的流程，展示创建、运行和读取结果三个步骤。

## 接入 CMake 工程

本库需要支持 C++23 的编译器。将仓库加入工程后，链接 `givm::givm`：

```cmake
add_subdirectory(path/to/givm)
target_link_libraries(your_target PRIVATE givm::givm)
```

可通过 `<givm/givm.hpp>` 引入完整公开接口，也可按需要引入 `<givm/definition.hpp>`、`<givm/table.hpp>` 和 `<givm/executor.hpp>`。

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
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 2 } }
    );

    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };

    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}

    std::println("经过的回合数: {}", table.state().round_number);
    std::println("是否双方告负: {}", execution.status() == givm::game_result::both_loss);
}
```

输出

```text
经过的回合数: 2
是否双方告负: true
```

要加入自己的角色、卡牌和效果，参阅[定义](../definition.md)。牌组需要先[链接](../definition/link_deck.md)，再由 [`card_table::load_deck`](../table/card_table/load_deck.md) 装载到牌桌中。
