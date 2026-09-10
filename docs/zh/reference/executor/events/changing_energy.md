[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **changing_energy**

# givm::changing_energy

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct changing_energy;
```

角色充能变化前的调整事件。响应者可以调整本次变化量。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 充能将要改变的角色；只读 |
| `delta` | `std::int32_t` | 本次变化量；正数增加，负数减少 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::changing_energy event{ .target = {}, .delta = 1 };
    ++event.delta;
    std::println("调整后的增加量: {}", event.delta);
}
```

输出

```text
调整后的增加量: 2
```
