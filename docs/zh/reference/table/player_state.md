[givm](../../reference.md) / [牌桌](../table.md) / **player_state**

# givm::player_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct player_state;
```

一位玩家在对局中的状态，包括持有的骰子、出战角色及当前支援区和召唤区容量。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `dice` | [`dice_counts`](../enums/dice_counts.md) | 玩家持有的各类元素骰 |
| `active_character` | `std::optional<character_id>` | 出战角色的 ID；没有出战角色时为空 |
| `support_limit` | `std::uint32_t` | 该玩家当前可容纳的支援数量上限，默认 4 |
| `summon_limit` | `std::uint32_t` | 该玩家当前可容纳的召唤物数量上限，默认 4 |

两位玩家分别拥有自己的 `summon_limit`。上限只限制新召唤物的创建；若当前上限低于已有召唤物数量，已有召唤物仍然保留。同定义召唤物仍会收到重复召唤请求，详见 [summon](../definition/commands/summon.md)。

两位玩家也分别拥有自己的 `support_limit`。[`add_support`](../definition/commands/add_support.md) 按执行时的有效支援数量判断是否仍有空位；降低上限不会移除已有支援。支援上限配置位于各玩家的状态中，不再由 `game_parameters` 提供。

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
    std::println("召唤物上限: {}", value.summon_limit);
}
```

输出

```text
骰子总数: 2
已有出战角色: false
召唤物上限: 4
```
