[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **relative_player**

# givm::relative_player

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
enum class relative_player : std::uint8_t
{
    current,
    other
};
```

相对于当前行动玩家的一方。

## 枚举值

| | |
| --- | --- |
| `current` | 当前行动玩家 |
| `other` | 另一名玩家 |

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::draw_cards instruction{ .count = 2, .player = givm::relative_player::other };
    std::println("为另一方抽牌: {}", instruction.player == givm::relative_player::other);
}
```

输出

```text
为另一方抽牌: true
```
