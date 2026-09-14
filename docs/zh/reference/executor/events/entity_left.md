[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **entity_left**

# givm::entity_left

定义于头文件 `<givm/executor.hpp>`

```cpp
struct entity_left;
```

实体离场后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `entity` | `const entity_id` | 已离场的实体标识；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::entity_left event{ .entity = givm::summon_id{} };
    std::println("离场对象是召唤物: {}", std::holds_alternative<givm::summon_id>(event.entity));
}
```

输出

```text
离场对象是召唤物: true
```
