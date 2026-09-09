[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **execute_next**

# givm::executor::execute_next

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
constexpr bool execute_next(card_table& table, TRandom& random_source);
```
[`card_table`](../../table/card_table.md)

完整执行当前位置的一次指令，并更新牌桌与执行现场。

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

`true` 表示本次执行未请求暂停；`false` 表示应停止自动推进，将控制权交给调用方。

是否终局应通过 [`status`](status.md) 判断。尚未终局而返回 `false` 时，可将 [`position`](position.md) 返回的位置传给 [`definition_library::instruction`](../../definition/definition_library/instruction.md)，取得下一条要执行的指令，按其约定处理输入或观察。

## 注意

调用前须具有有效执行现场，由 [`enter_entry`](enter_entry.md) 建立或从另一执行器复制、移动取得；经 [`clear`](clear.md) 清空后须重新建立。若正在等待输入，继续执行前须按该指令的约定提供输入。

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
        std::tuple{
            shuffle_deck{.player = player_id{0}},
            replace_cards{.player = player_id{0}}
        },
        std::tuple{start_round{}}
    );

    card_table table{library};
    executor execution{};
    execution.enter_entry(library);

    auto random = []() -> std::uint32_t { return 0; };
    // 自动推进到输入、观察或终局暂停点。
    while(execution.status() == game_result::no_result
          && execution.execute_next(table, random))
    {}

    std::cout << std::boolalpha
              << "has no result: " << (execution.status() == game_result::no_result) << '\n'
              << "waiting at replace_cards: "
              << library.instruction(execution.position()).is<replace_cards>() << '\n';
}
```

输出

```text
has no result: true
waiting at replace_cards: true
```

## 参阅

| | |
| --- | --- |
| [`enter_entry`](enter_entry.md) | 建立执行现场 |
| [`position`](position.md) | 取得当前执行位置 |
| [`status`](status.md) | 取得对局结果 |
| [`stack`](stack.md) | 访问指令约定的输入槽 |
