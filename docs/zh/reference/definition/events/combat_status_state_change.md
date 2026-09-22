[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **combat_status_state_change**

# givm::combat_status_state_change

定义于头文件 `<givm/definition.hpp>`

[set_combat_status_state](../commands/set_combat_status_state.md) 的动态输入，指定要修改的出战状态以及本次变化。

```cpp
struct combat_status_state_change
{
    combat_status_id status;
    combat_status_state state;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `status` | `combat_status_id` | 要修改的有效实体 |
| `state` | `combat_status_state` | 要设置的完整状态，默认两个字段均为零；执行时裁剪至定义上限 |
