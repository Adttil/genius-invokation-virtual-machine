[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **changing_secret_points**

# givm::changing_secret_points

定义于头文件 `<givm/executor.hpp>`

```cpp
struct changing_secret_points;
```

秘传点数变化前的调整事件。响应者可以调整本次变化量。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `const player_id` | 这次事件对应的玩家；只读 |
| `delta` | `std::int32_t` | 本次变化量；正数增加，负数减少 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::changing_secret_points event{ .player = givm::player_id{ 0 }, .delta = 1 };
    ++event.delta;
    std::println("调整后的增加量: {}", event.delta);
}
```

输出

```text
调整后的增加量: 2
```
