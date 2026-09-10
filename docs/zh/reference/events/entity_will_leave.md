[givm](../../reference.md) / [事件](../events.md) / **entity_will_leave**

# givm::entity_will_leave

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct entity_will_leave;
```

一个实体即将离场。响应者可以在离场前处理相关效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `entity` | `const entity_id` | 即将离场的实体标识；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::entity_will_leave event{ .entity = givm::summon_id{} };
    std::println("离场对象是召唤物: {}", std::holds_alternative<givm::summon_id>(event.entity));
}
```

输出

```text
离场对象是召唤物: true
```
