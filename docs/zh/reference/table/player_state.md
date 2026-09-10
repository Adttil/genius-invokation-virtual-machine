[givm](../../reference.md) / [牌桌](../table.md) / **player_state**

# givm::player_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct player_state;
```

一位玩家的骰子和出战角色状态。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `dice` | [`dice_counts`](../enums/dice_counts.md) | 玩家持有的各类元素骰 |
| `active_character` | `std::optional<character_id>` | 出战角色的 ID；没有出战角色时为空 |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::player_state value{};
    value.dice[givm::elemental_dice::omni] = 2;
    std::println("骰子总数: {}", value.dice.total());
    std::println("已有出战角色: {}", value.active_character.has_value());
}
```

输出

```text
骰子总数: 2
已有出战角色: false
```
