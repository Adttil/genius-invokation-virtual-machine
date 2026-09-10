[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **clear**

# givm::executor::clear

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void clear() noexcept;
```

丢弃尚未完成的结算及其临时数据。

调用后，[`stack`](stack.md) 返回的栈为空。[`position`](position.md) 和 [`status`](status.md) 的返回值保持不变。此函数不修改与执行器配合使用的 [`card_table`](../../table/card_table.md)。

## 返回值

（无）

## 注意

清空后不能继续原有结算。要重新开始执行，应调用 [`enter_entry`](enter_entry.md)。

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
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    const auto previous_position = execution.position();
    const auto previous_result = execution.status();
    execution.clear();
    std::println("临时数据已清空: {}", execution.stack().empty());
    std::println("执行位置保留: {}", execution.position() == previous_position);
    std::println("对局结果保留: {}", execution.status() == previous_result);
}
```

输出

```text
临时数据已清空: true
执行位置保留: true
对局结果保留: true
```

## 参阅

| | |
| --- | --- |
| [`enter_entry`](enter_entry.md) | 从定义库入口建立新的执行状态 |
| [`stack`](stack.md) | 访问执行栈 |
