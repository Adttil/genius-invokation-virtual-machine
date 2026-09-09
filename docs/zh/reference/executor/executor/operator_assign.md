[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **operator=**

# givm::executor::operator=

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr executor& operator=(const executor& other);      // (1)
constexpr executor& operator=(executor&& other) noexcept;  // (2)
```
[`executor`](../executor.md)

用 `other` 的执行状态替换本执行器的执行状态。

1. 复制 `other` 的执行位置和栈内容。赋值后，两个执行器分别拥有各自的栈。
2. 从 `other` 移动执行状态。

## 参数

| | |
| --- | --- |
| `other` | 要复制或移动的执行器 |

## 返回值

`*this`。

## 注意

继续执行时，调用方应提供与赋值后执行状态配套的 [`card_table`](../../table/card_table.md) 和随机源。复制模拟分支时对牌桌和随机源状态的处理方式参见[(构造函数)](constructor.md)。

## 示例

```cpp
#include <givm/givm.hpp>

#include <cstdint>
#include <iostream>
#include <tuple>
#include <utility>

int main()
{
    using namespace givm;

    definition_source_library sources{};
    const auto [library, id_map] = sources.compile(
        std::tuple{shuffle_deck{.player = player_id{0}}},
        std::tuple{start_round{.max_rounds = 0}}
    );

    card_table table{library};
    executor original{};
    original.enter_entry(library);

    // 分支同时复制牌桌和执行器。
    card_table branch_table{table};
    executor saved{};
    saved = original;
    executor branch{};
    branch = std::move(saved);
    auto random = []() -> std::uint32_t { return 0; };

    while(branch.status() == game_result::no_result
          && branch.execute_next(branch_table, random))
    {}

    std::cout << std::boolalpha
              << "original has no result: " << (original.status() == game_result::no_result) << '\n'
              << "branch: both players lost: " << (branch.status() == game_result::both_loss) << '\n';
}
```

输出

```text
original has no result: true
branch: both players lost: true
```

## 参阅

| | |
| --- | --- |
| [(构造函数)](constructor.md) | 构造执行器 |
| [`execute_next`](execute_next.md) | 完整执行当前位置的一次指令 |
