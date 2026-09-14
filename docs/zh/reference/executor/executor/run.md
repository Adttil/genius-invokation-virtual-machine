[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **run**

# givm::executor::run

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
execution_state run(
    const definition_library& library,
    table& card_table,
    TRandom& random_source
);
```
[`execution_state`](../execution_state.md)
[`table`](../../table/table.md)

推进对局，直到需要调用方输入或对局结束。

本函数略过只供观察的现场。开始执行前须已调用 [`enter_entry`](enter_entry.md)，或持有从有效执行器复制、移动取得的对应现场。在输入现场按相应视图的约定填写输入后，再继续推进。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的对象类型；其左值须可无参数调用，且结果可隐式转换为 `std::uint32_t` |

## 参数

| | |
| --- | --- |
| `library` | 与当前执行现场配套的定义库；执行器不会在返回后持有它 |
| `card_table` | 与当前执行现场配套的牌桌，其中的定义 ID 须属于本次使用的定义库 |
| `random_source` | 本次推进使用的随机源，以左值传入；执行器不会在返回后持有它 |

## 返回值

本次到达的输入现场种类，或表示对局已经结束的 `execution_state::finished`。通过 [`view_in`](view_in.md) 取得与返回值对应的视图。

## 注意

每次推进须使用与建立当前现场时相同的编译产物。牌桌与执行器都不保存定义库指针；调用方负责保持程序现场、实体定义 ID 和所传定义库相匹配。

本函数用于初始现场或输入现场。从 [`step`](step.md) 返回的仅供观察的现场继续时，应继续调用 `step`。已经到达 `finished` 的对局不能继续执行；重新开始应调用 `enter_entry`。

输入操作只填写参数，不推进对局，也不验证是否满足该操作的前置条件。

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
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::table table{};
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    const auto state = execution.run(library, table, random);
    std::println("等待换牌: {}", state == givm::execution_state::card_selection);
    execution.view_in<givm::execution_state::card_selection>().select({});
    std::println("提交后到达终局: {}",
        execution.run(library, table, random) == givm::execution_state::finished);
}
```

输出

```text
等待换牌: true
提交后到达终局: true
```

## 参阅

| | |
| --- | --- |
| [`step`](step.md) | 推进至下一处可观察现场、输入现场或终局 |
| [`view_in`](view_in.md) | 取得当前现场视图 |
