[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **is_controlled**

# givm::execution_view<execution_state::action_selection>::is_controlled

定义于头文件 `<givm/runtime.hpp>`

```cpp
bool is_controlled(const definition_library& library, const table& card_table) const noexcept(/* Release 为 true，Debug 为 false */);
```

检查当前行动玩家的出战角色是否受控，供行动界面决定技能和主动特技是否可选。

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库 |
| `card_table` | 当前行动发生的牌桌，当前行动方已有有效出战角色 |

## 返回值

当前出战角色的 [`definition_library::is_controlled`](../../definition_library/is_controlled.md) 结果。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。

此查询独立于费用、目标与支付检查，不改变候选数量，也不提交行动。调用方可在提交技能或特技前独立检查；[`use_skill`](use_skill.md)、[`use_technique`](use_technique.md) 在 Debug 提交时自动拒绝受控角色的使用，Release 由调用方保证合法。

控制不禁止主动切换出战角色；免控保护也不阻止主动切换。需要检查任意角色或在卡牌定义中检查时，使用定义库的同名查询。
