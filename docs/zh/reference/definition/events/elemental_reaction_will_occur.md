[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **elemental_reaction_will_occur**

# givm::elemental_reaction_will_occur

已确定反应槽位后、写入新附着和执行反应默认效果前的即时事件。来源、目标、输入元素、原附着、`reaction_id reaction` 和原因 `cause` 只读。

`element_aura new_aura` 已由所选反应定义的 `reaction_aura` 查询预填，响应可直接修改。`bool cancel_default_effects` 默认 false；普通响应结束后写入最终附着，若未取消则告知所选反应定义，执行派生伤害、实体生成等定制效果。反应事实及 `after_elemental_reaction` 始终保留。

`source_player()` 返回引发反应的来源玩家；`reaction.player_id` 是映射所属玩家，即目标的对方，两者可以不同。伤害加值另由 `damage_calculation::cancel_reaction_bonus` 控制。
