[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **support_state_modification**

# givm::support_state_modification

定义于头文件 `<givm/definition.hpp>`

[modify_support_state](../commands/modify_support_state.md) 的动态输入，指定要修改的支援以及本次变化。

```cpp
struct support_state_modification
{
    support_id support;
    std::int64_t count{};
    std::int64_t round_usages{};
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `support` | `support_id` | 要修改的有效实体 |
| `count` | `std::int64_t` | 对同名状态字段的增量，默认零 |
| `round_usages` | `std::int64_t` | 对同名状态字段的增量，默认零 |

增量作用于命令实际执行时的当前值，结果饱和至零和定义上限之间。
