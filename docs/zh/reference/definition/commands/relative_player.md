[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **relative_player**

# givm::relative_player

定义于头文件 `<givm/definition.hpp>`

```cpp
enum class relative_player : std::uint8_t
{
    self,
    opponent
};
```

相对于当前效果本方的一方。响应返回的程序以响应实体所属玩家为本方，与当前轮到哪位玩家行动无关。

## 枚举值

| | |
| --- | --- |
| `self` | 当前效果的本方 |
| `opponent` | 当前效果的对方 |

## 本方的确定

命令执行时以 [`table_state::self_player`](../../table/table_state.md) 为基准。响应返回的程序执行期间，本方为该响应实体所属玩家；嵌套效果完成后恢复外层本方。费用预览不改变此字段，缓存的费用效果实际执行时才使用对应响应实体所属玩家。

`handle` 函数本身仍读取外层的本方值，不因为正在调用哪个响应而改变。响应需要知道自身所属玩家时，使用已有实体参数的 `player().id()`。

根流程默认没有本方，`self_player` 初始为 `player_id{ 2 }`。根流程需要使用这些相对命令时，调用方须显式为牌桌设置有效本方，例如构造 `table{ { .self_player = player_id{ 0 } } }`。没有有效本方时执行相对命令属于未定义行为，不回退到 `active_player`。动态输入中的精确玩家或实体 ID 不依赖此基准。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::draw_cards instruction{ .count = 2, .player = givm::relative_player::opponent };
    std::println("为另一方抽牌: {}", instruction.player == givm::relative_player::opponent);
}
```

输出

```text
为另一方抽牌: true
```
