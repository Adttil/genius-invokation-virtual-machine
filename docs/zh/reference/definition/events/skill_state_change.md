[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **skill_state_change**

# givm::skill_state_change

定义于头文件 `<givm/definition.hpp>`

```cpp
struct skill_state_change
{
    skill_id skill;
    skill_state state;
};
```

[`set_skill_state`](../commands/set_skill_state.md) 的动态输入，包含实际技能实体和要写入的完整状态。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `skill_id` | 要修改的有效技能实体 |
| `state` | [`skill_state`](../../table/skill_state.md) | 要设置的完整状态，`count` 不裁剪 |

## 注意

技能可以属于后台角色或已战败但未离场的角色，不要求所属角色当前出战。状态为零不会删除技能。

本类型仅作为命令输入，不是可订阅的通知；命令不会广播状态变化事件。
