[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **execute_next**

# givm::executor::execute_next

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
constexpr bool execute_next(card_table& table, TRandom& random_source);
```
[`card_table`](../../table/card_table.md)

完成对局推进中的一次指令执行。

本次指令执行同步完成，不可中断。一次结算可能包含多条指令或同一指令的多次重入，因此可能跨越多次调用。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的对象类型；其左值须可无参数调用，且调用结果可隐式转换为 `std::uint32_t` |

## 参数

| | |
| --- | --- |
| `table` | 与当前执行现场配套的牌桌；其定义库须与建立该执行现场时使用的定义库相同 |
| `random_source` | 本次执行使用的随机源，以左值传入；执行器不会在返回后持有它 |

## 返回值

`true` 表示本次执行未请求暂停；`false` 表示应停止自动推进，将控制权交给调用方。对局正常结束时也返回 `false`，它不表示指令执行失败。

是否终局应通过 [`status`](status.md) 判断。尚未终局而返回 `false` 时，可将 [`position`](position.md) 返回的位置传给 [`definition_library::instruction`](../../definition/definition_library/instruction.md)，取得下一条要执行的指令，按其约定处理输入或观察。

## 注意

推进尚未结束的对局前，须由 [`enter_entry`](enter_entry.md) 准备开始，或从另一执行器复制、移动取得有效的对局进度。经 [`clear`](clear.md) 丢弃未完成的结算后，开始新对局前应调用 `enter_entry`。若正在等待输入，继续执行前须按该指令的约定提供输入。已经终局时，调用仍返回 `false`。

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
        std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("尚未终局: {}", execution.status() == givm::game_result::no_result);
    std::println("正在等待换牌输入: {}", library.instruction(execution.position()).is<givm::replace_cards>());
}
```

输出

```text
尚未终局: true
正在等待换牌输入: true
```

## 参阅

| | |
| --- | --- |
| [`enter_entry`](enter_entry.md) | 建立执行现场 |
| [`position`](position.md) | 取得当前执行位置 |
| [`status`](status.md) | 取得对局结果 |
| [`stack`](stack.md) | 访问指令约定的输入槽 |
