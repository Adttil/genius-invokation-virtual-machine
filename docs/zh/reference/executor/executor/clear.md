[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **clear**

# givm::executor::clear

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void clear() noexcept;
```

清空执行栈，丢弃其中保存的结算状态。

调用后，[`stack`](stack.md) 返回的栈为空。[`position`](position.md) 和 [`status`](status.md) 的返回值保持不变。此函数不修改与执行器配合使用的 [`card_table`](../../table/card_table.md)。

## 返回值

（无）

## 注意

清空后不能继续原有结算。要重新开始执行，应调用 [`enter_entry`](enter_entry.md)。

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
    executor execution{};
    execution.enter_entry(library);

    auto random = []() -> std::uint32_t { return 0; };
    while(execution.status() == game_result::no_result
          && execution.execute_next(table, random))
    {}

    const auto previous_position = execution.position();
    const auto previous_result = execution.status();
    execution.clear();

    std::cout << std::boolalpha
              << "stack empty: " << execution.stack().empty() << '\n'
              << "position preserved: "
              << (execution.position() == previous_position) << '\n'
              << "result preserved: "
              << (execution.status() == previous_result) << '\n'
              << "round: " << table.state().round_number << '\n';
}
```

输出

```text
stack empty: true
position preserved: true
result preserved: true
round: 1
```

## 参阅

| | |
| --- | --- |
| [`enter_entry`](enter_entry.md) | 从定义库入口建立新的执行状态 |
| [`stack`](stack.md) | 访问执行栈 |
