[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **support_state_change**

# givm::support_state_change

定义于头文件 `<givm/definition.hpp>`

[set_support_state](../commands/set_support_state.md) 的动态输入，指定要修改的支援以及本次变化。

```cpp
struct support_state_change
{
    support_id support;
    support_state state;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `support` | `support_id` | 要修改的有效实体 |
| `state` | `support_state` | 要设置的完整状态，默认两个字段均为零；执行时裁剪至定义上限 |
