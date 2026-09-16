[givm](../../reference.md) / [执行](../executor.md) / **random_fn**

# givm::random_fn

定义于头文件 `<givm/executor.hpp>`

```cpp
class random_fn;
```

随机函数视图，供命令和事件响应访问调用方提供的随机源。

## 成员函数

|  |  |
| --- | --- |
| [`(构造函数)`](random_fn/constructor.md) | 构造一个随机函数视图 |
| [`operator()`](random_fn/operator_call.md) | 取得下一个随机值 |

## 注意

不拥有随机源，复制也不会保存独立的随机序列状态。所接入对象须在使用期间保持有效；不同构造方式的生命周期约定见[构造函数](random_fn/constructor.md)。

随机源无需采用特定生成算法，每次调用提供一个 `std::uint32_t` 值。一次执行推进中的命令和事件响应共享调用方传入的随机源，各次调用按实际执行顺序使用同一序列。随机值对应的游戏效果及消耗次序由各[命令](../definition/commands.md)和定义响应约定。

[`cost_of_switch`](../definition/events/cost_of_switch.md)、[`cost_of_card`](../definition/events/cost_of_card.md) 费用响应，以及本牌的 [`card_cost_initialization`](../definition/events/card_cost_initialization.md)、[`card_target_check`](../definition/events/card_target_check.md) 查询响应不得使用随机数；调用随机函数属于未定义行为。这些同步查询无需传入随机源。

## 示例

```cpp
#include <print>
#include <cstdint>

#include <givm/givm.hpp>

int main()
{
    std::uint32_t next = 7;
    auto source = [&]() -> std::uint32_t { return next++; };
    givm::random_fn random{ source };
    std::println("第一次取得的值: {}", random());
    std::println("第二次取得的值: {}", random());
}
```

输出

```text
第一次取得的值: 7
第二次取得的值: 8
```
