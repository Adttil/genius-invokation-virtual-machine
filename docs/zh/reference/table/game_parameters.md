[givm](../../reference.md) / [牌桌](../table.md) / **game_parameters**

# givm::game_parameters

定义于头文件 `<givm/table.hpp>`

```cpp
struct game_parameters;
```

对局采用的公共参数，包括手牌数量上限与最多进行的回合数。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `hand_limit` | `std::uint32_t` | 手牌上限，默认 10 |
| `max_rounds` | `std::uint32_t` | 允许执行的最大回合数，默认 14 |

进入根回合流程前自动增加回合数并检查 `max_rounds`，超过时双方失败；初始化流程不受此项限制。详见 [`compile`](../executor/compile.md#自动回合推进)。

`max_rounds` 应小于 `UINT32_MAX`，使超限后的回合数仍可表示。

支援区容量由每位玩家的 [`player_state::support_limit`](player_state.md) 提供，默认值为 4。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::game_parameters value{ .hand_limit = 12 };
    std::println("手牌上限: {}", value.hand_limit);
}
```

输出

```text
手牌上限: 12
```
