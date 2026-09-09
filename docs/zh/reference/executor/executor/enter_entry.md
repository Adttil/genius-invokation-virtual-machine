[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **enter_entry**

# givm::executor::enter_entry

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void enter_entry(const definition_library& library);
```
[`definition_library`](../../definition/definition_library.md)

重置执行现场，准备从 `library` 的规则程序入口开始执行。

原有的待完成结算被丢弃。本函数不执行指令；通过 [`execute_next`](execute_next.md) 开始执行。

## 参数

| | |
| --- | --- |
| `library` | 提供待执行规则程序的定义库 |

## 返回值

（无）

## 注意

本函数不修改牌桌。调用方应准备与该次执行配套的 [`card_table`](../../table/card_table.md)，并使用同一个 `library`。

## 示例

```cpp
#include <givm/givm.hpp>

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
    table.state().round_number = 4;

    std::cout << "round before enter_entry: " << table.state().round_number << '\n';

    executor execution{};
    execution.enter_entry(library);

    std::cout << std::boolalpha
              << "next is shuffle_deck: "
              << library.instruction(execution.position()).is<shuffle_deck>() << '\n';
    std::cout << "round after enter_entry: " << table.state().round_number << '\n';
}
```

输出

```text
round before enter_entry: 4
next is shuffle_deck: true
round after enter_entry: 4
```

## 参阅

| | |
| --- | --- |
| [`execute_next`](execute_next.md) | 完整执行当前位置的一次指令 |
| [`clear`](clear.md) | 清空执行栈 |
| [`position`](position.md) | 取得当前执行位置 |
