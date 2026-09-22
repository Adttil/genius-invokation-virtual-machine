[givm](../../reference.md) / [牌桌](../table.md) / **table_state**

# givm::table_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct table_state;
```

双方共享的对局状态，用于记录当前回合和行动归属。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `max_rounds` | `std::uint32_t` | 最大回合数，默认 14；应小于 `UINT32_MAX` |
| `round_number` | `std::uint32_t` | 当前回合数，初始为 0 |
| `active_player` | [`player_id`](player_id.md) | 当前行动玩家，初始为玩家 0 |
| `first_ended` | `bool` | 本回合是否已经有玩家率先宣布结束，初始为 false |

进入根回合流程前自动增加回合数，超过 `max_rounds` 时双方失败；初始化流程不受此项限制。详见 [`compile`](../executor/compile.md#自动回合推进)。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::table_state value{};
    value.round_number = 3;
    value.active_player = givm::player_id{ 1 };
    std::println("回合: {}", value.round_number);
    std::println("行动玩家: {}", value.active_player.index);
}
```

输出

```text
回合: 3
行动玩家: 1
```
