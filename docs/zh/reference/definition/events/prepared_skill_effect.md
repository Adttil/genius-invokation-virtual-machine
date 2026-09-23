[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **prepared_skill_effect**

# givm::prepared_skill_effect

定义于头文件 `<givm/definition.hpp>`

```cpp
struct prepared_skill_effect
{
    const attachment_id attachment;
    action_speed speed = action_speed::combat;
};
```

执行准备技能的自身效果。能响应本事件的 attachment 即为准备技能附属；轮到所属角色行动时，它可以代替玩家选择，自动执行定义提交的程序。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `attachment` | `const attachment_id` | 本次消耗的准备技能附属；只读 |
| `speed` | [`action_speed`](../../enums/action_speed.md) | 本次行动的速度，默认战斗行动；响应可改为快速行动 |

## 触发与结算

[`begin_action`](../commands/begin_action.md) 每次即将提供行动选择时，先完成 [`before_action`](before_action.md) 及其响应程序，再检查当前出战角色。若角色未受控制，按其附属实体的遍历顺序选中第一个能响应本事件的有效 attachment。本次不返回 `action_selection`，也不等待玩家选择。

选中后先将该 attachment 标记为离场，再完成 [`attachment_removed`](attachment_removed.md) 的全场通知及其响应程序，最后仅向选中的 attachment 发送本事件。尽管它已经离场，此专属响应仍会调用，响应仍能通过实体参数和 `attachment` 读取其定义、状态及归属信息。响应使用 `invoke` 提交准备技能的效果程序。

选中即确定本次行动。离场响应随后改变出战角色或控制状态，不撤销已确定的准备技能。整个行动无需费用计算或支付，也不发出普通技能或特技的使用前后通知。

按 `speed` 的最终值处理行动权：战斗行动消耗下落攻击资格 `can_plunge`，快速行动保留该资格。同一时机只选一个准备技能；快速行动完成后再次进行选择前检查，因此剩余的准备技能仍可能先于玩家选择执行。

## 附属的保留与清除

定义约定将准备技能附属放在出战角色上；行动阶段不检查后台角色的准备技能。角色受控制时不触发，也不消耗准备技能附属。跨回合或宣布结束本身不清除这些附属。

成功切换出战角色时，原出战角色上所有能响应本事件的附属会一起标记为离场，再按附属顺序逐个广播 `attachment_removed`，最后处理正常的切换通知。被阻止的切换和重复设置同一出战角色不清除准备技能。观察模式的切换现场保留旧状态，继续推进后才进行清除。

## 伤害标志

准备技能直接产生的伤害由定义源显式设置 [`damage_flag_bits::prepared_skill`](../../enums/damage_flag_bits.md)。该标志不会自动加到响应程序中的所有伤害，也不表示发生了普通技能使用通知。

## 参阅

| | |
| --- | --- |
| [`attachment_view`](../../table/attachment_view.md) | 准备技能附属的实体视图 |
| [`subscribed_events`](../subscribed_events.md) | 各实体形态可响应的事件 |
| [`before_action`](before_action.md) | 选择行动前的自动效果 |
| [`attachment_removed`](attachment_removed.md) | 附属离场通知 |
