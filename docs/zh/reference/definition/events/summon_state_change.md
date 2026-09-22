[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **summon_state_change**

# givm::summon_state_change

定义于头文件 `<givm/definition.hpp>`

[set_summon_state](../commands/set_summon_state.md) 的动态输入，指定要修改的召唤物以及本次变化。

```cpp
struct summon_state_change
{
    summon_id summon;
    summon_state state;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `summon` | `summon_id` | 要修改的有效实体 |
| `state` | `summon_state` | 要设置的完整状态，默认两个字段均为零；执行时裁剪至定义上限 |
