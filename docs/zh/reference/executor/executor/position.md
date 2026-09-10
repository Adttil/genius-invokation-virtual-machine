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
#include <print>
#include <cstdint>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    std::println("下一条是洗牌: {}", library.instruction(execution.position()).is<givm::shuffle_deck>());
    execution.execute_next(table, random);
    std::println("下一条是回合开始: {}", library.instruction(execution.position()).is<givm::start_round>());
}
```

输出

```text
下一条是洗牌: true
下一条是回合开始: true
```

## 参阅

| | |
| --- | --- |
| [`definition_library::instruction`](../../definition/definition_library/instruction.md) | 取得指定执行位置的指令 |
| [`enter_entry`](enter_entry.md) | 准备开始一场对局 |
| [`execute_next`](execute_next.md) | 完整执行当前位置的一次指令 |
| [`status`](status.md) | 取得对局结果 |
