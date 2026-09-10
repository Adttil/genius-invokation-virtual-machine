[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **energy_changed**

# givm::energy_changed

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct energy_changed;
```

角色充能变化后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 充能已改变的角色；只读 |
| `previous` | `const std::uint32_t` | 变化前的充能；只读 |
| `current` | `const std::uint32_t` | 变化后的充能；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::energy_changed event{ .target = {}, .previous = 1, .current = 3 };
    std::println("变化前: {}", event.previous);
    std::println("变化后: {}", event.current);
}
```

输出

```text
变化前: 1
变化后: 3
```
