[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_removed**

# givm::attachment_removed

定义于头文件 `<givm/definition.hpp>`

角色附属实体或装备已移除后向其他有效实体广播的通知。已移除的对象自身不参与响应；其定义和状态在 cleanup 前仍可按通知中的旧 ID 读取。主动移除和装备替换都会产生本通知。

```cpp
struct attachment_removed
{
    const attachment_id attachment;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `attachment` | `const attachment_id` | 已移除的角色附属实体或装备标识 |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    const givm::attachment_id original{};
    givm::attachment_removed event{ .attachment = original };
    std::println("通知保留原标识: {}", event.attachment == original);
}
```

输出

```text
通知保留原标识: true
```
