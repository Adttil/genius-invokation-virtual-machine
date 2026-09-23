[givm](../../reference.md) / [牌桌](../table.md) / **table_state**

# givm::table_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct table_state;
```

双方共享的对局状态，用于记录当前回合、行动归属和当前效果的本方。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `max_rounds` | `std::uint32_t` | 最大回合数，默认 14；应小于 `UINT32_MAX` |
| `round_number` | `std::uint32_t` | 当前回合数，初始为 0 |
| `active_player` | [`player_id`](player_id.md) | 当前行动玩家，初始为玩家 0 |
| `self_player` | [`player_id`](player_id.md) | 当前效果的本方；初始 `player_id{ 2 }` 表示没有本方 |
| `first_ended` | `bool` | 本回合是否已经有玩家率先宣布结束，初始为 false |

进入根回合流程前自动增加回合数，超过 `max_rounds` 时双方失败；初始化流程不受此项限制。详见 [`compile`](../executor/compile.md#自动回合推进)。

## 当前效果的本方

响应返回的程序执行期间，`self_player` 为响应实体所属玩家。嵌套效果及其后续程序完成后恢复外层值；普通 `handle` 调用本身不切换本方。费用预览保持牌桌不变，实际执行已选费用效果时才建立相应本方。

根流程默认没有本方。需要在根流程执行抽牌、固定伤害等相对效果命令时，调用方须显式设置 `self_player` 为玩家 0 或 1。`player_id{ 2 }` 不能用于访问玩家；没有有效本方时执行相对命令属于未定义行为，不自动使用 `active_player`。精确玩家、实体 ID 的动态输入不受此限制。详见 [`relative_player`](../definition/commands/relative_player.md)。

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
