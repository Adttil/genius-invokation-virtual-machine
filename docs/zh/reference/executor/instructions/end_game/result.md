[givm](../../../../reference.md) / [执行](../../../executor.md) / [end_game](../end_game.md) / **result**

# givm::end_game::result

定义于头文件 `<givm/executor.hpp>`

```cpp
game_result result;
```
[`game_result`](../../../enums/game_result.md)

执行本指令时采用的终局结果。

## 注意

执行本指令时必须为 `game_result::player_0_win`、`game_result::player_1_win` 或 `game_result::both_loss`。`game_result::no_result` 不表示任何终局，以该值执行本指令属于未定义行为。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    const givm::end_game finish{ .result = givm::game_result::both_loss };
    std::println("结束时判定双方失败: {}",
        finish.result == givm::game_result::both_loss);
}
```

输出

```text
结束时判定双方失败: true
```
