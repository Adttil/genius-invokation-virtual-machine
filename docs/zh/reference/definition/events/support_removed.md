[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **support_removed**

# givm::support_removed

定义于头文件 `<givm/definition.hpp>`

支援已移除后向其他有效实体广播的通知。已移除的对象自身不参与响应；其定义和状态在 cleanup 前仍可按通知中的旧 ID 读取。

```cpp
struct support_removed
{
    const support_id support;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `support` | `const support_id` | 已移除的支援标识 |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    const givm::support_id original{};
    givm::support_removed event{ .support = original };
    std::println("通知保留原标识: {}", event.support == original);
}
```

输出

```text
通知保留原标识: true
```
