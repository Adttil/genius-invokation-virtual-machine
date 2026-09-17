[givm](../../reference.md) / [执行](../executor.md) / **execution_view**

# givm::execution_view

定义于头文件 `<givm/executor.hpp>`

```cpp
template<execution_state State>
class execution_view;
```
[`execution_state`](execution_state.md)

一处对局执行现场的视图，例如需要换牌的玩家、已经生效的伤害，或已结束对局的结果。

该模板组织各类现场视图。调用方通过 [`executor::view_in`](executor/view_in.md) 取得当前现场的访问对象，可用 `auto` 接收。不同现场提供不同的信息；需要外部输入的现场还提供相应输入操作。只作通知的现场不提供额外的读取或输入操作，相关信息直接从牌桌读取。

## 模板参数

| | |
| --- | --- |
| `State` | 视图对应的执行现场种类 |

## 特化

| | |
| --- | --- |
| [`execution_view<finished>`](execution_view/finished.md) | 已结束对局的结果视图 |
| [`execution_view<card_selection>`](execution_view/card_selection.md) | 指定玩家换牌现场的视图 |
| [`execution_view<initial_card_selection>`](execution_view/initial_card_selection.md) | 开局首次换牌选择现场的视图 |
| [`execution_view<initial_active_character_selection>`](execution_view/initial_active_character_selection.md) | 开局首次出战选择现场的视图 |
| [`execution_view<remaining_active_character_selection>`](execution_view/remaining_active_character_selection.md) | 开局剩余一方出战选择现场的视图 |
| [`execution_view<dice_selection>`](execution_view/dice_selection.md) | 骰子重投选择现场的视图 |
| [`execution_view<action_selection>`](execution_view/action_selection.md) | 选择行动的现场视图 |
| [`execution_view<health_reduced>`](execution_view/health_reduced.md) | 伤害扣除生命后的现场视图 |
| [`execution_view<active_character_changed>`](execution_view/active_character_changed.md) | 设置新出战角色前的现场视图 |

表中的模板实参均为 `execution_state` 的枚举项。

## 纯通知现场

主模板用于 `initial_active_characters_selected`、`round_started`、`action_started`、`round_end_declared` 和 `round_ending`。这些视图不提供读取或输入操作；对应的出战角色、玩家及回合数等信息直接从牌桌读取。

## 注意

有数据的视图借用当前执行现场，不拥有历史记录；下一次推进或重建现场后，先前的借用失效。复制执行器不会把原有视图重定向到副本。

输入操作通过参数填写选择，不返回可修改引用，不进行合法性检查，也不推进对局。调用方必须保证现场种类、参数和对局条件满足相应操作的要求；违反要求属于未定义行为。

独立检查接口供调用方按需使用，检查本身不提交输入。所有输入现场都须先提交合法选择，再调用 [`step`](executor/step.md)；查询或检查不能代替提交，空选择也须显式提交。

换牌、开局出战选择和重投视图通过 `selection_validate` 检查拟提交的参数；行动视图按操作提供检查，例如 `switch_payment_validate`。提交操作和后续推进均不会自动调用这些检查。

## 参阅

| | |
| --- | --- |
| [`executor`](executor.md) | 游戏对局的执行器 |
