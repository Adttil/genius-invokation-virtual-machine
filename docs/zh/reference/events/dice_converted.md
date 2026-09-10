[givm](../../reference.md) / [事件](../events.md) / **dice_converted**

# givm::dice_converted

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct dice_converted;
```

玩家的一部分元素骰已经转换种类的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `const player_id` | 这次事件对应的玩家；只读 |
| `from` | `const elemental_dice` | 转换或调和前的骰子种类；只读 |
| `to` | `const elemental_dice` | 转换或调和后的骰子种类；只读 |
| `count` | `const std::uint8_t` | 本次转换的骰子数量；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::dice_converted event{ .player = givm::player_id{ 0 }, .from = givm::elemental_dice::pyro, .to = givm::elemental_dice::omni, .count = 2 };
    std::println("转换数量: {}", event.count);
    std::println("转为万能骰: {}", event.to == givm::elemental_dice::omni);
}
```

输出

```text
转换数量: 2
转为万能骰: true
```
