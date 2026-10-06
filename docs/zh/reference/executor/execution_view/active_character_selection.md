[givm](../../../reference.md) / [执行](../../executor.md) / [执行视图](../execution_view.md) / **active_character_selection**

# givm::execution_view<execution_state::active_character_selection>

定义于头文件 `<givm/runtime.hpp>`。

本次结算确实造成击倒时，在它产生的通知及派生响应全部结束后检查双方出战，为仍已击倒的出战角色请求重新选择。选择结果产生普通切人通知，完成后恢复原程序。一个自身没有造成击倒的子响应不会提前处理父域留下的死亡出战；自身造成击倒的子响应会检查双方。

| 成员 | 说明 |
| --- | --- |
| [`player`](active_character_selection/player.md) | 需要选择的玩家 |
| [`selection_validate`](active_character_selection/selection_validate.md) | 验证角色属于该玩家、有效、存活且生命非零 |
| [`select`](active_character_selection/select.md) | 提交选择并推进 |

未定义 `NDEBUG` 时，不合法输入抛出 `view_input_error`。允许复制暂停中的执行器和牌桌后分别选择。双方都需要选择时先请求玩家 0，再请求玩家 1；收齐答案之前不修改牌桌，之后按当前行动方优先的顺序逐个切换并处理各自通知。
