[givm](../../reference.md) / [牌桌](../table.md) / **attachment_state**

# givm::attachment_state

定义于头文件 `<givm/table.hpp>`

角色附属实体的层数与本回合剩余次数。两者独立保存；层数或本回合次数归零后是否离场，由该实体的状态修改响应决定。

```cpp
struct attachment_state
{
    std::uint32_t count;
    std::uint32_t round_usages;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `count` | `std::uint32_t` | 当前层数 |
| `round_usages` | `std::uint32_t` | 本回合剩余可用次数 |

每回合次数的重置由定义在相应回合事件中，通过 [set_attachment_state](../definition/commands/set_attachment_state.md) 表达；次数上限由 [attachment_state_limit](../definition/queries/attachment_state_limit.md) 给出，重置时机由定义决定。

定义可以将多个独立的回合次数编码在 `round_usages` 的不同位段中，由定义自行解释和更新。状态命令仍将其当作一个整数，不会分别处理这些位段。上限查询的 `round_usages` 应返回各项次数全部恢复后的编码，[transfer_attachment](../definition/commands/transfer_attachment.md) 的可选次数恢复直接采用该值。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::attachment_state value{ .count = 3 };
    --value.count;
    std::println("剩余计数: {}", value.count);
}
```

输出

```text
剩余计数: 2
```
