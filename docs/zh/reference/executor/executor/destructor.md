[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **(析构函数)**

# givm::executor::~executor

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr ~executor();
```
[`executor`](../executor.md)

销毁执行器，释放它拥有的栈存储。

## 返回值

（无）

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
    const auto [library, id_map] = sources.compile(
        std::tuple{shuffle_deck{.player = player_id{0}}},
        std::tuple{start_round{.max_rounds = 1}}
    );

    card_table table{library};
    auto random = []() -> std::uint32_t { return 0; };

    {
        executor execution{};
        execution.enter_entry(library);
        while(execution.status() == game_result::no_result
              && execution.execute_next(table, random))
        {}
    }

    // 执行器析构后，牌桌仍保留结算结果。
    std::cout << "round after executor destruction: "
              << table.state().round_number << '\n';
}
```

输出

```text
round after executor destruction: 1
```

## 参阅

| | |
| --- | --- |
| [`clear`](clear.md) | 清空执行器的栈 |
