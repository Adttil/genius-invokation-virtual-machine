[givm](../../reference.md) / [事件](../events.md) / **entity_count_changed**

# givm::entity_count_changed

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct entity_count_changed;
```

具有次数或层数的实体，其计数已经改变的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `entity` | `const counted_entity_id` | 计数已改变的实体标识；只读 |
| `previous` | `const std::uint32_t` | 变化前的次数或层数；只读 |
| `current` | `const std::uint32_t` | 变化后的次数或层数；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::entity_count_changed event{ .entity = givm::combat_status_id{}, .previous = 1, .current = 3 };
    std::println("变化前: {}", event.previous);
    std::println("变化后: {}", event.current);
}
```

输出

```text
变化前: 1
变化后: 3
```
