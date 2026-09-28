[givm](../../reference.md) / [执行](../executor.md) / **execution_view**

# givm::execution_view

定义于头文件 `<givm/executor.hpp>`

```cpp
template<execution_state State>
class execution_view;
```
[`execution_state`](execution_state.md)

一处对局执行现场的视图，例如需要换牌的玩家、已经生效的伤害，或已结束对局的结果。

该模板组织各类现场视图。调用方通过 [`executor::view_in`](executor/view_in.md) 取得当前现场的访问对象，可用 `auto` 接收。不同现场提供不同的信息；需要外部输入的现场还提供相应输入操作。观察现场通过 [`resume`](execution_view/resume.md) 继续执行；纯通知现场的其他信息直接从牌桌读取。

## 模板参数

| | |
| --- | --- |
| `State` | 视图对应的执行现场种类 |

## 特化

| | |
| --- | --- |
| [`execution_view<initialized>`](execution_view/initialized.md) | 初始化完成、尚未执行游戏流程的视图 |
| [`execution_view<finished>`](execution_view/finished.md) | 已结束对局的结果视图 |
| [`execution_view<card_selection>`](execution_view/card_selection.md) | 指定玩家换牌现场的视图 |
| [`execution_view<initial_card_selection>`](execution_view/initial_card_selection.md) | 开局首次换牌选择现场的视图 |
| [`execution_view<initial_active_character_selection>`](execution_view/initial_active_character_selection.md) | 开局首次出战选择现场的视图 |
| [`execution_view<remaining_active_character_selection>`](execution_view/remaining_active_character_selection.md) | 开局剩余一方出战选择现场的视图 |
| [`execution_view<dice_selection>`](execution_view/dice_selection.md) | 双方投骰阶段的重投选择现场视图 |
| [`execution_view<dice_reroll_selection>`](execution_view/dice_reroll_selection.md) | 单方重投命令的选择现场视图 |
| [`execution_view<action_selection>`](execution_view/action_selection.md) | 选择行动的现场视图 |
| [`execution_view<deck_cards_discarded>`](execution_view/deck_cards_discarded.md) | 整批牌堆牌已经舍弃、效果尚未开始的现场视图 |
| [`execution_view<health_reduced>`](execution_view/health_reduced.md) | 伤害扣除生命后的现场视图 |
| [`execution_view<active_character_changed>`](execution_view/active_character_changed.md) | 设置新出战角色前的现场视图 |

表中的模板实参均为 `execution_state` 的枚举项。

## 纯通知现场

主模板用于 `initial_active_characters_selected`、`round_started`、`action_started`、`round_end_declared` 和 `round_ending`。这些视图提供 [`resume`](execution_view/resume.md)，不提供其他读取或输入操作；对应的出战角色、玩家及回合数等信息直接从牌桌读取。

## 注意

有数据的视图借用当前执行现场，不拥有历史记录；下一次推进或重建现场后，先前的借用失效。复制执行器不会把原有视图重定向到副本。

输入操作提交选择并直接推进，返回下一处 [`execution_state`](execution_state.md)。观察现场由 `resume` 推进；终局视图没有推进操作。查询、费用预览和独立合法性检查不提交输入，也不推进。

未定义 `NDEBUG` 时，每个视图操作都检查现场种类和是否仍有效；种类不匹配或旧视图被使用时抛出 [`execution_view_error`](execution_view_error.md)。检查包括同种类现场的再次出现，不能复用上一处现场的视图。Release 不保留这些诊断记录，不执行检查。

输入提交在 Debug 下还会自动验证相应参数，合法性结果失败时抛出 [`view_input_error`](view_input_error.md)，实体 ID 越界或已移除的诊断沿用 [`command_input_error`](command_input_error.md)；均发生在提交或推进之前。独立 `*_validate` 接口仍供 UI 自行查询；Release 提交直接使用输入，调用方必须保证其合法。

一旦开始推进，全部旧视图以及借出的引用和 span 失效；执行中抛出异常不回滚牌桌和执行进度，不应继续使用该现场。Debug 对视图的检查不能追踪已经借出的引用或 span。

## 参阅

| | |
| --- | --- |
| [`executor`](executor.md) | 游戏对局的执行器 |
