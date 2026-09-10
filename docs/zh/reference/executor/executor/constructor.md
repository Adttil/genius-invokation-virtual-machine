[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **(构造函数)**

# givm::executor::executor

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr executor() noexcept;                     // (1)
constexpr executor(const executor& other);         // (2)
constexpr executor(executor&& other) noexcept;     // (3)
```
[`executor`](../executor.md)

构造执行器。

1. 构造空执行器。栈为空，[`status`](status.md) 返回 [`game_result::no_result`](../../enums/game_result.md)；开始执行前需要调用 [`enter_entry`](enter_entry.md)。
2. 复制 `other` 的执行位置和栈内容。两个执行器分别拥有各自的栈，随后推进其中一个不会改变另一个的执行状态。
3. 从 `other` 移动执行状态。

## 参数

| | |
| --- | --- |
| `other` | 要复制或移动的执行器 |

## 返回值

（无）

## 注意

复制执行器以建立模拟分支时，调用方应同时复制配套的 [`card_table`](../../table/card_table.md)。需要复现后续随机选择时，还应保存随机源状态。

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
    givm::card_table branch_table{ table };
    givm::executor branch{ execution };
    while(branch.execute_next(branch_table, random))
    {}
    std::println("原对局尚无结果: {}", execution.status() == givm::game_result::no_result);
    std::println("分支双方告负: {}", branch.status() == givm::game_result::both_loss);
}
```

输出

```text
原对局尚无结果: true
分支双方告负: true
```

## 参阅

| | |
| --- | --- |
| [`enter_entry`](enter_entry.md) | 建立从定义库入口开始的执行状态 |
