[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **position**

# givm::executor::position

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr execution_position position() const noexcept;
```
[`execution_position`](../../definition/execution_position.md)

返回当前执行位置。将此位置传给配套定义库的 [`definition_library::instruction`](../../definition/definition_library/instruction.md)，可以取得下一条要执行的指令。

## 返回值

当前执行位置。

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
        std::tuple{start_round{.max_rounds = 0}}
    );

    card_table table{library};
    executor execution{};
    execution.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };

    std::cout << std::boolalpha
              << "next is shuffle_deck: "
              << library.instruction(execution.position()).is<shuffle_deck>()
              << '\n';

    execution.execute_next(table, random);

    std::cout << "next is start_round: "
              << library.instruction(execution.position()).is<start_round>()
              << '\n';
}
```

输出

```text
next is shuffle_deck: true
next is start_round: true
```

## 参阅

| | |
| --- | --- |
| [`definition_library::instruction`](../../definition/definition_library/instruction.md) | 取得指定执行位置的指令 |
| [`enter_entry`](enter_entry.md) | 建立从规则程序入口开始的执行现场 |
| [`execute_next`](execute_next.md) | 完整执行当前位置的一次指令 |
| [`status`](status.md) | 取得对局结果 |
