[givm](../../reference.md) / [定义](../definition.md) / **query_default**

# givm::query_default

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr character_state query_default(const character_initial_state&) noexcept;
constexpr action_cost_requirement query_default(const card_initial_cost&) noexcept;
constexpr target_validation query_default(const card_target_validation& query) noexcept;
```

定义源没有提供某项查询时，给出该查询的默认结果。定义源协议通过未限定名称的 `query_default(parameters)` 调用，以参数相关查找（ADL）选择匹配方法。

## 返回值

| 查询 | 默认结果 |
| --- | --- |
| [`character_initial_state`](queries/character_initial_state.md) | `character_state{}`。 |
| [`card_initial_cost`](queries/card_initial_cost.md) | 零骰子费用，行动速度为 `action_speed::fast`。 |
| [`card_target_validation`](queries/card_target_validation.md) | `target_count == 0` 时为 `valid_complete`，否则为 `invalid`。 |

## 注意

默认方法与源查询的返回类型都须正好等于查询类型的 `result_t`。空查询的默认结果也在编译定义库时保存；非空查询使用本次传入的参数。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    const auto state = givm::query_default(givm::character_initial_state{});
    const auto cost = givm::query_default(givm::card_initial_cost{});
    std::println("默认初始生命: {}", state.health);
    std::println("默认快速行动: {}", cost.speed == givm::action_speed::fast);
}
```

输出

```text
默认初始生命: 0
默认快速行动: true
```
