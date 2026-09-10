[givm](../../reference.md) / [枚举值](../enums.md) / **game_result**

# givm::game_result

定义于头文件 `<givm/enums/game_result.hpp>`

```cpp
enum class game_result : std::uint8_t
{
    no_result = 0,
    player_0_win = 1,
    player_1_win = 2,
    both_loss = 3
};
```

一场对局的结果，区分尚未结束、某一方获胜和双方均告负。

## 枚举值

|  |  |
| --- | --- |
| `no_result` | 尚无对局结果 |
| `player_0_win` | 玩家 0 获胜 |
| `player_1_win` | 玩家 1 获胜 |
| `both_loss` | 双方均告负 |

## 示例

```cpp
#include <algorithm>
#include <array>
#include <print>

#include <givm/enums/game_result.hpp>

int main()
{
    // 汇总几次模拟的结果，尚未结束的对局不计入完成数量。
    const std::array results{
        givm::game_result::player_0_win,
        givm::game_result::player_1_win,
        givm::game_result::both_loss,
        givm::game_result::player_0_win,
        givm::game_result::no_result
    };
    const auto unfinished = std::ranges::count(results, givm::game_result::no_result);
    std::println("已完成对局: {}", results.size() - unfinished);
    std::println("玩家 0 胜场: {}", std::ranges::count(results, givm::game_result::player_0_win));
}
```

输出

```text
已完成对局: 4
玩家 0 胜场: 2
```
