[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **secret_points_changed**

# givm::secret_points_changed

定义于头文件 `<givm/definition.hpp>`

```cpp
struct secret_points_changed;
```

秘传点数变化后的通知。它同时提供变化前后的点数。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `const player_id` | 这次事件对应的玩家；只读 |
| `previous` | `const std::uint32_t` | 变化前的秘传点数；只读 |
| `current` | `const std::uint32_t` | 变化后的秘传点数；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::secret_points_changed event{ .player = givm::player_id{ 0 }, .previous = 1, .current = 3 };
    std::println("变化前: {}", event.previous);
    std::println("变化后: {}", event.current);
}
```

输出

```text
变化前: 1
变化后: 3
```
