[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_state_modification**

# givm::attachment_state_modification

定义于头文件 `<givm/definition.hpp>`

[modify_attachment_state](../commands/modify_attachment_state.md) 的动态输入，指定要修改的角色附属实体以及本次变化。

```cpp
struct attachment_state_modification
{
    attachment_id attachment;
    std::int64_t count{};
    std::int64_t round_usages{};
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `attachment` | `attachment_id` | 要修改的有效实体 |
| `count` | `std::int64_t` | 对同名状态字段的增量，默认零 |
| `round_usages` | `std::int64_t` | 对同名状态字段的增量，默认零 |

增量作用于命令实际执行时的当前值，结果饱和至零和定义上限之间。
