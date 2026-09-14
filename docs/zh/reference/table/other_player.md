[givm](../../reference.md) / [牌桌](../table.md) / **other_player**

# givm::other_player

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr player_id other_player(player_id player) noexcept;
```

取得对方玩家的 ID。

## 参数

|  |  |
| --- | --- |
| `player` | 其中一方的 ID，编号为 0 或 1 |

## 返回值

另一方玩家的 [`player_id`](player_id.md)。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    const givm::player_id self{ 0 };
    std::println("对方玩家: {}", givm::other_player(self).index);
}
```

输出

```text
对方玩家: 1
```
