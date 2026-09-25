[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **summon_removed**

# givm::summon_removed

定义于头文件 `<givm/definition.hpp>`

召唤物已移除后向其他有效实体广播的通知。已移除的对象自身不参与响应；其定义和状态在 cleanup 前仍可按通知中的旧 ID 读取。[remove_summon](../commands/remove_summon.md) 的显式移除，以及 [modify_summon_state](../commands/modify_summon_state.md) 对带 `remove_at_zero_usages` 标签的召唤物进行耗尽移除，都会产生本通知。

```cpp
struct summon_removed
{
    const summon_id summon;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `summon` | `const summon_id` | 已移除的召唤物标识 |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    const givm::summon_id original{};
    givm::summon_removed event{ .summon = original };
    std::println("通知保留原标识: {}", event.summon == original);
}
```

输出

```text
通知保留原标识: true
```
