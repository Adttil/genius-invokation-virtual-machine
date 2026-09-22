[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **combat_status_removed**

# givm::combat_status_removed

定义于头文件 `<givm/definition.hpp>`

出战状态已移除后向其他有效实体广播的通知。已移除的对象自身不参与响应；其定义和状态在 cleanup 前仍可按通知中的旧 ID 读取。

```cpp
struct combat_status_removed
{
    const combat_status_id status;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `status` | `const combat_status_id` | 已移除的出战状态标识 |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    const givm::combat_status_id original{};
    givm::combat_status_removed event{ .status = original };
    std::println("通知保留原标识: {}", event.status == original);
}
```

输出

```text
通知保留原标识: true
```
